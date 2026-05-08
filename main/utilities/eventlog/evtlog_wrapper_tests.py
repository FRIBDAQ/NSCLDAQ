'''
Unit tests for the bits we can test in evtlog_wrapper.py.


'''
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



import unittest
import os
from pathlib import Path
import shutil

# Before we can import the eventlog_wrapper, we need to 
# define the env variables it attempts to define:
# Note that later we may modify these in individual tests:

os.environ['RECORD_PARTIAL'] = '0'
os.environ['RECORD_DEST'] = '/tmp/evtlog'
os.environ['RECORD_SRC']  = 'tcp://localhost/ron'
os.environ['RUN_NUMBER']  = '0'
os.environ['DAQBIN']      = '/usr/opt/daq/12.2-009/bin'   # We're not going to actually start it.

import eventlog_wrapper as wr



class evtlog_wrapper_tests(unittest.TestCase):
    def test_eventlog_path(self):
        path = wr._eventlog_path()
        self.assertEqual('/usr/opt/daq/12.2-009/bin/eventlog', path)
    def test_event_filename(self):
        name = wr._event_filename(123)
        self.assertEqual('run-0123-00.evt', name)
    def test_event_file_run(self):
        run = wr._event_file_run('run-0123-00.evt')
        self.assertEqual(123, run)
    def test_mktree(self):
        # This is a bit of an involved test:
        wr._make_directory_tree('/tmp/evtlog')
        
        top_dir = Path('/tmp/evtlog')
        self.assertTrue(top_dir.is_dir())  # Root is there.
        self.assertTrue(Path(top_dir, 'complete').is_dir())
        self.assertTrue(Path(top_dir, 'current').is_dir())
        experiment = Path(top_dir, 'experiment')
        self.assertTrue(experiment.is_dir())
        self.assertTrue(Path(experiment, 'current').is_dir())
        
        # Clean up top_dir and below:  
        # Simpler than walking the tree:
        
        shutil.rmtree(str(top_dir))
    def test_clean_orphan_1(self):
        wr._make_directory_tree('/tmp/eventlog')
        # Make an orphan run 1:
        
        top_dir = Path('/tmp/eventlog')
        run_dir = Path(top_dir, 'experiment', 'run1') # event file is here.
        os.makedirs(str(run_dir), exist_ok=True)
        run_file = Path(run_dir, 'run-0001-00.evt')
        run_file.touch(exist_ok=True)  # actual run file.
        os.symlink(str(run_file), '/tmp/eventlog/current/run-0001-00.evt')
        
        wr._clean_orphans('/tmp/eventlog')
        
        #  We should _not_ have the link we made in current
        
        self.assertFalse(Path(top_dir, 'current', 'run-0001-00.evt').exists())
        
        # We _should_ have one in complete..and it points to a file.
        
        self.assertTrue(Path(top_dir, 'complete', 'run-0001-00.evt').is_symlink())
        
        # We should have a bad ending marker in the run dir:
        
        self.assertTrue(Path(run_dir, 'run_improperly_ended').exists())
        
        # Clean up my mess:
        #  Need to be sure that we can do that (perms).
        
        os.system(f'chmod -R u+w {str(top_dir)}')
        shutil.rmtree(str(top_dir))
       
    def test_clean_orphan_2(self):
        # In this case the event file link is in 
        # experiment/current not current.
        
        wr._make_directory_tree('/tmp/eventlog')
        
        top_dir = Path('/tmp/eventlog')
        run_dir = Path(top_dir, 'experiment', 'run1') # event file is here.
        os.makedirs(str(run_dir), exist_ok=True)
        run_file = Path(run_dir, 'run-0001-00.evt')
        run_file.touch(exist_ok=True)  # actual run file.
        os.symlink(str(run_file), '/tmp/eventlog/experiment/current/run-0001-00.evt')
        
        wr._clean_orphans('/tmp/eventlog')
        
         #  We should _not_ have the link we made in current
        
        self.assertFalse(Path(top_dir, 'experiment',  'current', 'run-0001-00.evt').exists())
        
        # We _should_ have one in complete..and it points to a file.
        
        self.assertTrue(Path(top_dir, 'complete', 'run-0001-00.evt').is_symlink())
        
        # We should have a bad ending marker in the run dir:
        
        self.assertTrue(Path(run_dir, 'run_improperly_ended').exists())
        
        
        # Clean up my mess:
        #  Need to be sure that we can do that (perms).
        
        os.system(f'chmod -R u+w {str(top_dir)}')
        shutil.rmtree(str(top_dir))
    
if __name__ == "__main__":
    unittest.main()
    
    