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
from PyQt5.QtWidgets import QApplication, QTableView
from PyQt5.QtGui import    QStandardItemModel, QStandardItem
from PyQt5.QtCore import Qt, QTimer
from nscldaq.portmanager.PortManager import PortManager

import socket
import json

SERVICENAME="RING_MONITOR"
UPDATE_MILLISECONDS=5000   # I don't think the server updates faster.

KB = float(1024)
MB = float(KB*KB)
GB = float(MB*KB)
TB = float(GB*KB)

# vol_units
#   Given a number, returns a string containing
#   An appropriate number and units.
# 
#  E.g. given a number like 2048 returns 2 KB
#  Units go up to TB.
#
def vol_units(bytes) :
    if bytes > TB :
        return f"{bytes/TB} TB"
    elif bytes > GB :
        return f"{bytes/GB} GB"
    elif bytes > MB :
        return f"{bytes/MB} MB"
    elif bytes > KB :
        return f"{bytes/KB} KB"
    else :
        return f"{bytes} B"

# Same as vol_units but the resulting string
# gets /s appended to it:
def rate_units(rate) :    
    return vol_units(rate) + "/s"
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
#  If the name already exists, we just append it to the
#  model...sorting is done by the table.
# 
def populateModel(data, model):
    for ring_data in data:
        #  Set up the data:
        line = (
            QStandardItem(ring_data['name']), 
            QStandardItem(vol_units(ring_data['cum_statistics']['bytes'])),
            QStandardItem(vol_units(ring_data['cum_statistics']['bytes_this_run'])),
            QStandardItem(vol_units(ring_data['cum_statistics']['events'])),
            QStandardItem(vol_units(ring_data['cum_statistics']['events_this_run'])),
            QStandardItem(rate_units(ring_data['byte_rate'])),
            QStandardItem(rate_units(ring_data['event_rate']))
        )
        matches = model.findItems(ring_data['name'])  #  Exists?
        if len(matches) > 0 :
            itemIndex = model.indexFromItem(matches[0])
            row = itemIndex.row()
            for col, item in enumerate(line) :
                model.setItem(row, col, item)
        else : 
            # new row:
            model.appendRow(line)

# Update the model.. we take advantage
# Of the fact that host, port, data, model
# are defined at the global level
# That allows this to also be a timer slot.
           
def update():
    print("Update")
    data = getStatistics(host, port)
    populateModel(data, model)
    
if __name__ == '__main__':
    host = "localhost"
    if len(sys.argv) > 1:
        host = sys.argv[1]
    
    #  Get the ring monitor port:
    
    port = getServerPort(host)
    if port is None:
        print('The ring monitor server is not running in ', host)
        sys.exit(-1)
    
    
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
    window.setSortingEnabled(True)
    
    # Set up the model:
    
    setupModel(model)

    
    #  Get the initial data, populate the model, and 
    #  Schedule periodic
    
    update()
    
    timer = QTimer()
    timer.setInterval(UPDATE_MILLISECONDS)
    timer.setSingleShot(False)
    timer.timeout.connect(update)
    timer.start()

    # Optional: Set window properties
    window.setWindowTitle('Minimal PyQt Window')
    window.setGeometry(100, 100, 400, 200) # x, y, width, height

    # 3. Show the window
    window.show()

    # 4. Start the application's event loop
    sys.exit(app.exec())
