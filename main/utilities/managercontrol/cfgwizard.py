"""Contains the configuration prompting wizard - ConfigWizard.
"""
    
from PyQt5.QtWidgets import (
    QWizard, QWizardPage, QLabel, QLineEdit,
    QVBoxLayout, QHBoxLayout, QDialog
)
import os
from nscldaq.manager_client import Programs
from nscldaq.editablelist import ListToListEditor

DEFAULT_REST_SERVICE='DAQManager'
DEFAULT_OUTPUT_SERVICE='DAQManager-outputMonitor'

def _getlogin():
    """Return the login name.
    Note that in a shell under WSL with MSVisual code, os.getlogin() fails 
with the exception:
    
    Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: [Errno 6] No such device or address

    Therefore we use os.getlogin() and if it throws an exception fallback t
o os.getenv('USER').
    
    
    """
    try :
        return os.getlogin()
    except:
        return os.getenv('USER')



class IntroPage(QWizardPage):
    #  Wizard page that introduces the configuration wizard.
    #
    def __init__(self):
        super().__init__()
        
        layout = QVBoxLayout()
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            """
<h2>Instructions:</h2>
<p>
  The control panel requires some configuration.  This is expectged to be
  in the file <tt>mg_guiconfig.toml</tt> in the working directory at the
  time you run this program.
</p>
<p>
    If, as is the case now, that file does not exist, this wizard will prompt
    you for all of the information that is stored int that configuration file.
    Once you've finished the wizard, the <tt>mg_guiconfig.toml</tt>
    file will be written so you won't have to go through this again.
</p>
<p>
    You will need to know:
</p>
<h3>Connection parameters:</h3>
<ol>
    <li>Which host the manager will be run in.</li>
    <li>Which user will start the manager, if not this account</li>
    <li>If non standard service names for the ReST control and output monitor are advertised,
        the names of those services</li>
</ol>
<h3>Configuration information</h3>
<p>
   The names of programs that actually readout data.  These programs are monitored in one of the
   GUI status tabs so, if one fails, you can see which one.
</p>
<hr />
<p>
To get on with the wizard, click the <tt>Next</tt> button below.
"""
        )
        layout.addWidget(self._instructions)
        
        self.setLayout(layout)


class ConnectionPage(QWizardPage):
    #  Page to prompt for the connection parameters:
    
    def __init__(self):
        global DEFAULT_REST_SERVICE
        global DEFAULT_OUTPUT_SERVICE
        super().__init__()
        
        layout = QVBoxLayout()
        
        # Instructions:
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText("""
<h2>Instructions</h2>
<p>
    This page prompts for the manager service parameters. Note that
    default values have been filled in but may not be appropriate for your case.
    
</p>
<ul>
    <li>Manager host - is the name of the computer in which the manager is running.</li>
    <li>Manager user - is the name of the user that started the manager </li>
    <liL>Manager ReST service - is the name the ReST service the manager can be controlled with is advertised as </li>
    <li>Manager Output service - is the name the output realy service is advertised as</li>
</ul>
<hr />                                   
        """)
        layout.addWidget(self._instructions)
        
        #  mg host:
        
        host_layout = QHBoxLayout()
        self._hostlabel = QLabel("Manager host:", self)
        host_layout.addWidget(self._hostlabel)
        
        self._host = QLineEdit(self)
        self._host.setText('localhost')
        host_layout.addWidget(self._host)
        
        layout.addLayout(host_layout)
        
        # mg user:
        
        user_layout = QHBoxLayout()
        self._userlabel = QLabel('Manager user', self)
        user_layout.addWidget(self._userlabel)
        
        self._user = QLineEdit(_getlogin(), self)
        user_layout.addWidget(self._user)
        
        
        layout.addLayout(user_layout)
        
        #mg Rest Service:
        
        rest_layout = QHBoxLayout()
    
        self._restlabel = QLabel('Manager ReST service', self)
        rest_layout.addWidget(self._restlabel)
        
        self._rest = QLineEdit(DEFAULT_REST_SERVICE, self)
        rest_layout.addWidget(self._rest)
        
        layout.addLayout(rest_layout)
        # mg Ouptut service:
        
        outlayout = QHBoxLayout()
        self._outlabel = QLabel("Manager Output service", self)
        outlayout.addWidget(self._outlabel)
        
        self._outsvc = QLineEdit(DEFAULT_OUTPUT_SERVICE, self)
        outlayout.addWidget(self._outsvc)
        
        
        layout.addLayout(outlayout)
        
        
        
        
        self.setLayout(layout)
    # Attributes .. for now readonly:
    
    def host(self):
        return self._host.text()
    def user(self):
        return self._user.text()
    def rest(self):
        return  self._rest.text()
    def output(self):
        return self._outsvc.text()
    
class ReadoutPage(QWizardPage):
    #  Prompt for the set of programs that are readouts.
    
    def __init__(self, connection):
        super().__init__()
        self._connection = connection
        self._loaded = False
        # Now build the GUI:
        
        layout = QVBoxLayout()
        
        # Instructions:
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText("""
<h2>Instructions</h2>
<p>
    The GUI can monitor programs that are supposed to be persistent.  Normally, one monitors
    Readout programs, but others can be monitored as well.  Initiall, the names of all programs
    that are not transitory are loaded into the left list box below.  Using the controls,
    fill the right most listbox with the names of programs you want monitored.
</p>
<hr />                                   
        """)
        layout.addWidget(self._instructions)
        
        # The list box
        
        self._lists = ListToListEditor(self)
        layout.addWidget(self._lists)
        
        self.setLayout(layout)
    def initializePage(self):
        # If we've not done so, load the editable list.
        # Can't use hasVisited because we don't want to reload if
        # back/next and back removes us from the history.
        
        if not self._loaded:
            self._loaded = True
            connection = self._connection
            client = Programs(connection.host(), connection.user(), connection.rest())
            programs = client.status()['programs']  # Don't care about containers.
            programs = [p for p in programs  if p['type'] != 'Transitory']  # Can't monitor transitory programs.
            
            program_names = [p['name'] for p in programs]     # What we'll put in the listboxes.
            self._lists.appendSource(program_names)
    
    def programs(self):
        # Return what's been loaded in to the list box
        
        return self._lists.list()       
            
        
class ConfigWizard(QWizard):
    """Provides a self contained wizard for 
        configuring the control GUI.


    Subclass of:
       QWizard
       
    Note:
       exec - will, if necessary, modify the configuration passeed in to the constructor.
       
       
    """
    def __init__(self, config):
        super().__init__()
        self._config = config
        # Pages:
        
        self._intro = IntroPage()
        self.addPage(self._intro)
        
        self._services = ConnectionPage()
        self.addPage(self._services)
        
        self._programs = ReadoutPage(self._services)
        self.addPage(self._programs)

    def getProgramsId(self):
        return self._programsId    
    def done(self, how):
        # Called when finish or cancel is called if how is  QDialog.Accepted 
        # Edit the configuration.
        print('how', how, QDialog.Accepted)
        if how == QDialog.Accepted:
            #
            #  Connection stuff:
            
            self._config.setHost(self._services.host())
            self._config.setRest_service(self._services.rest())
            self._config.setMonitor_service(self._services.output())
            
            # Program list:
            
            self._config.setReadouts(self._programs.programs())
            
        
        super().done(how)  
        
if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication
    import sys
    from nscldaq.manager_control.config import Configuration
    
    app = QApplication(sys.argv)
    
    
    config = Configuration('/dev/null')
    wiz = ConfigWizard(config)
    wiz.exec()
    config.dump('wiztest.toml')