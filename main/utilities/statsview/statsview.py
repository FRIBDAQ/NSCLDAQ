#! /usr/bin/env python3

#
#  This is a script to view statistics from
#  the ring monitor program.
#  it periodically requests statistics from the RING_MONITOR
#  service and displays it in a tableview.
# Usage:
#   $DAQBIN/statsview [host]
#
# If host is supplied, it indcicates which host's ring monitor
# will be contacted.  If not supplied, locahost will be used.
#
import sys
from PyQt5.QtWidgets import QApplication, QWidget

if __name__ == '__main__':
    # 1. Create an instance of QApplication
    app = QApplication(sys.argv)

    # 2. Create a QWidget (our window)
    window = QWidget()

    # Optional: Set window properties
    window.setWindowTitle('Minimal PyQt Window')
    window.setGeometry(100, 100, 400, 200) # x, y, width, height

    # 3. Show the window
    window.show()

    # 4. Start the application's event loop
    sys.exit(app.exec_())
    