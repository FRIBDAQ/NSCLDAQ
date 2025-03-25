'''
  This is the model for our ring item display.  The model has the following hierarchy:
  
  host   - a host in the model.
    ring   - information about the ring.
      Consumers
           - information about the consumers for that ring
      Remote
           proxy_ring - information about remote proxy
             Consumers information about the consumers for that proxy ring.
             
The following headeer4s exist:
  Column 0 - none
  Column 1 - Name - ringname used by ring item names.
  Column 2 - producer - Producer command - used by ring items
  COlumn 3 - producer pid - used by ring items.
  Column 4 - Size - size of ring in k used by ring items.
  Column 5 - Free space - bytes free in ring - used by ring items.
  Column 6 - Consumer - consumer command, used by consumers.
  Column 7 - pid - Process id of consumer used by consumers.
  Column 8 - backlog - back log used by consumers.

'''
import pprint
from PyQt5.QtGui import QStandardItem, QBrush
from PyQt5.Qt import *

# Columns:
RING_NAME = 1
RING_PRODUCER = 2
RING_PRODUCER_PID = 3
RING_SIZE = 4
RING_FREE=5
CONSUMER_CMD = 6
CONSUMER_PID = 7
CONSUMER_BACKLOG = 8

NUM_COLUMNS = 9

class RingModel:
    def __init__(self, view, alarm_pct):
        '''
        Initialize a RIngModel where:
        view - is the RingView whose data we manage.
            The view's model must be set and be a QStandardItemModel.
        
        alarm_pct - percentage backlog that represents a stall.
            Rings are marked red if their free space is below 100-alarm_pct of ring size.
            Consumers are marked red if their backlog is above alarm_pct of 
            the ring size. 
            Hosts and Proxy items are marked red if they have any rings that are red
            (note that a red ring implies a consumer that's red by definition).
            
        
        '''
        self._model = view.model()
        self._alarm = alarm_pct
        self._debug = False   # Enable debug printing.
        
        #  Make an item so we can get the normal background brush:
        
        fake_item = QStandardItem('')
        self._okbrush = fake_item.foreground()
        self._errbrush = QBrush()
        self._errbrush.setColor(Qt.red)
        

    def setDebug(self, state):
        self._debug =  state
    def update(self, data):
        '''
            Update the model with new data gotten from e.g. RingUsage.systemUsage()
        '''
        if self._debug:
            print('Updating with: ')
            pprint.pp(data)
    
        # Clear alarms - they'll get re-asserted by consumer updates if appropriate:
        #  Each list is a host.
        
        self._clear_alarms()
        
        for host in data:
            host_item = self._find_or_create_host(host['host'])
            
            # Each host has rings as children
            
            for ring in host['rings']:
                ring_item = self._find_or_create_ring(host_item, ring['name'], False)
                self._update_ring(ring_item, ring)
                
                # Each ring may have consumers.
                
                consumers = ring['consumers']
                for consumer in consumers:
                    self._update_consumer(ring_item, consumer, ring['size'])
                
                # Each ring may have remote rings.
                # This all has identical structure to ring but
                # without a remot item on the ring.
                #
                remote_parent = ring_item.child(1, 0)    # The parent of all remotes:
                remotes = ring['proxies']
                for remote in remotes:
                    remote_item = self._find_or_create_ring(remote_parent, remote['name'], True)
                    self._update_ring(remote_item, remote)
                    
                    # Remotes also have consumers:
                    
                    consumers = remote['consumers']
                    for consumer in consumers:
                        self._update_consumer(remote_item, consumer, remote['size'])
        
        # Now that we've updated all existing items, we need to prune all items that
        # no longer exist.
        
        self._prune(data)
            
    def _collect(self, dictlist,  key):
        # GIven a list of dicts with a 'key' returns the value of all of those keys:
        
        return [x[key] for x in dictlist]
        
    def _collect_hosts(self):
        # Returns a list of all of the host items and their 
        # row nummbers in the model.
        # Sadly model rows need to be treated diferently than the
        # children... almost enough to make one wish I had a super top node
        # that _everytyhing_ is under...though the visual appeal of that
        # is not as good:
        
        result = []
        row = 0
        while True:
            host_item = self._model.item(row, 0)
            if host_item is None:
                break
            result.append((host_item, row))
            row += 1
        return result
        
    def _collect_child_column(self, parent, column):
        #  Given a parent item, returns a list of
        #  item,row pairs for all children of the parent.
        #
        result = []
        row = 0
        while True:
            item = parent.child(row, column)
            if item is None:
                break
            result.append((item, row))
            row += 1
        
        return result
    
    def _remove_hosts(self, rows):
        #  Given a list oif model rows, removes them.
        #
        
        rows.sort(reverse=True)  # So row numbers of remaining rows don't change
        
        for row in rows:
            self._model.removeRow(row)
            
    def _remove_children(self, parent, rows):
        # GIven a list of child rows, removes them from the parent
        
        rows.sort(reverse=True)
        for row in rows:
            parent.removeRow(row)
        
    def _clear_child_alarms(self, parent):
        # Clear alarms on all children recursively:
        # We clear the col 0 and, if they exist, 
        global RING_NAME
        global CONSUMER_CMD  
        row = 0
        while True:
            child = parent.child(row, 0)
            if child is None:
                return
            child.setForeground(self._okbrush)
            name = self._sibling(child, RING_NAME)
            if name is not None:
                name.setForeground(self._okbrush)
            consumer = self._sibling(child, CONSUMER_CMD)
            if consumer is not None:
                consumer.setForeground(self._okbrush)
            
            # Do our children too:
            
            self._clear_child_alarms(child)
            
            row += 1
    
    def _find_entry(self, dict_list, key, value):
        # For a list of dicts, returns the item whose key matches a specific value
        
        for d in dict_list:
            if d[key] == value:
                return d
        
        return None
        
    def _find_item(self, items, text):
        #  Finds an item with the matching text in a list of items:
        
        for item in items:
            if item.text() == text:
                return item
        
        return None
        
    def _clear_alarms(self):
        # Clear alarms by:
        # For all model rows:
        #   Setting col 0's background to the ok brush.
        #   Recusively for each child, set it's col0 background to okbrush
        #   and it's consumer_command to okbrush
        
        model_row = 0
        while True:
            top = self._model.item(model_row, 0)
            if top is None:
                return
            top.setForeground(self._okbrush)
            self._clear_child_alarms(top)
            model_row += 1
        
    def _sibling(self, item, column):
        #  Get the sibling of an item frequently used.
        
        return item.parent().child(item.row(), column)   
    
    def _insert_empty_row(self, parent):
        # insert an empty row under the parent and return the item in col 0.
        global NUM_COLUMNS
        row = []
        for i in range(NUM_COLUMNS):
            row.append(QStandardItem(''))
        
        parent.appendRow(row)
        return row[0]
        
    def _find_or_create_host(self, name):
        # The hosts are the top level of the hierarchy, their names
        # are in col 0.  If one already exists with text() == name,
        # return it if not, make one and return it.
        
        matches = self._model.findItems(name,  Qt.MatchExactly, 0)
        for match in matches:
            if match.text() == name:
                return match
        
        host_name = QStandardItem(name)
        self._model.appendRow(host_name)
        return host_name
    
    def _find_or_create_ring(self, host_item, ring_name, is_remote):
        # If a ring of the given ringname exists, return its col 0.
        # if not create:
        #    The ring items, 
        #    A child with text 'Consumers'
        #    A child with text 'Remote
        #
        # Returns the column 0 ring item.
        global RING_NAME
        row =0
        while True:
            ring_item = host_item.child(row, RING_NAME)
            if ring_item is None:
                break                        # Need to create
            else:
                if ring_item.text() == ring_name:
                    return self._sibling(ring_item, 0)
                row += 1
        
        # Have to create a new ring item:
        
       
        result = self._insert_empty_row(host_item)
        result.appendRow([QStandardItem('Consumers'),])
        if not is_remote:
            result.appendRow([QStandardItem('Remote')],)
        
        return result
    
    def _update_ring(self, ring_item, ring):
        # Update the row identified by ring_item with the ring statistics. for it's ring 
        # which are in ring:
        global RING_PRODUCER
        global RING_PRODUCER_PID
        global RING_SIZE
        global RING_FREE
        
        self._sibling(ring_item, RING_NAME).setText(ring['name'])
        self._sibling(ring_item, RING_PRODUCER).setText(ring['producer_command'])
        self._sibling(ring_item, RING_PRODUCER_PID).setText(str(ring['producer_pid']))
        self._sibling(ring_item, RING_SIZE).setText(str(ring['size']))
        self._sibling(ring_item, RING_FREE).setText(str(ring['free']))
    
    
    def _find_or_create_consumer(self, ring_item, consumer_data):
        #  Given a ring item (QStandardItemModel), and the data for a consumer,
        #  either finds the row for the consumer with the same PID as consumer_data['consumer_pid']
        #  Or creates a new one.
        # 
        #   THe caller should not assume anything about which item on that row is returned.
        #
        
        #  The first child of ring_item is the parent for consumers:
        
        consumer_parent = ring_item.child(0, 0)
        crow = 0
        while True:
            consumer_item = consumer_parent.child(crow, 0)
            if consumer_item is not None:
                pid_item = self._sibling(consumer_item, CONSUMER_PID)
                if int(pid_item.text()) == consumer_data['consumer_pid']:
                    return consumer_item
                else:
                    crow += 1
            else:
                break
        # Didn't find, have to create.
        
        return self._insert_empty_row(consumer_parent)
        
    
    def _update_consumer(self, ring_item, consumer_data, ring_size):
        global CONSUMER_CMD
        global CONSUMER_PID
        global CONSUMER_BACKLOG
        #
        #  Update the informationa about a consumer
        #
        consumer = self._find_or_create_consumer(ring_item, consumer_data)
        self._sibling(consumer, CONSUMER_CMD).setText(consumer_data['consumer_command'])
        self._sibling(consumer, CONSUMER_PID).setText(str(consumer_data['consumer_pid']))
        self._sibling(consumer, CONSUMER_BACKLOG).setText(str(consumer_data['backlog']))
        
        # if the backlog > self.alarm_pct of the size of the ring,
        # We color this and all parents red:
        
        threshold = ring_size * self._alarm /100
        if consumer_data['backlog']  > threshold:
            if self._debug:
                print('alarm:')
                pprint.pp(consumer_data)
                print('setting ', self._sibling(consumer, CONSUMER_CMD).text(), 'red')
            self._sibling(consumer, CONSUMER_CMD).setForeground(self._errbrush)
            parent = consumer.parent()
            while parent is not None:
                if parent.parent() is None:
                    col0 = self._model.item(parent.row(), 0)
                    col0.setForeground(self._errbrush)
                else:   
                    if self._debug:
                        print("making ", parent.text(), ' red') 
                    self._sibling(parent, 0).setForeground(self._errbrush)
                    name = self._sibling(parent, RING_NAME)
                    if name is not None:
                        name.setForeground(self._errbrush)
                parent = parent.parent()
        
        
    # remove host items that no longer exist:
    
    def _prune_hosts(self, data):
        # Remove hosts that dropped out of the data.
        
        host_items = self._collect_hosts()     # (item, row) pairs.
        hosts      = self._collect(data, 'host')
        
        dead_rows = [x[1] for x in host_items if x[0].text() not in hosts]
        
        self._remove_hosts(dead_rows)      
    
    def _prune_consumers(self, parent, consumer_data):
        #  Prune consumers of a ring.  parent is  the parent of all
        #  consumers in the model.
        #  consumer_data is the information about the consumers for
        #  the ring.
        global CONSUMER_PID
        
        actual_pids = self._collect(consumer_data, 'consumer_pid')
            
        pid_items  = self._collect_child_column(parent, CONSUMER_PID)  #(item, column) from model.
        if self._debug:
            print('pruning consumers')
            print("actual consumers:", actual_pids)
            print('GUI pids.', [int(x[0].text()) for x in pid_items])
            
        dead_rows  = [x[1] for x in pid_items if int(x[0].text()) not in actual_pids]
        self._remove_children(parent, dead_rows)
        
        
    def _prune_rings(self, host_item, ring_data):
        global RING_NAME
        # Remove rings froma  the host_item that don't appear in
        # ring_data.
        ring_names = self._collect(ring_data, 'name')
        ring_items = self._collect_child_column(host_item, RING_NAME)
        dead_rows = [x[1] for x in ring_items if x[0].text() not in ring_names]
        self._remove_children(host_item, dead_rows)
        
        remaining_rings = [x[0] for x in self._collect_child_column(host_item, RING_NAME)]  # Could have killed off some
        for ring_item in remaining_rings:
            ring_0 = self._sibling(ring_item, 0)    #  parent of children.
            consumer_item = ring_0.child(0,0)
            remote_item   = ring_0.child(1, 0)     # could be null if host_item is actually a remote.
            thering       = self._find_entry(ring_data, 'name', ring_item.text())
            self._prune_consumers(consumer_item, thering['consumers'])
            
            if remote_item is not None:    
                # Get the ring items in the remote and recurse witht he parent set to remote_item.
                
                proxies = thering['proxies']
                self._prune_rings(remote_item, proxies)
            
        
    def _prune(self, data):
        #  Prune all of the items in the model that no longer are
        # in the data:
        
        self._prune_hosts(data)
        remaining_host_items = [x[0] for x in self._collect_hosts()]    # Some may have been deleted.
        for host in data:
            host_name = host['host']
            host_rings = host['rings']
            parent_host = self._find_item(remaining_host_items, host_name)
            self._prune_rings(parent_host, host_rings)
            
        
if __name__ == '__main__':
    import RingUsage
    import RingView

    from PyQt5.QtWidgets import QApplication, QMainWindow
    

    app = QApplication(['ringview test'])
    mw = QMainWindow()
    tree = RingView.RingView()
    contents = RingModel(tree, 0.9)
    usage = RingUsage.systemUsage()
    contents.update(usage)
    mw.setCentralWidget(tree)
    mw.show()
    app.exec()


