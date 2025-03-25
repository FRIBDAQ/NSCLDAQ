#!/usr/bin/env python3 
'''
This script provides a Qt5 wizard for making 
programs.  Programs have:

A name, a host, a container, and an executable.

They have a type which might be:
e.g. 'Critical.

They have an execution environment that consists of:

A working directory, environment variables, an initialization script.

They are parameterized by options which typically (but need not) have values.
And parameters which are values.






Usage:
   $DAQBIN/mg_program_wizard database-file

'''
import sys
import sqlite3
from nscldaq.mg_database import Program, Container
from pathlib import Path

from PyQt5.QtWidgets import (
    QWizard, QApplication, QWizardPage, QLineEdit, QPushButton, QFileDialog,
    QComboBox, QLabel, QTableWidget, QMessageBox,
    QHBoxLayout, QVBoxLayout
)

from PyQt5.Qt import *


#--------------------------------------------------------------------------------------------
#  GUI code:


class EditableTable(QWidget):
    #  Megawidget that has a table that can be edited.
    #  Editing consists of supported deletion of the current row,
    #  Adding rows to the end, and editing the entries.
    # 
    #  Layout is:
    #   
    #    +--------------------------+
    #    |    The table             | [+]
    #    |                          | [x]
    #    +--------------------------+
    #
    #   The table() method returns the table widget.
    #   this allows for things like setting the headers, the # of colomns
    #   loading it etc.
    #
    def __init__(self, *args):
        super().__init__(*args)
        layout = QHBoxLayout()
        
        self._table = QTableWidget(self)
        defaultItem = QTableWidgetItem('')
        self._table.setItemPrototype(defaultItem)
        layout.addWidget(self._table)
        
        #  The buttons:
        
        buttonlayout = QVBoxLayout()
        self._addrow = QPushButton('+', self)
        buttonlayout.addWidget(self._addrow)
        self._deleterow = QPushButton(self)
        self._deleterow.setIcon(self.style().standardIcon(getattr(QStyle, 'SP_DialogCancelButton')))
        buttonlayout.addWidget(self._deleterow)
        
        
        layout.addLayout(buttonlayout)
        
        self.setLayout(layout)    
        self._addrow.clicked.connect(self._addRow)
        self._deleterow.clicked.connect(self._deleteRow)
        
    def _addRow(self):
        self._table.setRowCount(self._table.rowCount() + 1)
        new_row = self._table.rowCount() - 1
        cols = self._table.columnCount()
        for c in range(cols):
            self._table.setItem(new_row, c, self._table.itemPrototype().clone())
    def _deleteRow(self):
        row = self._table.currentRow()
        self._table.removeRow(row)
        
    def table(self):
        ''' Returns the table itself. '''
        return self._table
    def col0List(self):
        ''' Returns a list of the values in column 0 with empties suppressed: '''
        
        table = self._table
        rows  = table.rowCount()
        return [table.item(r, 0).text()
                for r in range(rows)
                if len(table.item(r,0).text()) > 0 
                and not table.item(r, 0).text().isspace()]

    def getPairs(self):
        '''
           Return a list of col0, col1 pairs with
           Elements that have no col0 value omitted.
           The result is a list of two element lists for cases
           when both columns are filled in and one element lists if the
           col1 is not filled in.
        '''
        
        table = self._table
        rows = table.rowCount()
        result = []
        
        rawoptions = [[table.item(r, 0).text(), table.item(r, 1).text()] 
                      for r in range(rows)
                      if len(table.item(r, 0).text()) > 0
                      and not table.item(r, 0).text().isspace()
                      ]
        for (option, value) in rawoptions:
            if len(value) == 0 or value.isspace():
                result.append((option,))
            else:
                result.append((option, value))
        return result
        
        
class IdentificationPage(QWizardPage):
    def __init__(self, db, *args):
        super().__init__(*args)
        self._db = db
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
Welcome to the program creation wizard.<br/>
<p>
   In the managed data acquisition, programs are executable code, or
scripts that run in some host in a container.   This wizard will lead you
through the creation of a program.  Note that mg_config  features a program
editor that you can use to modify the program, once created or create new programs,
once you become a bit more confident in the process.
</p>
<p>
    On this page you will select
    <ul>
    <li>A unique name by which the program will be referred to within the system.</li>
    <li>The executable for the program. Note the path to this executable must be valid within
    the host/container in which it runs. </li>
    <li>host in which the program runs</li>
    <li>The container in which the program runs in that host</li>
    </ul>
</p>
<hr/>
            '''
        )
        
        layout = QVBoxLayout()
        layout.addWidget(self._instructions)
        
        # Name and program image:
        
        id = QHBoxLayout()
        
        self._namelbl = QLabel('Program Name:', self)
        id.addWidget(self._namelbl)
        self._name = QLineEdit(self)
        id.addWidget(self._name)
        
        self._pgrmlbl = QLabel('Executable:', self)
        id.addWidget(self._pgrmlbl)
        self._pgm = QLineEdit(self)
        id.addWidget(self._pgm)
        self._pgmchooser = QPushButton('Browse...',self)
        id.addWidget(self._pgmchooser)
        
        layout.addLayout(id)
        
        host = QHBoxLayout()
        self._hostlbl = QLabel('Host IP:', self)
        host.addWidget(self._hostlbl)
        self._hostname = QLineEdit(self)
        host.addWidget(self._hostname)
        layout.addLayout(host)
        
        c = Container(self._db)
        containers = [x['name'] for x in c.list()]
        container = QHBoxLayout()
        self._containerlbl = QLabel('Container:', self)
        container.addWidget(self._containerlbl)
        self._container = QComboBox(self)
        self._container.addItems(containers)
        container.addWidget(self._container)
        
        layout.addLayout(container)
        
        self.setLayout(layout)
        
        self._pgmchooser.clicked.connect(self._browse)
    
    def _browse(self):
        #  Browse for a program executable.
        
        dialog = QFileDialog(self)
        dialog.setAcceptMode(QFileDialog.AcceptOpen)
        dialog.setFileMode(QFileDialog.AnyFile)
        dialog.exec()
        
        dest = dialog.selectedFiles()
        if len(dest) != 0:
            self._pgm.setText(dest[0])
    
    #  Accessors:
    
    def program(self):
        return self._pgm.text()
    def name(self):
        return self._name.text()
    def host(self):
        return self._hostname.text()
    def container(self):
        return self._container.currentText()    
        

class TypeAndWdPage(QWizardPage):
    def __init__(self, db, *args):  
        super().__init__(*args)
        #
        #  Get the list of program types:
        #
        p = Program(db)
        types = p.types()
        
        layout = QVBoxLayout()
        
        #  The instructions:
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        # Note that program types below must be updated if additional types are added
        self._instructions.setText(
            '''
            <p> On this page we will set the working directory in effect when the program
            starts.  That directory must be a valid directory in the container in which the
            program is running.  We will also specify the program type.
            </p>
            <p>
              Program types are used to describe to the system what you want to happen
              if the program exits.  The following types are defined 
            </p>
            <ul>
            <li><b>Critical</b> - Critical programs are considered essential to the data acquisition
            system.  If a critical program exits, the experiment shuts down and has to be rebooted.
            For example, an event builder data sourcde is typically critical.
            </li>
            <li><b>Persistent</b> - Persistent programs are not critical but are expected to run for the
            life of the experiment.  If a critical program exits, a message will be emitted to the 
            output window <em>but</em> the experiment continues to run.  For example, a histogrammer
            failure does not need to shutdown the experiment.  Typically the current run can be cleanly
            ended and then the experiment rebooted to restart it, or a histogrammer external to the managed
            system can be attached to the data flow (interactively) and the experiment can continue
            to run until it's convenient to restart everything.
            </li>
            <li><b>Transitory</b> - Transitory programs are expected to run for some short time and then
            exit.  WHen they exit no special action is taken by the experiment manager. An example of a 
            transitory program are the programs that interact with readout programs to tell them to start and
            stop data taking.  These are clients of the Readout's REST plugin and simply send a REST request
            to an associated Readout to tell them to do something and then exit.
            </li>
            </ul>
            '''
        )
        layout.addWidget(self._instructions)
        
        #  Current working directory has a browse operation:
        
        wdlayout = QHBoxLayout()
        self._wdlabel = QLabel('Working Directory: ', self)
        wdlayout.addWidget(self._wdlabel)
        
        self._wd     = QLineEdit(self)
        wdlayout.addWidget(self._wd)
        
        self._wdbrowse = QPushButton('Browse...', self)
        wdlayout.addWidget(self._wdbrowse)
        
        layout.addLayout(wdlayout)
        
        # Combobox to select the program type:
        
        typelayout = QHBoxLayout()
        self._typelbl = QLabel('Program Type: ', self)
        typelayout.addWidget(self._typelbl)
        self._type = QComboBox(self)
        self._type.addItems(types)
        typelayout.addWidget(self._type)
        
        layout.addLayout(typelayout)
        
        
        self.setLayout(layout)
    # Accessors:
    def program_type(self):
        return self._type.currentText()
    def wd(self):
        return self._wd.text()
        
class IniScriptAndOptions(QWizardPage):
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
            <p>
                Now we will set up the way the program is called and its environment.
                is started and its environment.
            </p>
            <ul>
                <li>
                    <b>initscriptK/b> - This is an optional shell script that is run prior
                     to starting your program.   This can be used to perform complex setup operations
                     or source scripts containing environment variables like a <tt>daqsetup.bash</tt>
                     script.  Note that the script file path is interpreted in the context of this
                     wizard and the contents of that file are sucked into the configuration databse.
                </li>
                <li>
                    <b>options</b> - These are pairs of command parameters that are passed to the program on 
                    its invocation command line.  For example <tt>--sourceid 0</tt> would be an option
                    named <tt>--sourceid</tt> with mthe value <tt>0</tt>
                </li>
                <li>
                    <b>parameters</b> - These are command line parameters.  For example the names of files
                    needed by the program.  If you are passing file paths to a program they must make sense in
                    the contenxt of the container and host in which you have chosen to run the program.
                </li>
                <li>
                    <b>environmen</b>  - The program environment is a set of name/value pairs.
                    The <tt>getenv(3)</tt> library function can be used to retrieve the values of
                    an environment variable.  Each environment variable has a name and a value, for
                    example <tt>DAQROOT</tt> might have the value <tt>/usr/opt/daq/12.1-003</tt>
                </li>
            </ul>
            <p>
            This page and subsequent ones will allow you to specify these bits of the program environment.
            </p>
            '''
        )
        layout.addWidget(self._instructions)
        
        iniscript = QHBoxLayout()
        self._iniscrlabel = QLabel('Init script file: ', self)
        iniscript.addWidget(self._iniscrlabel)
        
        self._initscript = QLineEdit(self)
        iniscript.addWidget(self._initscript)
        
        self._inibrowse = QPushButton('Browse...', self)
        iniscript.addWidget(self._inibrowse)
        layout.addLayout(iniscript)
        
        self._optlabel = QLabel('Program options', self)
        layout.addWidget(self._optlabel)
        self._options = EditableTable(self)
        self._optiontable = self._options.table()
        self._optiontable.setColumnCount(2)
        self._optiontable.setHorizontalHeaderLabels(['Option', 'Value'])
        layout.addWidget(self._options)
       
        self.setLayout(layout)
        self._inibrowse.clicked.connect(self.browse)
        
    def browse(self):
        dialog = QFileDialog(self)
        dialog.setAcceptMode(QFileDialog.AcceptOpen)
        dialog.setFileMode(QFileDialog.ExistingFile)
        dialog.exec()
        
        files = dialog.selectedFiles()
        if len(files) > 0:
            self._initscript.setText(files[0])
    
    # Accessors:
    
    def initscript(self):
        return self._initscript.text()
    def options(self):
        # Factor this into the editable table
        # Along with one for single col tables.
        
        #  Returns a list of name/value pairs or just name if there's no value.
        #   We remove those for which the name is empty.
        
        return self._options.getPairs()
        
        
class ParameterPage(QWizardPage):
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
            <p>
                Fill in the table below with the ordered list of parameters that will
                be added to the program command line after the options filled in on the
                previous page.
            </p>
            '''
        )
        layout.addWidget(self._instructions)
        
        self._paramlabel = QLabel('Parameters:', self)
        layout.addWidget(self._paramlabel)
        
        self._paramtable = EditableTable(self)
        self._parameters = self._paramtable.table()
        self._parameters.setColumnCount(1)
        self._parameters.setHorizontalHeaderLabels(['Parameter',])
        layout.addWidget(self._paramtable)
        
        self.setLayout(layout)
    # Accessors:
    
    def parameters(self):
        return self._paramtable.col0List()

    
class Environment(QWizardPage):    
        def __init__(self, *args):
            super().__init__(*args)
            layout = QVBoxLayout()
            
            self._instructions = QLabel(self)
            self._instructions.setWordWrap(True)
            self._instructions.setText(
                '''
                <p>
                Fill in the table below with environment variable names and their values.
                </p>
                '''
            )
            layout.addWidget(self._instructions)
            
            self._envlabel = QLabel('Environment: ', self)
            layout.addWidget(self._envlabel)
            self._envtable = EditableTable(self)
            self._env = self._envtable.table()
            self._env.setColumnCount(2)
            self._env.setHorizontalHeaderLabels(['Variable', 'Value'])
            layout.addWidget(self._envtable)
            
            self.setLayout(layout)
        def environment(self):
            return self._envtable.getPairs()
            
class ProgramWizard(QWizard):
    def __init__(self, db):
        super().__init__()

        self._ident = IdentificationPage(db, self)
        self.addPage(self._ident)
        
        self._wdtype = TypeAndWdPage(db, self)
        self.addPage(self._wdtype)
        
        self._iniopts = IniScriptAndOptions(self)
        self.addPage(self._iniopts)
        
        self._params = ParameterPage(self)
        self.addPage(self._params)

        self._env = Environment(self)
        self.addPage(self._env)
    
    # Accessors:
    
    def program(self):
        return self._ident.program()
    def name(self):
        return self._ident.name()
    def host(self):
        return self._ident.host()
    def container(self):
        return self._ident.container()
    
    def program_type(self):
        return self._wdtype.program_type()
    def wd(self):
        return self._wdtype.wd()
    
    def initscript(self):
        return self._iniopts.initscript()
    def options(self):
        return self._iniopts.options()
    
    def parameters(self):
        return self._params.parameters()
    
    def environment(self):
        return self._env.environment()

#-------------------------------------------------------------------------------------------


#
def Usage():
    sys.stderr.write(
        '''
Usage:
    $DAQBIN/mg_program_wizard config-file-path
Where:
    config-file-path - is the path to the configuration file to edit.
        '''
    )
    sys.exit(-1)
    
    
    
# Handle cancel or premature close:

def abort():
    exit(0)

#  Entry point:

if len(sys.argv) != 2:
    Usage()
    
config = sys.argv[1]
p = Path(config)
if not p.exists():
    sys.stderr.write(f'No such config file "{config}"\n')
    exit(-1)
    
    
db = sqlite3.connect(config)

# Do the wizard:

app = QApplication(sys.argv)
wizard = ProgramWizard(db)
wizard.show()
wizard.rejected.connect(abort)
app.exec()

executable = wizard.program()
name       = wizard.name()
host       = wizard.host()
container   = wizard.container()
type      = wizard.program_type()
cwd        = wizard.wd()

iniscript  = wizard.initscript()
options    = wizard.options()
params     = wizard.parameters()
env        = wizard.environment()

#  marshall up the options argument to Program.add():

program_options = {
    'tyoe': type,
    'initscript': iniscript,
    'options' : options,
    'parameters': params,
    'environment': env
}

#  We'll add the item in a try/except block so that the error can be posted as a  MessageBox.:

pgmdb = Program(db)
try:
    pgmdb.add(name, executable, host, container, cwd, program_options)
    exit(0)
except Exception as e:
    QMessageBox.critical(wizard, 'Creation Failed', f'Failed to make the program : {str(e)}')
    raise
    exit(-1)