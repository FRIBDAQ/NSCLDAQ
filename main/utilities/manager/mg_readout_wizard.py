#!/usr/bin/env python3
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

'''

This file provides a pythonized version of the Readout wizard.
It simplifies the creation of Readout programs and the helpers they
have to run.

@file mg_readout_wizard.py
@purpose simplify definition of readout programs and helpers
@author Ron Fox
'''


from PyQt6.QtWidgets import QWizard, QWizardPage, QTextEdit, QVBoxLayout
from PyQt6.QtCore    import QObject, pyqtSignal

class IntroPage(QWizardPage):
    '''
        Just provides introductory text.
        This will be registered with page id 1 and always
        branches to page id 2.
    '''
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
    def initializePage(self) -> None:
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        self._intro  = QTextEdit(self)
        self._intro.setReadOnly(True)
        self._layout.addWidget(self._intro)
        
        # Fill the intro text:
        
        self._intro.setHtml('''
<p>
This wizard helps you configure readout programs and their
helpers.  The helper programs do things like ensure the readout
program knows the run number and title prior to running, as well
as informing the readout programs of state changes in the experiment.
</p>
<p>
This wizard applies to the FRIB/NSCLDAQ managed environment. 
Initially, you'll be prompted for values that are independent
of the type of readout and the readout type.  We support 
generating the configuration for the following Readout program types:
</p>
<ul>
    <li>XIA/DDAS Readout system</li>
    <li>VMUSB VME with Wiener/Jtec USB controller</li>
    <li>CCUSB CAMAC with Wiener/Jtec USB controller</li>
    <li>MVLC VME via the Mesytec MVLC  controller 
         (requires mesytec-mvlc installation with FRIB extensions)</li>
    <li>Custom readout (arbitrary Readout program).</li>
</ul>
<p>
Click the next button when you are ready to continue.
</p>
''')
        self.setTitle("Introduction:")
    def nextid(self) -> int:
        ''' stub for now:'''
        return -1
    
    def  pageId(self) -> int:
        '''' All our wizard pages will know their own page id and next id.'''   
        return 1
        
class ReadoutWizard(QWizard):
    '''
        Readout configuration wizard.  Note that this wizard
        is branching:
        Page ids starting at 100 are the XIA/DDAS configuration.
        Page ids starting at 200 are the VMUSB/CCUSB configuration
        Page ids starting at 300 are the MVLC configuration.
        Page ids starting at 400 are custom readouts.
        
        Page ids below that are common initial pages
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Register the pages and their ids:
        
        # Common pages:
        
        self._intro = IntroPage(self)
        self.setPage(self._intro.pageId(), self._intro)
        self.setStartId(self._intro.pageId())
        
        

##  Test code for now:

if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication
    import sys

    def done():
        print("accepted:")
        
    app = QApplication(sys.argv)
    wiz = ReadoutWizard()
    wiz.accepted.connect(done)
    wiz.show()
    
    sys.exit(app.exec())
        
        