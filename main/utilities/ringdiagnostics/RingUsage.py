'''
This module provides the ability to track ringbuffer usage.
What we do:

  We start with the ringbuffers in localhost enumerate them 
  and assign commands to the producer (if there is one) and all
  consumers.
  
  Since, due to pipelines we don't know the actual data flow
  within the system and since this could be run by a troubleshooter
  rather than experimenter, we assume all rings are involved.
  
  Once we've assigned the producer/consumer names, we can look for
  consumers of the form:
  
  ringtostdout some-hbostname
  
  or
  
  ring2stdout ... --comment "Hoisting to host"
  
  The first of these can come from a native NSCLDAQ ringmaster, the
  second from the Rust ringmaster.
  
  We then enumerate the unique set of hosts we are hoisting data to.
  The assumption, normally true, but not necessrily true, is that
  nobody will hoist from a proxy ring.
  
  Having enumerated the hosts data are being hoisted to, 
  we can enumerate the ringbuffers in those remote systems.  
  Ring buffers with names of the form hostname.ringname
  are proxy rings for our local ringname ringbuffer.
  
  Assigning producer/consumer names, requires using the
  ssh package to run the program (not package)
  pidtocommand.phy in the remote systems.
  
  This is possibily the most time consuming part of this process.
  
  It is possible that some non proxy rings may be hoisted in turn
  to a remote system so we keep going, discarding hosts we've already
  seen, until we've mapped the entire system.
  
  The structure we try to create is a list of dicts. Each top level
  dict represents a host and has the keys;
  
  'host' - the hostname.
  'rings' - enumeration of non-proxy rings in that host.  This enumeration
  is, itself, a list of ring definitions.  These are dicts that are
  ringbuffer dicts that come from nscldaqutils.RingMaster.list_rings with the addeed keys:
  'producer_command' with the producer (if any) command string.
  and for each consumer 'consumer_command" which is the command the consumer runs.
  'proxies' - which are the list of dicts that describe the proxy rings in remote system.
     These are ringbuffer definition dicts with the ring name replaced by 
     ring@host where host is the host the proxy lives in.
     

    As you can see, the logic of describing the data flow given an startting point is:
    1. Complex.
    2. Incomplete as we only can chart the dataflows that have ultimate origin in this system.
      (consider starting with an spdaq system in an event built case, we only see the branch
       of the dataflow that originates in that system not, e.g. the dataflow asssociated with other
       source ids).
       
    The incompleteness is not normally a problem because we are typically troubleshooting dataflow
    stalls that we know, or think we know, are due to the failure of a specific data sourcde or
    the stalling of some consumer in that source's data flow.l
'''

from nscldaq import nscldaqutils
from nscldaq import pidtocommand
import socket
import os.path


#------------------------ Internal utiltities

# _getLocalCommand
#   Return a string that represents the 
#   command associated with a pid.
#   If pidtocommand.pidToCommand() raises,
#   '<unavailable>' is returned.

def _getLocalCommand(pid):
    try:
        return ' '.join(pidtocommand.pidToCommand(pid))
    except:
        return '<unavailable>'
    
    
# _getRemoteCommand
#   Get the command associated with a pid in an external host
#  (well really this will work with local commands if host is e.g. 'localhost' but will
#  be much slower than _getLocalCommand)
#
# Prerequisistes:
#   * The DAQ must have been setup (e.g. we relay on DAQBIN)
#   * Passwordless ssh must have been set up for the account.
#
# Return value is, as for _getLocalCommand.
#
def _getRemoteCommand(host, pid):
    result = nscldaqutils.ssh(host, f'$DAQBIN/pidtocommand {pid}')
    output = nscldaqutils.getSshOutput(result)
    error  = nscldaqutils.getSshError(result)
    # If there are no lines in ouptput, then we got nothing:
    
    
    if len(output) >0:
        return output[0]
    else:
        return "<unavailable>"
        
## Get target from specific hoisters:

# For the native hoister, the second command word is the host:
# Note that it's possible to use ringtostdout in a non
# hoisting capacity when (hopefully) it won't have a  parameter:
# and will just liook like "ringtostdout ringname" rather than
# "ringtostdout ringname hostname"
def _native_hoister(command):
    if len(command) == 3:
        return command[2]
    else :
        return None

# For the rust hoister, we have to find the text after --comment.
#  The host will be the 3'd word of its parameter.
def _rust_hoister(command):
    for idx, param in enumerate(command):
        if param == "--comment":
            return  command[idx+3]
    return None

# Table of hoister target getters by command word.

hoister_target = {
    'ringtostdout' : _native_hoister,
    'ring2stdout'  : _rust_hoister
}

##
# _getHoistTarget
#   Givnen a consumer command, determine the host to which
#   that command is hoisted or return None if the command
#   is not a hoisting command.
#
def _getHoistTarget(cmd):
    global hoister_target
    
    #
    #  Get the command word untangled from path stuff.
    #
    
    command_words = cmd.split()
    command = command_words[0]
    command_name = os.path.split(command)[1]
    
    if command_name in hoister_target.keys():
        return hoister_target[command_name](command_words)
    
    return None

##
# _ProxyRing
#   Given a ringbuffer name and host, return the ring name it is a proxy for or None if
#  the ring name is not a proxy ring.  Proxy ring names are of the form
#  FQDN.ring  where FQDN is the fully qualified host name from which the
#  data are being hoisted in to the proxy and ring the name of the ring in that host.
#  For example, if a ring is named
#    spdaq10.frib.msu.edu.fox
#
#  it is a proxy for the ring named 'fox' in 'spdaq10.frib.msu.edu'
#  Return is either a two element (host, ring) list or None if this is not a proxy ring.
#  In the previous exmaple, we'd return ('spdaq10.frib.msu.edu', 'fox').  
#
#  NOTE:
#    We will get fooled by names with periods in them.  A ring like george.of.the.jungle
#    While not a proxy ring will appear to us like a proxy ring for jungle@george.of.the

def _proxyRing(ringname):
    components = ringname.split('.')
    if (len(components) > 1):
        ring = components.pop()
        host = '.'.join(components)
        return (host, ring)
    else:
        return None

##
# _findSourceRing
#   Given a ring that is known to be a proxy ring and has been marked as such,
#   and the system charted so far, find the ring that is the source for that
#   proxy.
# 
#   Returns  None if there is not (yet) a match.
#
def _findSourceRing(proxy, system):
    host = proxy['proxyhost']
    hostfqdn = socket.getfqdn(host)
    ring = proxy['proxyring']
    for  h in system:
        if h['host'] == host or h['host'] == hostfqdn:
            for r in h['rings']:
                if r['name'] == ring:
                    return r
            break            # No such ring in the host - orphaned proxy.
    
    return None               # No matching host or ring in host.

#------------------------ Public entries

def makeLocalRingInfo():
    ''' 
        Get the ringbuffer information for the local rings.
        For those we can directly ask the system what the 
        commands are for the producer and consumers
    
        The return will be an element of the list of dicts that define the 
        rings in a host.    
    '''
    host = socket.getfqdn()
    rm = nscldaqutils.RingMaster(host)
    usage = rm.list_rings()
    
    result = {'host': host, 'rings': []}
    for ring in usage:
        
        # We dont' distentangle proxies but we _do_ make an empty list for them.
        
        ring['proxies'] = []
        
        #  Add the producer and consuer commands.
        if ring['producer_pid'] != -1 :
            ring['producer_command'] = _getLocalCommand(ring['producer_pid'])
            
        else :
            ring['producer_command'] = 'None'
        
        for consumer in ring['consumers']:
            consumer['consumer_command'] = _getLocalCommand(consumer['consumer_pid'])
        
        result['rings'].append(ring)

    return result
    

def makeRemoteRingInfo(host):
    '''
        Get the ring buffer information for rings in a remote system.
        See makeLocalRingInfor for what is returned.  Note that we need
        to use ssh to get the process name information in this case.
    '''
    rm = nscldaqutils.RingMaster(host)
    usage = rm.list_rings()
    result = {'host': socket.getfqdn(host), 'rings': []}
    for ring in usage:
        # We dont' distentangle proxies but we _do_ make an empty list for them.
        
        ring['proxies'] = []
        
        #  Add the producer and consuer commands.
        if ring['producer_pid'] != -1 :
            ring['producer_command'] = _getRemoteCommand(host, ring['producer_pid'])
            
        else :
            ring['producer_command'] = 'None'
        
        for consumer in ring['consumers']:
            consumer['consumer_command'] = _getRemoteCommand(host, consumer['consumer_pid'])
            
        
        result['rings'].append(ring)
    return result
    
    
def getHoistedHosts(info):
    '''
        Given information about a ringbuffer, return the set of hosts that ring is
        being hoisted to.  This is done by examining the consumers as described in the module
        comments.
        
        Returns, an array of the names or IP addresss of hosts to which the ring is being hoisted.
    '''
    result = []
    for consumer in info['consumers']:
        cmd = consumer['consumer_command']
        host = _getHoistTarget(cmd)
        if host is not None:
            result.append(socket.getfqdn(host))    # Always save the fully qualified name.
    
    return result

def removeProxies(data):
    '''
        Given a list of Rings for a host, removes the rings that are
        proxies.  The list of removed rings are returned as their original dicts
        with two added keys:
        proxyhost:  - Host that holds the original ring.
        proxyring:  - Ring this is a proxy form.
        
    '''
    rings = data['rings']
    host = data['host']
    result = []
    removelist = []
    for (n, ring) in enumerate(rings):
        proxyInfo = _proxyRing(ring['name'])
        if proxyInfo is not None:
            ring['proxyhost'] = proxyInfo[0]
            ring['proxyring'] = proxyInfo[1]
            ring['localhost'] = host
            # Diddle the ring so it says where it lives not where it came from:
            
            ring['name'] = '@'.join([ring['proxyring'], host])
            result.append(ring)
            removelist.append(n)
    # Now remove the indices in removelist from rings:

    
    removelist.sort(reverse=True)
    for i in removelist:
        del rings[i]
    
    return result
    

def getUniqueNames(listOfLists):
    '''
    Given an  iterable of iterables (probably containing strings),
    returns an iterable containing the unique names.
    '''

    result = {}
    for outer in listOfLists:
        result[outer] =''
    return [x for x in result.keys()]
    
def addProxies(proxies, rings):
    
    '''
        Given  proxy ring definitions gotten from removeProxies on some host, and the list of
        dicts we have so far, Put the proxies in their rightful place.  It is possible that the
        host for some proxy has not yet been scanned. 
        
        Therefore, the proxies is in/out.  As we add proxies to the rings, we remove them from
        the input list. 
        
        Parameters:
        proxies - the proxies we want to integrate if possible.
        rings   - The rings for the hosts we have so far.
        
        Returns:
        The proxies we were not able to integrate with the existing rings.
    '''
    
    matched = []
    
    #  Integrate the proxies we can, recording their indices.
    for (n, proxy) in enumerate(proxies):
        source = _findSourceRing(proxy, rings)
        if source is not None:
            source['proxies'].append(proxy)
            matched.append(n)

    # Remove the matched ones:
                
    matched.sort(reverse=True)
    for i in matched:
        del proxies[i]
        
    # Return the remaining proxies:
    
    return proxies

def removeCheckedHosts(data, hosts) :
    #  Remove any hosts that have been checked from the  host list.
    
    # Enumerate the hosts in data:
    
    checked_hosts = [x['host'] for x in data]
    return [h for h in hosts if h not in checked_hosts]
def systemUsage():
    ''' 
        This is the normal entry point.  It probes the entire system in a 
        -   First the local system is probed and its proxies and hosters
            are determined.
        -   The hosts we hoist to and have proxies for a re used to generate a
            single list of unique other hosts.
        For each host in that list of unique hosts;
           - We probe that host and _its_ proxies and hoistera are determined.
           - All proxies are integrated into hosts as possible.
           - Unique new hosts are added to the host list that we are looking at.
        The loop above is executed until there are no more hosts in the host list.
        Any hosts for which we cannot get information are ignored as they are clearly _not_
        in the data flow since they've gone down.
    '''
    result = []
    remaining_hosts = []
    # Do the local host:
    
    local_rings = makeLocalRingInfo()
    proxies     = removeProxies(local_rings)

    for ring in local_rings['rings']:
        remaining_hosts += getHoistedHosts(ring)
    for p in proxies:
        remaining_hosts.append(p['proxyhost'])
    remaining_hosts = getUniqueNames(remaining_hosts)
    result.append(local_rings)
    
    # In a sane system, we can't integrate the proxies as in a sane system
    # There won't be proxies for local rings.
    
    seen_hosts = [socket.gethostname()]
    
    while len(remaining_hosts) > 0:
        host = remaining_hosts.pop(0)
        seen_hosts.append(host)
        raw_host = host
        host = socket.getfqdn(host)
        
        
        # It' spossible that we've already looked at the host
        # Because we're processing a host found in a hoist or a proxy
        # If that's the case we skip it.
        
        if host not in remaining_hosts:
            try:
                remote_rings = makeRemoteRingInfo(host)
                proxies += removeProxies(remote_rings)
                result.append(remote_rings)
                for ring in remote_rings['rings']:
                    remaining_hosts += getHoistedHosts(ring)
                for p in proxies:
                    if p['proxyhost'] not in seen_hosts:
                        remaining_hosts.append(p['proxyhost'])
                addProxies(proxies, result)
                remaining_hosts = getUniqueNames(remaining_hosts)
                remaining_hosts = removeCheckedHosts(result, remaining_hosts)
            except:                                   
                print("exception for", host)
                pass                               # Ignore hosts we can't talk to.
            # Prune the hosts we've already seen.. in case, some how, there are
            # loops in the host graph.
            
            remaining_hosts = [x for x in remaining_hosts if x not in seen_hosts]
    return result

if __name__ == "__main__":
    import pprint
    pprint.pp(systemUsage())