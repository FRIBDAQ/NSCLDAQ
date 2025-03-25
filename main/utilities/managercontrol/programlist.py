"""
Provide a program listing widget.  This is used to provide information abou8t
programs - whether they are running or not and their type.

We have two classes here, though only one of them should be considered public:

ProgramModel - contains is a StandardItemModel that dessribes the
    current set of monitored programs and their states.  It knows how to
    update itself given the dict description of program gotten from the 
    API.    This need not be instantiated by the client.
ProgramView - TableView widget that contains a ProgramModel for its data.
   ProgramView automatically instantiates a ProgramModel which is why user code
   generall need not do so.
"""


from PyQt5.QtWidgets import (QTableView, QAbstractItemView)
from PyQt5.QtGui import (QStandardItemModel, QStandardItem)
from PyQt5.QtCore import Qt


class ProgramModel(QStandardItemModel):
    """_summary_
        This is a standard item model that provides information about monitored programs.
        The columns are as  follows:
        
        Name  - name of the program.
        Host  - Where the program runs.
        Container - name of the container it runs in
        Type  - Program type (e.g. Critical).
        Active - Is program active.
        
        The intention is that this model can be incorporated into ProgramView to supply its data
        
    There's only one public method of note and that's:
        update - which should be handed the results of a Programs.status() call.
    Base class:
        QStadnardItemModel (_type_)
    """
    def __init__(self, *args):
        """ Constructs. *args are any arguments you might want passed
        to the constructor of QStandardItemModel.
        """
        super().__init__(*args)
        self._filter = None
    def setFilter(self, programs):
        """Provide the list of programs to show.  None means all are shown.

        Args:
            programs (list): List of programs to show.
        """
        self._filter = programs
        
    def update(self, data):
        """Update the model data from data returned from Programs.status().  Note, since the table view 
          is not going to be editable, we just do a clear() and restock rather than trying to be cute and smart.
          If performance is a problem, then we can change this later.

        Args:
            data (dict): Data from Program.status() we only care about the data in the 'programs' key.
        
        TODO: 
            Edit in place because what clear/install does is undo any column width changes the user die
             to make things readable.
        """
        self.clear()
        programs = data['programs']
        for program in programs: 
            # Build the row:
            
            if self._filter is None or program['name'] in self._filter:
                name = QStandardItem(program['name'])
                host = QStandardItem(program['host'])
                container = QStandardItem(program['container'])
                type = QStandardItem(program['type'])
                active = QStandardItem('X' if program['active'] else ' ')
                
                self.appendRow([name, host, container, type, active])            
    
    def headerData(self, col, orient, role):
        # Needed to provide headers to the view.
        
        headers = ('Name', 'Host', 'Container', 'Type', 'Active')
        if role == Qt.DisplayRole:
            if orient == Qt.Horizontal:
                return headers[col]
        return None

class ProgramView(QTableView):
    """
        Table with appropriate headers and a ProgramModel model.  The model gets its data by e.g.
        
        p = manager_client.Programs(...)
        v = ProgramView()
        v.model().update(p.status())

    BaseClass:
        QTableView 
    """
    def __init__(self, *args):
        """Constructor
           *args are passed to the QTableView constructor without modification.
        """
        super().__init__(*args)
        self._model = ProgramModel(self)
        self.setModel(self._model)
        
        # Set the headers:
        
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)
        
# Test code:
# Requires a manager running in localhost.
if __name__ == "__main__":
    from nscldaq.manager_client import Programs
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    import sys
    p = Programs('localhost')
    data = p.status()
    
    app = QApplication(sys.argv)
    mw = QMainWindow()
    table = ProgramView()
    table.model().setFilter(['exp1-testing', 'exp2-testing', 'exp3-testing'])
    table.model().update(data)
    mw.setCentralWidget(table)
    
    mw.show()
    app.exec()
    