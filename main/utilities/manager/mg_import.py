#!/usr/bin/env python3

from nscldaq.mg_database import (
    Container, EventLog, Program, Sequence,
)
import sqlite3

import sys






def qualify_name(name, qualification):
    return f'{qualification}-{name}'


#   Import the containers from the source db:

def import_containers(inp, name, out):
    in_container_api = Container(inp)
    out_container_api = Container(out)
    
    #  Get the definitions to import: and cook the names:
    
    containers = in_container_api.list()
    for c in containers:
        c['name'] = qualify_name(c['name'], name)
        
    # Now add the definitions to the exp database...we're going to have to cook the definition
    # a bit since we only have the initscript ctonetns, not the initscript
    
    for c in containers:
        out_container_api.add(c['name'], c['image'], None, c['bindings'])
        
        # Update the  input script:
        
        cursor = out.cursor()
        cursor.execute(
'''
    UPDATE container SET init_script=? WHERE container = ?
    
''', (c['init_script'], c['name'])
        )
    out.commit()
    
#
#  Import the programs from the database connected to inp 
# into the database connected with out
# Prefix the names of the programs with 'name'.
#
def import_programs(inp, name, out):
    in_pgm_api = Program(inp)
    out_pgm_api = Program(out)
    
    programs = in_pgm_api.list()
    
    #  Note we'll have to do a direct insert of the  init script6
    #  since we don't have the filename....and it might not even exist.
    
    for program in programs:
        program['name'] = qualify_name(program['name'], name)    # Munge the name.
        out_pgm_api.add(
            program['name'], program['path'], program['host'],  
            qualify_name(program['container'], name), program['directory'], program['more']
        )   # note we need to qualify the container name...
        # No w put the init script content into the root recrod:
        
        cursor = out.cursor()
        cursor.execute(
            '''
            UPDATE program SET initscript=? WHERE id = ?
            ''',(program['more']['initscript_contents'], program['id'])
        )
        out.commit()     # COmmit this modification.
                        


def import_eventloggers(inp, name, out):
    # Import event loggers from the database connected to inp to the database
    # connected to out - use the 'name' to prefix/uniquify things.
    # Note that there might be loggers that duplicate the destiniation
    #  We report them but continue.
    
    in_api = EventLog(inp)
    out_api = EventLog(out)
    
    loggers = in_api.list()
    
    for logger in loggers:
        # Qaulify the container name and make the options:
        
        container = qualify_name(logger['container'], name)
        options = {
            'partial' : logger['partial'],
            'critical' : logger['critical'],
            'enabled': logger['enabled']
        }
        try:
            out_api.add(logger['root'], logger['ring'], logger['destination'], container, logger['host'], options)
        except Exception as e:
            sys.stderr.write(f"Could not import event logger  from {logger['ring']} to {logger['destination']} {str(e)}\n")


def import_sequences(inp, name, outp):
    in_api = Sequence(inp)
    out_api = Sequence(outp)
    
    #  Get the sequence definitions and qualify the sequence and program names.
    
    sequences = in_api.list()
    for seq in sequences:
        seq['name'] = qualify_name(seq['name'], name)
        steps = seq['steps']
        seq['steps'] = []
        for step in steps:
            pgm_name = qualify_name(step[0], name)
            seq['steps'].append([pgm_name, step[1], step[2]])
    
        out_api.add(seq['name'], seq['trigger'], seq['steps'])
    
#---------------------------------------------- main ----------------------------------
if len(sys.argv) != 4:
    sys.stderr.write('''
Usage:
    mg_import input_db name exp_db
    
Where:
    input_db - is the experiment database to be imported into this experiment.
    name     - Is the imported experiment name.  This is used to uniquify names from the imported experiment.
    exp_db   - Is the experiment database into which the definitions in input_db are imported.
         
    ''')
    sys.exit(-1)
        

# open the databases and  get the import name.
    
    
db_to_import  = sqlite3.connect(sys.argv[1])
import_name   = sys.argv[2]
exp_db        = sqlite3.connect(sys.argv[3])


import_containers(db_to_import, import_name, exp_db)
import_programs(db_to_import, import_name, exp_db)
import_eventloggers(db_to_import, import_name, exp_db)
import_sequences(db_to_import, import_name, exp_db)

                                             