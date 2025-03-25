''' 
Provides a python interface to the manager database definition.  This
will grow as more python GUIs are added, I imagine.

The use case is to provide support for PyQt user interfaces for configuration
and so on.
'''

import sqlite3
def boolToInt(b):
    return  1 if b else 0
def boolToInt(b):
    return  1 if b else 0


class Container:
    def __init__(self, handle):
        ''' Construct an interface to the container part of a db.
            
            handle - is an sqlite3 handle open on the database.
                Note handle ownership is not transferred to the 
                object.. you can happily pass the same handle
                to more than  one Container constructor or
                even to more than one type of database object.
        '''
        self._db = handle
        
    def _exists(self, name):
        # True if name is already a container.
        
        c  = self._db.cursor()
        res = c.execute('SELECT COUNT(*) FROM container WHERE container = ?', (name,))
        
        (count, ) = res.fetchone()
        return count != 0
    
        
    # Public interface:
    
    def exists(self, name):
        ''''''
        ''' determines if the container 'name' exists. '''
        return self._exists(name)

    def add(self, name, image, initscript, mountpoints):
        '''
            Add a new container to the database.
            
            name - name of the container - this is a 'handle' the user can use to refer to the
                  container.
            image - The container image file in the host filesystem.
            initscript - Path to the initscript that will be  used to intialize the container 
                environment for programs run in it.  Note:
                *  The file path is in the host.
                *  The script will be sucked into the database rather than referenced externally.
                *  If the value of ths is None, then no init script is provided for the
                container.
            mountpoints -  
                A list of host -> container filesystem bindings.  Each binding is a one or
                two element list.  If a one element list the target for the binding will be
                the same as it is in the host.  If a two element list the target is specified
                in the second element so, for example:
                 (/mnt/evtdata/0400x,) - mounts /mnt/evtdata/0400x -> /mnt/evtdata/0400x in the container.
                 (/usr/opt/opt-buster, /usr/opt) mounts /usr/opt/opt-buster -> /usr/opt in the container
            
        '''
        
        # Ensure the name is unique:
        
        if self._exists(name):
            raise ValueError(f'{name} is already a container')
        
        # Suck in the init script ...raises if the file is not found.
        
        init_contents = ''
        if initscript is not None:
            with open(initscript, 'r') as file:
                init_contents = file.read()
       
        #  We have enough to create the container ...
        # Which we do inside a transaction that is implicitly opened:
        # In case of error, we roll it back:
        cursor = self._db.cursor()
        try:
            cursor.execute(
                '''INSERT INTO container (container, image_path, init_script)
                        VALUES(?,?,?)
                ''',
                (name, image, init_contents)
            )
            container_id = cursor.lastrowid
            for binding in mountpoints:
                source = binding[0]
                if len(binding) > 1:
                    dest = binding[1]
                else:
                    dest = binding[0]
                cursor.execute('''
                        INSERT INTO bindpoint (container_id, path, mountpoint)
                            VALUES (?,?,?)
                    ''',
                    (container_id, source, dest)
                )
            self._db.commit()
        except:
            self._db.rollback()
            raise
        
    def remove(self, name):
        '''
            Removes the named container definition.
            Raises a value error if there is no such container.
        '''
        
        #  Get the id of the container.
        
        cursor = self._db.cursor()
        res = cursor.execute('''
            SELECT id FROM container WHERE container = ?
                                ''', (name,)                   
        )
        id = res.fetchone()
        if id is None:
            raise ValueError(f'No such container {name}')
    
        cursor.execute(
            '''
                DELETE FROM bindpoint WHERE container_id = ?
            ''', id
        )
        cursor.execute(
            '''
                DELETE FROM container where id = ?
            ''', id
        )
        
        self._db.commit()
    
    def replace(self, oldname, newname, image, initscript, mountpoints):
        '''
            Removes the container 'oldname' and replaces it with the
            container definition in newname, image, initsscript and
            mountpoints.
            
            This is just a convenience  front end to 'remove' followed by add.
            
            Note that while delete and add are both done in a transaction,
            those transactions are independent, so it's possible the delete will suceed
            but somehow the add will fail.
        '''
        self.remove(oldname)
        self.add(newname, image, initscript, mountpoints)
    
    def list(self):
        '''
            Returns a list of all containers.  Container lists are returned as
            a list of dicts with the keys:
            
            * name  - Name of the container.
            * image - Container image file.
            * init_script - the contents of the init_script.
            * bindings - The bindings specifications.  THese are an iterable containing
             two element lists of binding source binding destination.  For example,
             the binding of /usr/opt/opt-buster -> /usr/opt will be:
             ('/usr/opt/opt-buster', '/usr/opt'), For identity bindings both elements will
             be the same.
        '''

        containers = {}       # Indexed by container name.
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT container, image_path, init_script, path, mountpoint
            FROM container
            INNER JOIN bindpoint ON bindpoint.container_id = container.id
            '''
        )
        while True:
            row = r.fetchone()
            if row is None:
                break
            
            if row[0] not in containers.keys():
                name = row[0]
                image = row[1]
                script = row[2]
                containers[name] = {
                    'name': name, 'image': image, 'init_script': script, 'bindings': []
                }
        
            # Append the bindings to the container def:
            
            containers[name]['bindings'].append((row[3], row[4]))
        
        return list(containers.values())
    
    def id(self, name):
        '''
           Returns either the id of the named container or None if there is no match.
        '''
        
        cursor = self._db.cursor()
        result = cursor.execute('''
            SELECT id FROM container WHERE container = ?
                                ''', (name,))
        row = result.fetchone()
        if row is None:
            return None
        else:
            return row[0]
class EventLog:
    def __init__(self, handle):
        '''
            Supports editing the event log definition database.
            handle - the sqlite3 database handle open on the configuration database.
               The caller retains ownership.
               The same handle can be passed to as many database facade objects as desired.
        '''
        self._db = handle

    # Private methods:
    
    def _makeDict(self, row):
        '''
           Given a row delected from e.g. info or list, produces a dict
           that describes the logger in that row.  Note both of these do the
           appropriate inner joing with container to get the container name not id.
        '''
        
        return {
            'id': row[0],
            'root': row[1],
            'ring' : row[2],
            'host': row[3],
            'partial': True if row[4] > 0 else False,
            'destination': row[5],
            'critical': True if row[6] > 0 else False,
            'enabled' : True if row[7] > 0 else False,
            'container': row[8]
            
        }
    
    def _idExists(self, id):
        # Return True if the logger specified by ID exists.
        
        r = cursor = self._db.cursor();
        cursor.execute(
            '''
                SELECT COUNT(*) FROM logger WHERE id = ?
            ''', (id, )
        )
        (count, ) = r.fetchone()
        return count > 0
    
    # PUblic methods
    def exists(self, destination):
        '''
           Determines if there's a logger on the destination. 
           
           Returns boolean, True if there is one and False if not
        '''        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT COUNT(*) from logger WHERE destination = ?
            ''', (destination,)
        )
        (count, ) = r.fetchone()
        return count != 0
        
    def add(self, root, source_uri, destination, container, host, options):
        '''
            Adds a new definition of an event logger to the syhstem.  One may only have
            one logger to a destination. Parameters are:
            *  root - NSCLDAQ root directory from which eventlog will be used.
            *  source_uri - URI of the ringbuffer from which the data will be logged.
            *  destination - Destination directory in which the logging will be done. 
                 See, however  the 'partial' option.
            * container - name of the container the logger will run in.  Note the
                Tcl world allows containerless loggers, but we don't.
            * host - the host in which the logger runs.
            * options:  A dict of options which provide additional control over the
               logger.  Keys that matter are:
                * 'partial' - Boolean valued which, if true, specifies the event logger runs
                    like the old multilogger, just accumulating event files in that directory.
                    The default for this is False (note this is different from the Tcl API).
                * 'critical' - Boolean, if True, If the logger exits unexpectedly,   
                   The experiment shuts down and needs to be rebooted.  This is True by default 
                * 'enabled' boolean that if True means the logger is enabled and will, once the
                   next run begins, start logging data. Default is True.
            On success, returns the id of the logger created.
        '''
        # Make sure the destination is unique.
        
        if self.exists(destination):
            raise ValueError(f'There is already a logger saving data at {destination}')
        
        #  Get the container
        
        c = Container(self._db)
        container_id = c.id(container)
        if container_id is None:
            raise ValueError(f'There is no container named {container}')
        
        # Untangle the options:
        
        partial = 0
        critical = 0
        enabled = 0
        option_keys = options.keys()
        if 'partial' in option_keys:
            opt = options['partial']
            if type(opt) != bool:
                raise ValueError(f'The value of the "partial" option must be a boolean it was {opt}')
            partial = boolToInt(opt)
        if 'critical' in option_keys:
            opt = options['critical']
            if type(opt) != bool:
                raise ValueError(f'The value of the "critical" option must be a boolean it was {opt}')
            critical = boolToInt(opt)
        if 'enabled' in option_keys:
            opt = options['enabled']
            if type(opt) != bool:
                raise ValueError(f'The value of the "enabled" option must be a boolean it was {opt}')
            enabled = boolToInt(opt)
            
        # Now we can do the insert.
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
            INSERT INTO logger 
                (daqroot, ring, host, partial, destination, critical, enabled, container_id)
            VALUES
                (?,?,?,?,?,?,?,?)
            
            ''',
            (root, source_uri, host, partial, destination, critical, enabled, container_id)
        )
        result = cursor.lastrowid
        self._db.commit()
        return result
        
    def info(self, destination):
        '''
        Returns a dict that describes the logger to destination (or  None if there is no such logger)
        The return value has the following keys (note that the dict can be used as the options
        for a create):
        
        * 'id'  -  Logger id (row' primary key).
        * 'root' - DAQROOT for the logger.
        * 'ring' - Ring URI from which the data are logged.
        * 'host' - Host on which the logger runs.
        * 'partial' - Bool that is true if the logger is partial.
        * 'destination' - Where the data are being logged.
        * 'critical' - Bool that is true if the logger is specified to be critical.
        * 'enabled' - Bool that is true if the logger is enabled.
        * 'container' - name of the container the logger runs in.
        '''
        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT logger.id, daqroot, ring, host, partial, destination, critical, enabled, container
            FROM logger
            INNER JOIN container ON container.id = container_id
            WHERE destination = ?
            ''', (destination,)
        )
        row = r.fetchone()
        if row is None:
            return None
        
        return self._makeDict(row)
    def list(self):
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT logger.id, daqroot, ring, host, partial, destination, critical, enabled, container
            FROM logger
            INNER JOIN container ON container.id = container_id
            ''')
        result = []
        while True:
            row = r.fetchone()
            if row is None:
                break
            result.append(self._makeDict(row))
        return result
        
    def delete(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")

        cursor = self._db.cursor()
        cursor.execute(
            '''
            DELETE FROM logger WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
        
    def enable(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 1 WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
    def disable(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 0 WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
    def enable_all(self):
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 1
            '''
        )
        self._db.commit()
    def disable_all(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 0
            '''
        )
        self._db.commit()
    def start_recording(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE recording SET state = 1
            '''
        )
    def stop_recording(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE recording SET state = 0
            '''
        )
        self._db.commit()
    def is_recording(self):
        cursor = self._db.cursor()
        res = cursor.execute(
            '''
            SELECT state FROM recording
            '''
        )
        row = res.fetchone()
        if row is None:
            raise RuntimeError("is_recording - was not able to fetch a rwo from recording")
        
        return True if row[0] != 0 else False

class Program:
    def __init__(self, db):
        '''
           Create an object facade for the programs part of the database.
           The encompases tables for the program itself, it command line options 
           and parameters as well as additional environment variables it might need.
           
        '''
        self._db = db
    
    def _typeId(self, typeName):
        # Return the id of the default type:
        
        cursor = self._db.cursor()
        res = cursor.execute(
            '''
            SELECT id FROM program_type WHERE type = ?
            ''', (typeName,)
        )
        result = res.fetchone()
        if result is None:
            return None
        else:
            return result[0]
    def exists(self, name):
        '''
            Returns True if a program 'name' already exists.
        '''
        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
              SELECT COUNT(*) FROM program WHERE name = ?
            ''', (name,)
        )
        (count, ) = r.fetchone()
        return count > 0
    def id(self, name):
        '''
           Return the id of a program else None if there is not one:
        '''
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT id FROM program WHERE name =?
            ''', (name, )
        )
        result = r.fetchone()
        return result [0] if result is not None else None
    
    def add(self, name, path, host, container, wd, options):
        '''
          Add a new program:
          *  name - the name of the program, used to refer to it elsewhere - must be unique.
          *  path - Path to the executable >in the container< in which it runs.
          *  host - host in which the program runs.
          *  container - name of the container the program runs in.
          *  wd  - Working directory the program runs in.  This must be valid in the containe in
                   which the program will ru7n.
          *  options - dict containing program options keys we care about are:
              * 'type' - one of the program types in the program_type table.
                If not provided, this is 'Critical'.
              * 'initscript' - path to a script that will run prior the program.
               If not provided no script will run.  Note that the script's current contents
               will be sucked into the database and stored, rather than the path.           
              * options If provided, this is a list of option/value pairs e.g.
                  [('--source-id', 123), ('--ring', 'fox') ...]  options can also be single
                  elements if they have not value e.g. ('--debug',)
                If not provided, no options are passed to the program at start time.
              * parameters - if provided a list of parameters passed to the program on startup.
              * environment - if provided a list of pairs containing environment variable names and
                values which will be set prior to starting the program.
        
            Returns the program id (primary key of the added root record).
        
        '''
        
        # Ensure we really can make this program:
        
        if self.exists(name):
            raise ValueError(f'A program named "{name}", is already defined')
        containers = Container(self._db)
        if not containers.exists(container):
            raise ValueError(f'There is no container named "{container}"')
        
        container_id = containers.id(container)
        
        # Figure out the options and if user provided if it's legal:
        
        type_id = self._typeId('Critical')
        init_script = ''
        cmd_options = []
        cmd_params  = []
        cmd_environment = []
        
        option_keys = options.keys()
        if 'type' in option_keys:
            type_id = self._typeId(options['type'])
            if type_id is None:
                raise ValueError(f'Invalid program type: "{options["type"]}"')
        if 'initscript' in option_keys:
            init_file = options['initscript']
            if len(init_file) > 0 and not init_file.isspace():
                with open(init_file, 'r') as file:
                    init_script = file.read()
        if 'options' in option_keys:
            cmd_options = options['options']
        if 'parameters' in option_keys:
            cmd_params = options['parameters']
        if 'environment' in option_keys:
            cmd_environment = options['environment']
            
        #  The Creation of the program is done in a transaction.
        # so it'll be all or nothiung with a consistent end result.
        # We catch exceptions and rollback if one was raised:
        
        cursor = self._db.cursor()
        try:
            # Root record:
            
            cursor.execute(
                '''
                INSERT INTO program (
                     name, path, type_id, host, directory, container_id, initscript, service
                ) 
                VALUES (?, ?, ?, ?, ?, ?, ?, '')
                ''',
                (name, path, type_id, host,  wd, container_id, init_script)
            )
            program_id = cursor.lastrowid
            # Program options:
            
            for opt in cmd_options:
                name = opt[0]
                if len (opt) > 1:
                    value = opt[1]
                else: 
                    value = ''
                
                cursor.execute(
                    '''
                    INSERT INTO program_option (program_id, option, value)
                    VALUES(?, ?, ?)
                    ''', (program_id, name, value)
                )
            # Program parameters:
            
            for param  in cmd_params:
                cursor.execute(
                    '''
                    INSERT INTO program_parameter (program_id, parameter)
                    VALUES (?,?)
                    ''', (program_id, param)
                )
            # Environment variables:
            
            for env in cmd_environment:
                name = env[0]
                if len(opt) > 1:
                    value = opt[1]
                else: 
                    value = ''
                cursor.execute(
                    '''
                    INSERT INTO program_environment (program_id, name, value)
                    VALUES (?, ?, ?)
                    ''', (program_id, name, value)
                )
        except:
            self._db.rollback()
            raise
            
        # No exception so commit the complete add:
        self._db.commit()
        return program_id
    def delete(self, name):
        '''
            Deletes all traces of the named program from the database.
        '''
        cursor = self._db.cursor()
        
        # First get the program's id and raise an error if there is no such program.
        
        r = cursor.execute(
            '''
            SELECT id FROM program WHERE name = ?
            ''', (name,)
        )
        row = r.fetchone()
        if row is None:
            raise ValueError(f'There is no program with the name "{name}"')
        program_id = row[0]
        
        try:
            # Delete the root record:
            
            cursor.execute(
                '''
                DELETE FROM program WHERE  id = ?
                ''', (program_id,)
            )
            #  The program options:
            
            cursor.execute(
                '''
                DELETE FROM program_option WHERE program_id = ?
                ''', (program_id,)
            )
            # THe program parameters:
            
            cursor.execute(
                '''
                DELETE FROM program_parameter WHERE program_id = ?
                ''', (program_id,)
            )
            #  The environment:
            
            cursor.execute(
                '''
                DELETE FROM program_environment WHERE program_id = ?
                ''', (program_id,)
            )
        except:
            self._db.rollback()
        
        
        self._db.commit()
        
    def list(self):
        '''
        Lists the programs in the database.  The listing will be a list of dicts.  Each
        dict will have the following keys:
        
        *  id - primary key of the root record (program id)..
        *  name - name of the progtram.
        *  path - Path to the program in the container file system.
        *  host - Host the program will run in .
        *  directory  -directory in the container that will be the cwd of the program when started.
        *  container - name of the container the program runs in.
        *  more     - Dict of additionalal stuff.  This can be fed back to the 'options'
        *       parameter on the 'add' method with the exception that 'initscript'
        *       will be 'initscript_contents' and will contain the text of the initialization
        *       script.
        *  
        '''
        
    
        
        result = []
        cursor = self._db.cursor()
        
        # Get the root records:
        
        r = cursor.execute(
            '''
                SELECT program.id, name, path, type, host, directory, container, initscript FROM program
                INNER JOIN program_type ON program_type.id = type_id
                INNER JOIN container ON container.id = container_id
            '''
        )
        # Iterate over them adding additional information to the resulting dict.
        roots = r.fetchall()
        
        for root in roots:
            pgm_id        = root[0]
            name          = root[1]
            path          = root[2]
            type          = root[3]
            host          = root[4]
            wd            = root[5]
            container      = root[6]
            init_contents = root[7]
            
            program_dict = {
                'id': pgm_id, 'name': name, 'path' : path, 'host': host, 'directory': wd,
                'container' : container,
                'more': {
                    'type' : type,
                    'initscript_contents': init_contents,
                    'options' : [], 'parameters': [], 'environment' : []
                }
            }
            
            #  Fill in the program options orering ASC by primary key preserves order.
            
            c = self._db.cursor()
            c.execute(
                '''
                SELECT option, value FROM program_option WHERE program_id = ?
                ORDER BY id ASC
                ''', (pgm_id,)
            )
            opts = c.fetchall()
            for opt in opts :
                if opt[1] == '':
                    option = (opt[0],)
                else:
                     option = (opt[0], opt[1])
                program_dict['more']['options'].append(option)
            
            # Fill in the program arguments:
            
            c.execute(
                '''
                SELECT parameter FROM program_parameter 
                WHERE program_id = ? 
                ORDER BY id ASC
                ''', (pgm_id,)
            )
            params = c.fetchall()
            program_dict['more']['parameters'] = [x[0] for x in params]
            
            # Finally the environment variable:
            
            c.execute(
                '''
                SELECT name, value FROM program_environment
                WHERE program_id = ?
                ORDER BY id ASC
                ''', (pgm_id,)
            )
            env = c.fetchall()
            for (e, v) in env:
                if v == '':
                   program_dict['more']['environment'].append((e,)) 
                else:
                    program_dict['more']['environment'].append((e,v))
            # Add it to the return value:
            
            result.append(program_dict)
        
        return result
    def types(self):
        ''' 
        returns an interable containing the program typenames.
        '''
        
        cursor = self._db.cursor()
        rset = cursor.execute(
            '''
            SELECT TYPE from program_type ORDER BY type ASC
            '''
        )
        rows = rset.fetchall()
        return [x[0] for x in rows]
        
class Sequence:
    ''' 
        Implements an API to the sequence part of the database.
    
    '''
    step_increment = 10    # How much step numbers will be incremented by.
    
    def __init__(self, db):
        
        '''
          Construct 
          db - is an sqlite3 connection object open on the database we'll work with.
        '''
        self._db = db
        self._program = Program(self._db)

    def _trigger_id(self, trigger_name):
        # Convert a trigger state/transition name into a trigger/transition id:
        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT id FROM transition_name WHERE name =?
            ''', (trigger_name,)
        )
        # IF there's no match its a value error:
        
        result = r.fetchone()
        if result is None:
            raise ValueError(f'There is no transition named {trigger_name}')
        
        return result[0]
    
    def exists (self, name):
        '''
            Returns True if the sequence named exists.
        '''
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
            SELECT COUNT(*) FROM sequence WHERE name =?
            ''', (name,)
        )
        return cursor.fetchone()[0] > 0
    def add(self, name, trigger, steps):
        '''
           Adds a new sequence.
           
           name  - is the name of the new seuqence,
           trigger -is the name of the transition that triggers the sequence e.g. 'BEGIN'
           steps  - are the steps in the sequence.  These are an array of arrays.  where each triple contains:
                [0] - program name,
                [1] -  pre-delay,
                [2] - post-delay.
                
            Note that step numbers will be assigned bu this method.
            Note - steps must not have any elements after post-delay.
        
        returns the sequence ID.
        '''
        # Dont' allow a duplicat name:
        
        if self.exists(name):
            raise ValueError(f'A sequence named {name} already exists.')
        
        # Convert the trigger to a transition id:  
        
        trigger_id = self._trigger_id(trigger)
        
        # Convert the program names in to program ids...which we append as element [3] of each step.
        
        for step in steps:
            
            step.append(self._program.id(step[0]))
            if step[3] is None:
                raise ValueError(f'There is no program named {step[0]} in the steps')
        
        # Everytihng is validated, so we can do the insertions:
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
              INSERT INTO sequence (name, transition_id) VALUES (?,?)
            ''', (name, trigger_id)
        )
        seq_id = cursor.lastrowid
        
        stepno = self.step_increment
        for step in steps:
            cursor.execute(
                '''
                    INSERT INTO step (sequence_id, step, program_id, predelay, postdelay)
                    VALUES(?, ?, ?, ?, ?)
                ''',
                (seq_id, stepno, step[3], step[1], step[2])
            )
            stepno += self.step_increment
        
        self._db.commit()
        return seq_id
    
    def list(self):
        
        '''
            List all of the sequences that have been deefined and their steps.  This returns a list of dicts.  Each dict
            describes a sequence and hast the keys:
            
            id     - id of the sequence.
            name  - Name ofthe sequnce
            trigger - name of the transition that triggers the sequence
            steps   - A list of 5 elment lists:
                [0] - name of the program in the step
                [1] - pre-delay
                [2] - post -delay
                [3] - program id (primary key in 'program' table)
                [4] - step number
        '''
        result = []
        cursor = self._db.cursor()
        
        #  Fetch  all elments in the root record.
        
        cursor.execute(
            '''
            SELECT sequence.name, transition_name.name, sequence.id FROM sequence
            INNER JOIN transition_name ON transition_name.id = sequence.transition_id
            '''
        )
        sequence_roots = cursor.fetchall()
        
        
        for sequence in sequence_roots:
            seq_dict = {
                'name' : sequence[0], 'trigger': sequence[1], 'id': sequence[2],
                'steps': []
            }
            # Fetch the steps:
            
            cursor.execute(
                '''
                SELECT program.name, predelay, postdelay, program.id, step FROM step
                INNER JOIN sequence ON sequence.id = step.sequence_id
                INNER JOIN program ON program.id = step.program_id
                WHERE sequence.name = ?
                ORDER BY step.step ASC
                ''', (seq_dict['name'], )
            )
            seq_dict['steps'] = cursor.fetchall()
            result.append(seq_dict)
        
        return result