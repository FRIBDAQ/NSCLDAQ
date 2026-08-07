''' 
Provides a python interface to the manager database definition.  This
will grow as more python GUIs are added, I imagine.

The use case is to provide support for PyQt user interfaces for configuration
and so on.
'''

import sqlite3

def _is_empty_table(handle: sqlite3.Connection, table_name : str) -> bool:
    cursor = handle.execute(
        f'''
            SELECT COUNT(*) as count FROM {table_name}
        '''
    )
    row = cursor.fetchone()
    if row is None:
        raise RuntimeError(f'Failed to get count of table {table_name}')
    
    return row[0] == 0

def _state_id(handle : sqlite3.Connection, state_name: str) -> int:
    # Get the state id for a state that's in the table... 
    # It's a bugcheck for there not to be a state by that name.
    
    cursor = handle.execute(
        '''
        SELECT id FROM transition_name WHERE name=?
        ''',
        (state_name,)
    )
    row = cursor.fetchone()
    if row is None:
        raise RuntimeError(f'There is no state named {state_name} in transition_name')
    return row[0]
    
def _make_Container_schema(handle : sqlite3.Connection) -> None:
    # Make the tables associated with defining containers:
    # The caller must commit this transaction.
    
    handle.execute(
        '''
        CREATE TABLE IF NOT EXISTS container  (
            id         INTEGER PRIMARY KEY,
            container  TEXT,
            image_path TEXT,
            init_script TEXT
        )
        '''
        
    )
    handle.execute(
        '''
        CREATE TABLE IF NOT EXISTS bindpoint (
            id            INTEGER PRIMARY KEY,
            container_id  INTEGER,    -- FK to container
            path          TEXT,
            mountpoint    TEXT DEFAULT NULL
        )
        '''
        
    )
def _make_Programs_schema(handle : sqlite3.Connection) -> None:
    # Create the tables that define programs.  Note that
    # The caller must commit this transaction.  The program type table
    # is populated.
    
    handle.execute('''
            CREATE TABLE IF NOT EXISTS program_type (
            id                INTEGER PRIMARY KEY,
            type              TEXT
        )
    ''')
    # If the program type table has not been populated, we must populate it.
    if _is_empty_table(handle, 'program_type'):
        handle.execute('''
            INSERT INTO program_type (type)
                VALUES ('Transitory'), ('Critical'), ('Persistent')
        ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS program (
            id           INTEGER PRIMARY KEY,
            name         TEXT,      -- Name used to refer to the program.
            path         TEXT,
            type_id      INTEGER, -- FK to program_type
            host         TEXT,
            directory    TEXT,
            container_id INTEGER, -- FK to container
            initscript   TEXT,
            service      TEXT
        )
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS program_option (
                id          INTEGER PRIMARY KEY,
                program_id  INTEGER,  -- FK to program
                option      TEXT,
                value       TEXT
            )
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS program_parameter (
            id             INTEGER PRIMARY KEY,
            program_id     INTEGER,   -- FK to program.
            parameter      TEXT
        )          
    ''')
    handle.execute('''
       CREATE TABLE IF NOT EXISTS program_environment (
                id         INTEGER PRIMARY KEY,
                program_id INTEGER,
                name       TEXT,
                value      TEXT
            ) 
    ''')
    
def _make_StateMachine_schema(handle : sqlite3.Connection) -> None:
    # Create/populate the schema for the state machine.
    
    #  Initial valid state transition table keyed by initial state name
    # contents are tuples of legal subsequent states.
    
    legalTransitions = {
        'BOOT'     : ('SHUTDOWN', 'HWINIT'),
        'SHUTDOWN' : ('SHUTDOWN', 'BOOT'),
        'HWINIT'   : ('SHUTDOWN', 'BEGIN'),
        'BEGIN'    : ('SHUTDOWN', 'END'),
        'END'      : ('SHUTDOWN', 'BEGIN', 'HWINIT')
    }
    
    handle.execute('''
        CREATE TABLE IF NOT EXISTS sequence (
            id                INTEGER PRIMARY KEY,
            name              TEXT,
            transition_id     INTEGER -- FK to transition triggering seq.
        )           
    ''')
    handle.execute('''
         CREATE TABLE IF NOT EXISTS transition_name (
            id        INTEGER PRIMARY KEY,
            name      TEXT
        )           
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS legal_transition (
            id              INTEGER PRIMARY KEY,
            from_id         INTEGER,   -- FK in to transition_name
            to_id           INTEGER    -- Also FK into transition_name
        )
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS last_transition (
            state           INTEGER     -- FK to transition_name
        )
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS step (
            id                       INTEGER PRIMARY KEY,
            sequence_id              INTEGER, -- fk to sequence
            step                     REAL,
            program_id               INTEGER, -- fk to program table.
            predelay                 INTEGER DEFAULT 0,
            postdelay                INTEGER DEFAULT 0
        )
    ''')
    #  If there's no entries in the transition_name table, stock it:
       
    if _is_empty_table(handle, 'transition_name'):
        for state in legalTransitions.keys():
            handle.execute('''
                INSERT INTO transition_name (name)
                VALUES (?)
            ''', [state,])
    
    # If there are no sequences, one named 'run_state'.
    
    if _is_empty_table(handle, 'sequence'):
        handle.execute('''
            INSERT INTO sequence (name) VALUES ('run_state')
        ''')
    
    if _is_empty_table(handle, 'legal_transition'):
        for from_name, to_names in legalTransitions.items():
            from_id = _state_id(handle, from_name)
            for to_name in to_names:
                to_id = _state_id(handle, to_name)
                handle.execute('''
                INSERT INTO legal_transition
                    ( from_id, to_id)
                    VALUES (?,?)
                ''', (from_id, to_id))
   
    # If there is no last state, set it to 'SHUTDOWN'.
    
    if _is_empty_table(handle, 'last_transition'):
        shutdown_id = _state_id(handle, 'SHUTDOWN')
        handle.execute('''
            INSERT INTO last_transition (state) VALUES (?)
        ''', (shutdown_id,))

def _make_transitionLog_schema(handle: sqlite3.Connection) -> None:
    # Make the transition log table.
    
    handle.execute('''
        CREATE TABLE IF NOT EXISTS transition_log  (
            id            INTEGER PRIMARY KEY,
            transition_id INTEGER,  -- FK to transition_name
            timestamp     INTEGER,
            success       INTEGER
        )
    ''')

def _make_eventLog_schema(handle : sqlite3.Connection) -> None:
    # Make the schema needed to describve event logging.  
    # Caller needs to commit:
    
    handle.execute('''
        CREATE TABLE IF NOT EXISTS logger (
            id                 INTEGER PRIMARY KEY,
            daqroot            TEXT,
            ring               TEXT,
            host               TEXT,
            partial            INTEGER DEFAULT 0,
            destination        TEXT,
            critical           INTEGER DEFAULT 1,
            enabled            INTEGER DEFAULT 1,
            container_id       INTEGER DEFAULT NULL -- FK to container tbl.
        )
    ''') 
    handle.execute('''
        CREATE TABLE IF NOT EXISTS recording (
            state     INTEGER
        )
    ''') 
    if _is_empty_table(handle, 'recording'):
        # Set initial recording state to off.:
        
        handle.execute('''
            INSERT INTO recording (state) VALUES (0)
        ''') 
def _make_kvStore_schema(handle : sqlite3.Connection) -> None:
    # Make the kv store and stock it with the run number and
    # title.
    
    initial_kvStore = {   # Initial store contents.
        'run': '0',
        'title' : 'Set a new title'
    }
    handle.execute('''
        CREATE TABLE IF NOT EXISTS kvstore (
            id        INTEGER PRIMARY KEY,
            keyname   TEXT,
            value     TEXT
        )
    ''')
    # If needed, stock the kv store for the first time:
    
    if _is_empty_table(handle, 'kvstore'):
        for key, value in initial_kvStore.items():
            handle.execute(
                '''INSERT INTO kvstore (keyname, value) VALUES (?,?)''',
                (key, value)
            )
def _make_userAndRoles_schema(handle : sqlite3.Connection) -> None:
    #  the user and roles are not yet used but are there for
    #  future applications where the things a particular user is allowed
    #  to do might be limited.

    handle.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id        INTEGER PRIMARY KEY,
            username  TEXT
        )
    ''')
    handle.execute('''
        CREATE TABLE IF NOT EXISTS roles (
            id       INTEGER PRIMARY KEY,
            role     TEXT
        )
    ''')
    handle.execute('''
         CREATE TABLE IF NOT EXISTS user_roles (
            user_id    INTEGER,
            role_id    INTEGER
        )
    ''')

def make_schema(handle : sqlite3.Connection) -> None:
    '''
        Creates the manager schema
        @param handle - a writable connection to an sqlit3 database
        @note  If there are already tables in the database referred to by
        'handle', the new tables are simple added to it.  
        @note since CREATE TABLE IF NOT EXISTS is used to create
             all of the tables,  if this is already a manager configuration
             database the function is a no-op.
    '''
    _make_Container_schema(handle)
    _make_Programs_schema(handle)
    _make_StateMachine_schema(handle)
    _make_transitionLog_schema(handle)
    _make_eventLog_schema(handle)
    _make_kvStore_schema(handle)
    _make_userAndRoles_schema(handle)
    
    #  None of the above actually commit their changes so:
    
    handle.commit()

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
        except Exception:
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
                if len(env) > 1:
                    value = env[1]
                else: 
                    value = ''
                cursor.execute(
                    '''
                    INSERT INTO program_environment (program_id, name, value)
                    VALUES (?, ?, ?)
                    ''', (program_id, name, value)
                )
        except Exception:
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
        except Exception:
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
        *  more     - Dict of additional stuff.  This can be fed back to the 'options'
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

    def _raiseIfNoState(self, name : str) -> int:
        # Raise a value error the state 'name' does not exist:
        # Return the state id if it does.
        result = self.stateExists(name)
        if result is None:
            raise ValueError(f'There is no state named {name}')
    
        return result
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
    def _sequenceId(self, name : str) -> int | None:
        matches = [x['id'] for x in  self.list() if x['name'] == name]
        return matches[0] if len(matches) > 0 else None
    
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
    def add(self, name : str, trigger: str, steps : list):
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
    
    def addStep(self, seq_name : str, program_name :str, predelay: int = 0, postdelay : int= 0) -> None:
        '''
            Add a step to an existing sequence:
            @param seq_naem - name of the sequence
            @param program_name - Name of the program to add.
            @param predelay - predelay, defaults to 0
            @param postdelay - postdelay, defaults to 0.
            
            @throws
                IndexError - seq_name or program_name don't exist.
            @note
                We find the number of the largest step and get the step number by adding self.step_increment to that.
                
        '''
        seqid = self._sequenceId(seq_name)
        if seqid is None:
            raise IndexError(f'There is no squence named {seq_name}')
        progid = self._program.id(program_name)
        if progid is None:
            raise IndexError(f'There is no program named {program_name}')
        
        # Figure out the step number.. if there are no steps, we use self.step_increment so:
        
        step_num = self.step_increment
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT step FROM step WHERE sequence_id = ?  ORDER BY step DESC LIMIT 1
            ''', (seqid,))
        row = cursor.fetchone()
        if row is not None:
            step_num = row[0] + self.step_increment
        
        # Now we can insert:
        
        self._db.execute('''
            INSERT INTO step (sequence_id, step, program_id, predelay, postdelay)
            VALUES(?, ?, ?, ?, ?)
            ''', (seqid, step_num, progid, predelay, postdelay))
        
        self._db.commit()
    def list(self):
        
        '''
            List all of the sequences that have been deefined and their steps.  This returns a list of dicts.  Each dict
            describes a sequence and has the keys:
            
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
    
    def deleteSequence(self, name: str) -> None:
        '''
            @param name -name of the sequence to delete.
            @throw ValueError if name is not a sequence.
        '''
        seq_def = [x for x in self.list() if x['name'] == name]
        if len(seq_def) == 0:
            raise ValueError(f'There is no sequence named {name}')
        
        seq_def = seq_def[0]
        # Remove the steps and then
        # the sequence itself:
        
        self._db.execute('''
            DELETE FROM step WHERE sequence_id = ?
        ''',(seq_def['id'],))
        self._db.execute('''
             DELETE FROM SEQUENCE WHERE id = ?            
        ''', (seq_def['id'],))
        self._db.commit()
        
    def stateExists(self, name : str) -> int | None:
        '''
            @param name - name of the state to look for.
            @return id of the state if it exists or None if it does not.
        '''
        cursor = self._db.cursor()
        
        cursor.execute('''
            SELECT id FROM transition_name WHERE name = ?
        ''', (name,))
        ids = cursor.fetchall()
        if len(ids) == 0:
            return None
        else:
            return ids[0][0]              # Assume we're not allowing dupllicates.
    def addState(self, name: str) -> None:
        '''
            Add a new state to the state machine.
            
            @param name - name of the new state.
            @throws ValueError - if the state already exists.
        '''
        if self.stateExists(name):
            raise ValueError(f'There is already a state named {name}')
        
        self._db.execute(
            '''
                INSERT INTO transition_name (name) VALUES(?)
            ''',
            (name,)
        )
        self._db.commit()
    def addTransition(self, initial :str, final: str) -> None:
        '''
            Add a legal transition:
            @param initial - initial state name.
            @param final   - final state name.
            @throw ValueError if either initial or final don't exist or
                    the transition is already defined.
        '''
        
        from_id = self._raiseIfNoState(initial)
        to_id = self._raiseIfNoState(final)
        
        # Does the transition already exist?
        
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT COUNT(*) FROM legal_transition WHERE from_id = ? AND to_id =?
        ''', (from_id, to_id))
        result = cursor.fetchone()  
        if result[0] != 0:
            raise ValueError(f'The state transition {initial} -> {final} is already defined')
        
        # Insert the new transition:
        
        self._db.execute('''
            INSERT INTO legal_transition (from_id, to_id) VALUES(?,?)                         
        ''', (from_id, to_id))
        self._db.commit()
        
    def removeTransition(self, initial: str, final: str) -> None:
        '''
            Remove the legal transition descsribed by:
            @param initial - the name of the initial state.
            @param final   - the final state name.
            @throw ValueError if either initial or final are not defined.  
            @note it is a no-op to delete a transition that is not legal.
        '''
        
        from_id = self._raiseIfNoState(initial)
        to_id   = self._raiseIfNoState(final)
        
        self._db.execute('''
            DELETE from legal_transition WHERE from_id = ? AND to_id = ?
        ''', (from_id, to_id))
        self._db.commit()
        
    def listStates(self) ->list:
    
        '''
            @return list[str] - lits of known state names.
        '''
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT name FROM transition_name
        ''', tuple())
        
        # Marshall the results:
        
        result =list()
        for row in cursor.fetchall():
            result.append(row[0])
        return result
    
    def legalFromStates(self, name : str) -> list:
        ''''
            List the names of the legal predecessor states
            to the named state.
            @param name - the successor state we're asking about.
            @return list[str] - list or state names that can transition  to 'name'
            @throw ValueError - if name is not a state:
        '''
        to_id = self._raiseIfNoState(name)
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT name FROM legal_transition INNER JOIN
                transition_name ON legal_transition.from_id = transition_name.id
                WHERE legal_transition.to_id = ?
        ''', (to_id,))
        
        result = list()
        for row in cursor.fetchall():
            result.append(row[0])
        
        return result
    def legalSuccessorStates(self, name : str) -> list:
        '''
            Determine the successor states for the named state:
            @param name - name of the state whose successor states we want.
            @return list[str] -state names we can transition to.
            @throw ValueError if name is not a valid state.
        '''
        from_id = self._raiseIfNoState(name)
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT name FROM legal_transition INNER JOIN
                transition_name ON legal_transition.to_id = transition_name.id
                WHERE legal_transition.from_id = ?''',
            (from_id,))
        
        result = list()
        for row in cursor.fetchall():
            result.append(row[0])
        return result            
    
    def deleteState(self, name):
        '''
            Remove a state from the database.  Note this also removes transitions
            from and to the state.
            
            @param name - the name of the state to remove.
            @throw ValueError - if the state is not defined.
        '''
        # do the entire operation  in a save point. in case there are errors.
        self._db.execute('SAVEPOINT deleting_state', tuple())
        try:
            self._raiseIfNoState(name)
            
            # Delete predecessors:
            
            predecessors = self.legalFromStates(name)
            for from_state in predecessors:
                self.removeTransition(from_state, name)
                
            # Delete successors:
            
            successors = self.legalSuccessorStates(name)
            for to_state in successors:
                self.removeTransition(name, to_state)
            self._db.execute('DELETE FROM transition_name WHERE name = ?', (name,))
        except Exception as e:
            self._db.execute('ROLLBACK TO SAVEPOINT deleting_state')
            raise       # Propagate the exception
         
        # Some inner calls do commits.:
        try:
            self._db.execute('RELEASE SAVEPOINT deleting_state', tuple())   
        except Exception:
            pass
        try:
            self._db.commit()       # May need to commit the outer xaction.
        except Exception:
            pass
      
class KvStore:
    ''' Implement access to the key value store: '''  
    
    def __init__(self, db : sqlite3.Connection):
        self._db = db
    
    def exists(self, key : str) -> bool:
        '''
        @return bool - True if 'key' exists in the database
        
        '''
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT COUNT(*) FROM kvstore WHERE keyname = ?
                       ''', (key,))
        result = cursor.fetchone()
        assert result is not None, f'Unable to determine if {key} is in the KvStore'
        return result[0] != 0
    
    def create(self, key: str, value: str) -> None:
        '''
            @param key -  a new key to create, must be new.
            @param value - the value to give that key.
            @throws If the key already exists, ValueError is thrown
            
        '''
        if self.exists(key):
            raise ValueError(f'{key} already exists')
        
        self._db.execute('''
            INSERT INTO kvstore (keyname, value) VALUES (?,?)
            ''', (key, value))
        self._db.commit()
    
    def modify(self, key : str, value : str) -> None:
        '''
            @param key - key to modify.
            @param value - New value for th key.
            @throws ValueError if the key does not exist.
        '''
        if not self.exists(key):
            raise ValueError(f'{key} has not been defined, use the "create" method to do  so.')
        
        self._db.execute('''
                UPDATE kvstore SET value = ? WHERE keyname = ?
            ''', (value, key))
        self._db.commit()
        
    def remove(self, key : str) -> None:
        '''
           @param key - the key to delete.
           @throws ValueError if the key does not exist.
        '''
        if not self.exists(key):
            raise ValueError(f'{key} does not exist.')
        
        self._db.execute('''
            DELETE FROM kvstore WHERE keyname = ?
            ''', (key,))
        self._db.commit()
    
    def get(self, key : str) -> str:
        '''
          @param key - key whose value must be retrieved.
          @return str - the value of the key.
          @raise ValueError if there is no such key.
        '''
        cursor = self._db.cursor()
        cursor.execute('''
               SELECT value FROM kvstore WHERE keyname = ?                            
            ''', (key,))
        
        row = cursor.fetchone()
        if row is None:
            raise ValueError(f'{key} does not exist.')
        
        return row[0]

    def listKeys(self) -> list[str]:
        '''
            @return list[str] - list of the defined keys.
        '''
        
        cursor = self._db.cursor()
        cursor.execute('''SELECT keyname FROM kvstore''', tuple[str]())
        rows = cursor.fetchall()
        if rows is None:
            return list()
        
        return [r[0] for r in rows]
        
    def listAll(self) -> list[tuple[str,str]]:
        '''
            @return list[tuple[str,str]]  list of all key value pairs in the store.
                The first element of each tuple is the key, the second, its value.
        '''
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT keyname, value FROM kvstore
                       ''', tuple[str]())
        
        result = cursor.fetchall()
        if result is None:
            return []
        return result
        
class Auth:
    ''' 
    Note that while user/and role restrictions are not yet implemented/enforced,
    the mechanisms for defining them are.
    There are three tables we're concerned with:
    1. roles which defines the set of roles in the system.
    2. users which defines the users in the experiment
    3. user_roles which defines the set of roles a user has.
    
    We support:
    - Defining and removing roles.
    - Adding and removing users.
    - Granting and revoking roles to a user.
    
    Getting this information.
    '''
    def __init__(self, db: sqlite3.Connection):
        self._db = db
    
    def role_id(self, role : str) -> int | None:
        '''
        @param role : str - the name of a role to query for.
        @return int | None- If the role exists, returns its id. 
        @retval None  no such role.
        '''
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT id FROM roles WHERE role = ?
        ''', (role, ))
        row = cursor.fetchone()                   # We will enforce uniqueness.
        return row[0] if row else None
    
    def addRole(self, role : str) -> None:
        '''
            @param role : str- The new role to add.
            @throws ValueError if there's already sucha a row.
            
        '''
        if self.role_id(role):
            raise ValueError(f'{role} is an existing role.  Refusing to make a duplicate')
    
        self._db.execute('''
            INSERT INTO roles (role) VALUES(?)
        ''', (role,))
        
        self._db.commit()
    
    def removeRole(self, role : str) -> None:
        '''
            @param role : str - the role to remove.
            @note all user_roles table entries with that role are also removed.
            @throws KeyError if the role does not exist.
        '''
        
        role_id = self.role_id(role)
        if not role_id:
            raise KeyError(f'There is no role named {role}')
        
        # This is already a transaction so just commit at the end.
        
        self._db.execute('''
            DELETE FROM user_roles WHERE role_id = ?
        ''',  (role_id,))
        
        self._db.execute('''
            DELETE FROM roles WHERE id = ?
                         ''', (role_id,))
        
        self._db.commit()
        
    def user_id(self, username : str) -> int | None:
        '''
        @param username : str - a username to hunt for.
        @return int | None - the user id of the user.
        @retval None - there is no such user.
        '''
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT id FROM users WHERE username = ?
            ''', (username, ))
        row = cursor.fetchone()
        return row[0] if row else None
    
    def addUser(self, username: str) -> None:
        '''
        @param username : str - name of new user to add.
        @throws ValueError if the user already exists.
        '''
        if self.user_id(username):
            raise ValueError(f'{username} is already a user. Not going to make a duplicate')

        self._db.execute('''
            INSERT INTO USERS (username) VALUES (?)
        ''', (username,))
        
        self._db.commit()
        
    def removeUser(self, username : str) -> None:
        '''
            @param username : str - Name of the user to remove.
            @note The entries in the user_roles table that pertain to this user are also removed.
            @throws KeyError if the username does not exist.
        '''
        id = self.user_id(username)
        if not id:
            raise KeyError(f'There is no user named {username}')
        
        self._db.execute('''
            DELETE FROM user_roles WHERE user_id = ?
            ''', (id,))
        self._db.execute('''
            DELETE FROM users WHERE id=?
            ''', (id, ))
        
        self._db.commit()
        
    def grant(self, role_name : str, username: str) -> None:
        '''
            @parm role_name : str - name of the role to grant to
            @param username  :str - the username getting the role.
            @throws KeyError if either rol_name or username don't exist.
            @throws ValuError If the user already has the role.
            
        '''
        uid = self.user_id(username)
        if not uid:
            raise KeyError(f'There is no user named {username}')
        role = self.role_id(role_name)
        if not role:
            raise KeyError(f'There is no role named {role_name}')
        
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT COUNT(*) FROM user_roles WHERE user_id = ? AND role_id = ?
            ''', (uid, role))
        row = cursor.fetchone()
        if row[0] != 0:
            raise ValueError(f'{username} has already been granted {role_name}')        
        
        # Now finally we can grant the role:
        
        self._db.execute('''
              INSERT INTO user_roles (user_id, role_id) VALUES (?, ?)      
        ''',(uid, role))
        
        self._db.commit()
        
    def revoke(self, role_name : str, username : str) -> None:
        '''
            @param role_name : str - name of the role to revoke.
            @param username  : str - name of the user from whom it is revoked.
            @raise KeyError - if either the user or the role is not defined.
            @raise ValueError - If the user doesn't have the role to begin with.
        '''
        
        uid = self.user_id(username)
        if not uid:
            raise KeyError(f'There is no user named {username}')
        rid = self.role_id(role_name)
        if not rid:
            raise KeyError(f'There is no role named {role_name}')
        
        # Does the user have the rol to begin with:
        
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT COUNT(*) FROM user_roles WHERE user_id = ? AND role_id = ?
            ''', (uid, rid))
        row = cursor.fetchone()
        if row[0] == 0:
            raise ValueError(f'{username} was never granted {role_name}')        
        
        # Finally, Revoke the role:
        
        self._db.execute('''
            DELETE FROM user_roles WHERE user_id = ? AND role_id = ?
            ''', (uid, rid))
    
    # Queries:
    
    def listUsers(self) -> list[str]:
        '''
            @return list[str] - list of valid usernames.
        '''
        cursor = self._db.cursor()
        cursor.execute(
            '''
            SELECT username FROM users
            ''')
        rows = cursor.fetchall()

        return [row[0] for row in rows]
    
    def listRoles(self) -> list[str]:
        '''
            @return list[str] - list of all the defined roles.
        '''
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
                SELECT role FROM roles
            '''
        )
        rows = cursor.fetchall()
        return [row[0] for row in rows]
    
    def grantedRoles(self, user : str) -> list[str]:
        '''
        @param user : str - username to check
        @return list[str] - A list of the roles this user has been granted.
        @raises KeyError if the user is not valiid.
        '''
        
        userid = self.user_id(user)
        if not userid:
            raise KeyError(f'{user} is not a defined username.')
        
        cursor = self._db.cursor()
        cursor.execute('''
            SELECT roles.role FROM user_roles
            INNER JOIN roles ON roles.id = user_roles.role_id
            WHERE user_roles.user_id = ?
            ''', (userid,))
        rows = cursor.fetchall()
        
        return [row[0] for row in rows]
        
        
#    This software is Copyright by the Board of Trustees of Michigan
    
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321

