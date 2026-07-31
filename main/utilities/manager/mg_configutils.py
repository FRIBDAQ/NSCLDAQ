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

''' Module with configuration utilities.

'''
from PyQt6.QtWidgets import QDialog, QDialogButtonBox, QWidget, QVBoxLayout
from PyQt6.QtCore    import QObject
class SaveDialog(QDialog):
    '''
        This is a dialog with a button box that has save and cancel buttons.
        with a workarea widget that's passed in at construction time.

    '''
    def __init__(self, work_area: QWidget, parent: QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._workarea = work_area
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel,
            self
        )
        self._layout.addWidget(self._buttons)
    
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
        
        
    def workarea(self) -> QWidget:
        return self._workarea
    
    # Turn off key handling:
    
    def keyPressEvent(self, event) -> None:
        pass
    
