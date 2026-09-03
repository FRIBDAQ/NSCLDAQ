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
    Create a new database, optionally prompting graphically for the
    parameters required to build it.   If the database already
    exists, we don't allow this.
    
    @file lg_create.py
    @brief Create a new logbook database.
    @author Ron Fox
    
'''
import sys
import argparse
from nscldaq.LogBook import logbookadmin
from nscldaq.LogBook import LogBook

def define_arguments() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog = 'lg_create',
        description='Create and optionally make current a logbook',
        epilog='Only -filename is mandatory.   Missing options will be prompted graphically'
    )
    parser.add_argument('-filename', required=True, help='Path to logbook file to create')
    parser.add_argument('-current', type=bool, default=False, help='True if the logbook created should be made current, False if omitted')
    parser.add_argument('-experiment', help='The facility experiment number being logged')
    parser.add_argument('-spokesperson', help='The name of the experiment spokesperson')
    parser.add_argument('-purpose', help='The experiment purpose.')
    return parser


def gui_prompt(filename :str, experiment : str | None, spokesperson : str | None, purpose : str | None) -> tuple[str]:
    #  Pop up a gui prompting dialog for the experiment, spokesperson and purpose
    #  The contents of the dialog will be loaded with any non-None values passed in.
    #  If the dialog is accepted, the results are passed back as a tuple
    # of (experiment, spokesperson, purpose4)
    #
    # Note we make it so that we don't even pull in Qt stuff unless this is called.
    from PyQt6.QtWidgets import (
        QDialog, QDialogButtonBox, QLabel, QLineEdit, QTextEdit, 
        QWidget, QApplication,
        QHBoxLayout, QVBoxLayout
    )
    
    # The prompter class:
    
    class Prompter(QDialog):
        def __init__(self):
            super().__init__(None)
            
            self._layout = QVBoxLayout()
            self.setLayout(self._layout)
        
            # There's a top strip that contains the experiment, spokesperson blanks:
            top = QHBoxLayout()
            
            top.addWidget(QLabel('Experiment: ', self))    
            self._experiment = QLineEdit(self)
            top.addWidget(self._experiment)
            
            top.addWidget(QLabel('Spokesperson: ', self))
            self._spokesperson = QLineEdit(self)
            top.addWidget(self._spokesperson)
            
            self._layout.addLayout(top)
            
            # Label and editor for the purpose:
            
            self._layout.addWidget(QLabel("Experiment Purpose", self))
            self._purpose = QTextEdit(self)
            self._layout.addWidget(self._purpose)
            

            self._buttons = QDialogButtonBox(
                QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel,
                self
            )
            self._layout.addWidget(self._buttons)
            self._buttons.accepted.connect(self.accept)
            self._buttons.rejected.connect(self.reject)
            
        # Provide attributes to get/set the values of the fields:
        
        def experiment(self) -> str:
            return self._experiment.text()
        def setExperiment(self, exp :str) -> None:
            self._experiment.setText(exp)

        def spokesperson(self) -> str:
            return self._spokesperson.text()
        def setSpokesperson(self, person: str) -> None:
            self._spokesperson.setText(person)
        def purpose(self) -> str:
            return self._purpose.toPlainText()
        def setPurpose(self, purpose: str) -> None:
            self._purpose.setPlainText(purpose)
    
    # Now we have a prompter we can use it:
    
    app = QApplication(sys.argv)
    prompt = Prompter()
    prompt.setWindowTitle(f'Paramters for the logbook {filename}')
    if experiment:
        prompt.setExperiment(experiment)
    if spokesperson:
        prompt.setSpokesperson(spokesperson)
    if purpose:
        prompt.setPurpose(purpose)
        
    if prompt.exec() == QDialog.DialogCode.Accepted:
        return (
            prompt.experiment(), prompt.spokesperson(), prompt.purpose()
        )
    else:
        return (None, None, None)   # Reject so no values.
    
def main() -> int:
    parser = define_arguments()
    parsed_args = parser.parse_args()    # Defaults to procecessing sys.argv
    
    # Extract the values of the arguments that were parsed into the namespace
    
    filename     = parsed_args.filename
    current      = parsed_args.current
    experiment   = parsed_args.experiment
    spokesperson = parsed_args.spokesperson
    purpose      = parsed_args.purpose
    
    # If we have all we need, make the logbook:
    
    if not (experiment and spokesperson and purpose):
        (experiment, spokesperson, purpose) = gui_prompt(filename, experiment, spokesperson, purpose)
        
        # Make the logbook if everyting was accepted:
    if experiment and spokesperson and purpose:
        try :
            logbookadmin.createLogBook(filename, experiment, spokesperson, purpose, current)
        except LogBook.error as e:
            print(f'Unable to create logbook: {e}')
            return -1
    return 0    
    
        
    
    
    

if __name__ == "__main__":
    sys.exit(main())
