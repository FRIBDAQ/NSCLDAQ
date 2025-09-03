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
# TODO: allow more than one host and make the table view 
#     show rings from all the requested hosts.
#
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QTableView
from PyQt5.QtGui import    QStandardItemModel
from PyQt5.QtCore import Qt
from nscldaq.portmanager.PortManager import PortManager

import socket
import json

SERVICENAME="RING_MONITOR"
UPDATE_MILLISECONDS=5000   # I don't think the server updates faster.

##
# getServerPort
#    Return the port the Ring monitor is on.
#  @param host - the host to connect to.
#  @return port number (16 bit integer)
#  @retval None - can't get a port.

def getServerPort(host) :
    pm = PortManager(host)
    info = pm.find(service=SERVICENAME)
    if len(info) > 0:
        return info[0]['port']
    else:
        return None

#  Get the statistics from the server and return it
# sorted by ring name.

def getStatistics(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    data = sock.recv(1000000)
    sock.shutdown(socket.SHUT_RDWR)
    info = json.loads(data)
    info = sorted(info, key=lambda item : item['name'])
    return info
    
# 
#   Setup the model characteristics...specifically
#   We need the titles.
#
def setupModel(model) :
    model.setHorizontalHeaderLabels((
        "Ring", 
        "Total Volume", 
        "Volume This Run", 
        "Total Events", 
        "Events This Run", 
        "Data Volume Rate", 
        "Event Rate"))
#
#  Popluate the model with data.
#  data comes from getStatistics and
#  The model is assumed to be a standard item model.
#  If the name already exists, we just replace the statistics
#  If the name does not exist, we figure out where to insert it
# 
def populateModel(data, model):
    pass

if __name__ == '__main__':
    host = "localhost"
    if len(sys.argv) > 1:
        host = sys.argv[1]
    print("Getting statistics from host: ", host)
    
    #  Get the ring monitor port:
    
    port = getServerPort(host)
    if port is None:
        print('The ring monitor server is not running in ', host)
    
    print("Ring monitor is running  on", port)
    
    # 1. Create an instance of QApplication
    app = QApplication(sys.argv)
    # 2. Create a QWidget (our window)
    window = QTableView()
    window.setShowGrid(True)
    window.setGridStyle(Qt.SolidLine)
    model = QStandardItemModel(window)
    window.setModel(model)
    window.horizontalHeader().show()
    window.verticalHeader().show()
    
    # Set up the model:
    
    setupModel(model)
    
    # Here we'd set up a timer
    
    #  Get the initial data, populate the model, and 
    #  Schedule updates.
    
    data = getStatistics(host, port)
    print(data)
    populateModel(data, model)
    

    # Optional: Set window properties
    window.setWindowTitle('Minimal PyQt Window')
    window.setGeometry(100, 100, 400, 200) # x, y, width, height

    # 3. Show the window
    window.show()

    # 4. Start the application's event loop
    sys.exit(app.exec())
