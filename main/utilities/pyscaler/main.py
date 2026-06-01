'''
  This contains the main line code for the pyScaler python scaler display.
  
  @file main.py
  @brief main pyScaler program.
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
import configfile
import ScalerGui
from   PyQt5.QtWidgets import QApplication, QMainWindow


from sys import argv, exit, stderr

## 
# usage:
# Output program usage.
def usage():
    print("Usage:" , file=stderr)
    print(f'   {argv[0]}  configfile', file=stderr)
    print( 'Where', file=stderr)
    print('    configfile - is a pyScaler configuration file.', file=stderr)
    
    
##
#  configure_display
#
#  Configure the tabs etc. in the GUI:
# Parameters:
#    display - the ScalerDisplay widget to configure.
#    configuration - the processed configuration.
#
def configure_display(display, configuration):
    #
    # Add the notebook pages:
    
    pages = configuration.pages()
    for page in pages:
        display.addPage(page)


##
#  entry point.
def main():
    # There must be exactly one parameter, the configuration file
    # and it must exist:
    
    if len (argv) != 2:
        usage()
        exit(-1)
    
    # 
    # Process the configuration file:
    #
    with open(argv[1], 'r') as f:
        config_text = f.read()
    configuration = configfile.Configuration(config_text)
    warnings = configuration.check()
    if len(warnings) > 0:
        print('The configuration file had warnings:', file=stderr)
        for warning in warnings:
            print(warning, file=stderr)
        print(
            "For now ignoring but you might want to fix them for next time", 
            file=stderr
        )
    
    #  Set up the UI.
    
    application = QApplication(argv)
    main_window = QMainWindow()
    
    
    display     = ScalerGui.ScalerDisplay(main_window)
    configure_display(display, configuration)
    main_window.setCentralWidget(display)
    
    
    
    # Set up the data structures needed for the updates.
    
    # Set up the data sources.
    
    
    # Run the QtApp.
    main_window.show()
    # Set the size to 850 x 700 which works on WSL/Windows.
    # Though Y size needs some thought.
    main_window.resize(850, 700)
    
    exit(application.exec())
    
    

if __name__ == '__main__':
    # Note that doing things this way supports unit testing.
    
    main()
