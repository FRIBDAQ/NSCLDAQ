#!/usr/bin/python3

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2024.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#    http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Genie Jhang
#	   FRIB
#	   Michigan State University
#	   East Lansing, MI 48824-1321

import sys, os
from PyQt5 import QtWidgets, QtCore, uic, QtGui
from PyQt5.QtWidgets import QApplication, QMainWindow
import tkinter, argparse, json

DAQBIN = os.environ["DAQBIN"]

BIT_B1 = 9
BIT_B0 = 8
BIT_CH_ACTIVE = 7
BIT_CH = 2 # 2-6
BIT_T1 = 1
BIT_T0 = 0

LOG_GOOD = '#30C300'
LOG_WARNING = '#C28E00'
LOG_ERROR = '#C30000'

class MDPPProxy:
    def __init__(self, module, server, port):
        self.tcl = tkinter.Tcl()
        self.tcl.eval('set DAQROOT $::env(DAQROOT)')
        self.tcl.eval('lappend auto_path [file join $DAQROOT TclLibs]')
        self.tcl.eval('lappend auto_path [file join $DAQROOT lib]')

        self.tcl.eval('package require mdppproxy')

        self.tcl.eval('MDPPProxy conn -module %s -server %s -port %s' % (module, server, port))

    def getProxy(self):
        return self.tcl

class About(QtWidgets.QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        uic.loadUi(DAQBIN + '/MDPPTriggerControl_about.ui', self);

class Window(QtWidgets.QMainWindow):
    def __init__(self, tcl, data):
        super().__init__()

        self.setWindowFlags(QtCore.Qt.WindowMinimizeButtonHint | QtCore.Qt.WindowCloseButtonHint)

        self.tcl = tcl

        uic.loadUi(DAQBIN + '/MDPPTriggerControl.ui', self)

        self.LB_log = self.findChild(QtWidgets.QLabel, 'LB_log')

        self.RB_TS_WB = self.findChild(QtWidgets.QRadioButton, "RB_TS_WB")
        self.RB_TS_SC = self.findChild(QtWidgets.QRadioButton, "RB_TS_SC")
        self.RB_TS_LEMO = self.findChild(QtWidgets.QRadioButton, "RB_TS_LEMO")

        self.CB_TS_B0 = self.findChild(QtWidgets.QCheckBox, "CB_TS_B0")
        self.CB_TS_B1 = self.findChild(QtWidgets.QCheckBox, "CB_TS_B1")

        self.CB_TS_SC = self.findChild(QtWidgets.QComboBox, "CB_TS_SC")
        self.CB_TS_SC.currentTextChanged.connect(self._syncComboBoxes)

        self.CB_TS_T0 = self.findChild(QtWidgets.QCheckBox, "CB_TS_T0")
        self.CB_TS_T1 = self.findChild(QtWidgets.QCheckBox, "CB_TS_T1")
        self.LE_TS_T0 = self.findChild(QtWidgets.QLineEdit, "LE_TS_T0")
        self.LE_TS_T1 = self.findChild(QtWidgets.QLineEdit, "LE_TS_T1")

        self.RB_TO_WB = self.findChild(QtWidgets.QRadioButton, "RB_TO_WB")
        self.RB_TO_SC = self.findChild(QtWidgets.QRadioButton, "RB_TO_SC")
        self.RB_TO_LEMO = self.findChild(QtWidgets.QRadioButton, "RB_TO_LEMO")

        self.CB_TO_B0 = self.findChild(QtWidgets.QCheckBox, "CB_TO_B0")
        self.CB_TO_B1 = self.findChild(QtWidgets.QCheckBox, "CB_TO_B1")

        self.CB_TO_SC = self.findChild(QtWidgets.QComboBox, "CB_TO_SC")
        self.CB_TO_SC.currentTextChanged.connect(self._syncComboBoxes)

        self.CB_TO_T0 = self.findChild(QtWidgets.QCheckBox, "CB_TO_T0")
        self.CB_TO_T1 = self.findChild(QtWidgets.QCheckBox, "CB_TO_T1")

        PB_apply = self.findChild(QtWidgets.QPushButton, 'PB_apply')
        PB_update = self.findChild(QtWidgets.QPushButton, 'PB_update')
        PB_load = self.findChild(QtWidgets.QPushButton, 'PB_load')
        PB_save = self.findChild(QtWidgets.QPushButton, 'PB_save')

        PB_apply.clicked.connect(self.applyToModule)
        PB_update.clicked.connect(self.updateFromModule)
        PB_load.clicked.connect(self.openLoadfileDialog)
        PB_save.clicked.connect(self.openSavefileDialog)

        if data != None:
            self._updateFromJson(data)

        PB_about = self.findChild(QtWidgets.QPushButton, "PB_about")
        PB_about.clicked.connect(self._openAboutDialog)

        self.show()

    def applyToModule(self):
        confirmation = self._popupConfirmation()
        if confirmation == QtWidgets.QMessageBox.No:
            self._setLog(LOG_WARNING, 'Operation cancelled!')
            return;

        triggerSource = self._encodeTriggerSource()
        triggerOutput = self._encodeTriggerOutput()

        self.tcl.eval('conn Set triggerSource %d' % triggerSource)
        self.tcl.eval('conn Set triggerOutput %d' % triggerOutput)

        receivedTriggerSource = int(self.tcl.eval('conn Get triggerSource'))
        receivedTriggerOutput = int(self.tcl.eval('conn Get triggerOutput'))

        if (triggerSource == receivedTriggerSource) and (triggerOutput == receivedTriggerOutput):
            self._setLog(LOG_GOOD, 'Successfully applied the settings!')
        else:
            msg = 'Error updating device: '
            errParam = []

            if triggerSource != receivedTriggerSource:
                errParam.append('triggerSource')

            if triggerOutput != receivedTriggerOutput:
                errParam.append('triggerOutput')

            msg += errParam[0]
            if len(errParam) == 2:
                msg += ', %s' % errParam[1]

            self._setLog(LOG_ERROR, msg)

    def updateFromModule(self):
        confirmation = self._popupConfirmation()
        if confirmation == QtWidgets.QMessageBox.No:
            self._setLog(LOG_WARNING, 'Operation cancelled!')
            return;

        self._updateTriggerSource(int(self.tcl.eval('conn Get triggerSource')))
        self._updateTriggerOutput(int(self.tcl.eval('conn Get triggerOutput')))

        self._setLog(LOG_GOOD, 'Successfully updated the settings!')

    def openLoadfileDialog(self):
        options = QtWidgets.QFileDialog.Options()

        selectedFile, _ = QtWidgets.QFileDialog.getOpenFileName(self, "Load settings to file", '', "JSON files (*.json)", '', options)

        if selectedFile == '':
            self._setLog(LOG_WARNING, 'No changes made!')

            return
        else:
            with open(selectedFile) as file:
                data = json.load(file)

            self._updateFromJson(data)
            self._setLog(LOG_GOOD, 'Successfully loaded the file! - %s (Not applied yet)' % selectedFile.split('/')[-1])


    def openSavefileDialog(self):
        options = QtWidgets.QFileDialog.Options()


        triggerSource = hex(self._encodeTriggerSource())
        triggerOutput = hex(self._encodeTriggerOutput())

        sc = []
        for i in range(0, 32):
            sc.append(self.CB_TS_SC.itemText(i))

        lemo = []
        lemo.append(self.LE_TS_T0.text())
        lemo.append(self.LE_TS_T1.text())

        dataDict = {"triggerSource":triggerSource,"triggerOutput":triggerOutput,"sc":sc,"lemo":lemo}

        selectedFile, _ = QtWidgets.QFileDialog.getSaveFileName(self, "Save settings to file", '', "JSON files (*.json)", '', options)
        if selectedFile == '':
            self._setLog(LOG_WARNING, 'Nothing saved!')

            return
        else:
            if not selectedFile.lower().endswith('.json'):
                selectedFile += '.json'

            with open(selectedFile, "w") as file:
                json.dump(dataDict, file, indent=2)

            self._setLog(LOG_GOOD, 'Successfully saved to file! - %s' % selectedFile.split('/')[-1])

    def _encodeTriggerSource(self):
        triggerSource = 0

        if self.RB_TS_WB.isChecked():
            if self.CB_TS_B0.isChecked():
                triggerSource |= (0x1 << BIT_B0)
            else:
                triggerSource &= ~(0x1 << BIT_B0)

            if self.CB_TS_B1.isChecked():
                triggerSource |= (0x1 << BIT_B1)
            else:
                triggerSource &= ~(0x1 << BIT_B1)

        elif self.RB_TS_SC.isChecked():
            triggerSource |= (0x1 << BIT_CH_ACTIVE)

            triggerSource |= (self.CB_TS_SC.currentIndex() << BIT_CH)

        elif self.RB_TS_LEMO.isChecked():
            if self.CB_TS_T0.isChecked():
                triggerSource |= (0x1 << BIT_T0)
            else:
                triggerSource &= ~(0x1 << BIT_T0)

            if self.CB_TS_T1.isChecked():
                triggerSource |= (0x1 << BIT_T1)
            else:
                triggerSource &= ~(0x1 << BIT_T1)

        return triggerSource

    def _encodeTriggerOutput(self):
        triggerOutput = 0

        if self.RB_TO_WB.isChecked():
            if self.CB_TO_B0.isChecked():
                triggerOutput |= (0x1 << BIT_B0)
            else:
                triggerOutput &= ~(0x1 << BIT_B0)

            if self.CB_TO_B1.isChecked():
                triggerOutput |= (0x1 << BIT_B1)
            else:
                triggerOutput &= ~(0x1 << BIT_B1)

        elif self.RB_TO_SC.isChecked():
            triggerOutput |= (0x1 << BIT_CH_ACTIVE)

            triggerOutput |= (self.CB_TO_SC.currentIndex() << BIT_CH)

        return triggerOutput

    def _updateTriggerSource(self, triggerSource):
        isTS_B1 = (triggerSource & (0x1 << BIT_B1)) >> BIT_B1
        isTS_B0 = (triggerSource & (0x1 << BIT_B0)) >> BIT_B0
        isTS_SC = (triggerSource & (0x1 << BIT_CH_ACTIVE)) >> BIT_CH_ACTIVE
        TS_SC_IDX = (triggerSource & (0x1f << BIT_CH)) >> BIT_CH
        isTS_T1 = (triggerSource & (0x1 << BIT_T1)) >> BIT_T1
        isTS_T0 = (triggerSource & (0x1 << BIT_T0)) >> BIT_T0
        
        if isTS_B0 or isTS_B1:
            self.RB_TS_WB.setChecked(isTS_B0 | isTS_B1)
            self.CB_TS_B0.setChecked(isTS_B0)
            self.CB_TS_B1.setChecked(isTS_B1)
        elif isTS_SC:
            self.RB_TS_SC.setChecked(isTS_SC)
            self.CB_TS_SC.setCurrentIndex(TS_SC_IDX)
        elif isTS_T0 or isTS_T1:
            self.RB_TS_LEMO.setChecked(isTS_T0 | isTS_T1)
            self.CB_TS_T0.setChecked(isTS_T0)
            self.CB_TS_T1.setChecked(isTS_T1)

    def _updateTriggerOutput(self, triggerOutput):
        isTO_B1 = (triggerOutput & (0x1 << BIT_B1)) >> BIT_B1
        isTO_B0 = (triggerOutput & (0x1 << BIT_B0)) >> BIT_B0
        isTO_SC = (triggerOutput & (0x1 << BIT_CH_ACTIVE)) >> BIT_CH_ACTIVE
        TO_SC_IDX = (triggerOutput & (0x1f << BIT_CH)) >> BIT_CH
        isTO_T1 = (triggerOutput & (0x1 << BIT_T1)) >> BIT_T1
        isTO_T0 = (triggerOutput & (0x1 << BIT_T0)) >> BIT_T0

        if isTO_B0 or isTO_B1:
            self.RB_TO_WB.setChecked(isTO_B0 | isTO_B1)
            self.CB_TO_B0.setChecked(isTO_B0)
            self.CB_TO_B1.setChecked(isTO_B1)
        elif isTO_SC:
            self.RB_TO_SC.setChecked(isTO_SC)
            self.CB_TO_SC.setCurrentIndex(TO_SC_IDX)

    def _syncComboBoxes(self, value):
        self.CB_TS_SC.currentTextChanged.disconnect()
        self.CB_TO_SC.currentTextChanged.disconnect()

        self.CB_TS_SC.setItemText(self.sender().currentIndex(), value)
        self.CB_TO_SC.setItemText(self.sender().currentIndex(), value)

        self.CB_TS_SC.currentTextChanged.connect(self._syncComboBoxes)
        self.CB_TO_SC.currentTextChanged.connect(self._syncComboBoxes)

    def _updateFromJson(self, data):
        self._updateTriggerSource(int(str(data['triggerSource']), 16 if str(data['triggerSource']).startswith('0x') else 10))
        self._updateTriggerOutput(int(str(data['triggerOutput']), 16 if str(data['triggerOutput']).startswith('0x') else 10))

        for i in range(len(data['sc'])):
            self.CB_TS_SC.setItemText(i, data['sc'][i])
            self.CB_TO_SC.setItemText(i, data['sc'][i])

        self.LE_TS_T0.setText(data['lemo'][0])
        self.LE_TS_T1.setText(data['lemo'][1])

    def _setLog(self, level, text):
        self.LB_log.setStyleSheet('color: %s;' % level)
        self.LB_log.setText(text)
        font = self.LB_log.font()
        font.setPointSizeF(20)
        self.LB_log.setFont(font)
        
        rect = self.LB_log.contentsRect()
        font = self.LB_log.font()
        size = font.pointSize()
        metric = QtGui.QFontMetrics(font)
        rect2 = metric.boundingRect(rect, QtCore.Qt.TextWordWrap, text)
        step = -0.01 if rect2.height() > rect.height() else 0.01
        while True:
            font.setPointSizeF(size + step)
            metric = QtGui.QFontMetrics(font)
            rect2 = metric.boundingRect(rect, QtCore.Qt.TextWordWrap, text)
            if size <= 1:
                break
            if step < 0:
                size += step
                if rect2.height() < rect.height():
                    break
            else:
                if rect2.height() > rect.height():
                    break
                size += step
        
        font.setPointSizeF(size)
        self.LB_log.setFont(font)

    def _popupConfirmation(self):
        confirmation = QtWidgets.QMessageBox()
        confirmation.setIcon(QtWidgets.QMessageBox.Warning)
        confirmation.setText('Are you sure the DAQ is not running?\n\nClicking Yes on running will pause DAQ multiple times.')
        confirmation.setStandardButtons(QtWidgets.QMessageBox.No | QtWidgets.QMessageBox.Yes)
        confirmation.setDefaultButton(QtWidgets.QMessageBox.No)

        font = confirmation.font()
        font.setPointSizeF(20)
        confirmation.setFont(font)

        return confirmation.exec()

    def _openAboutDialog(self):
        About(self).exec_()
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='MDPP module trigger control GUI launcher')
    parser.add_argument('-s', '-server', metavar='server', dest='server',
                        default='localhost',
                        help='hostname of VMUSB slow control server (default: localhost)')
    parser.add_argument('-p', '-port',   metavar='port',   dest='port',
                        default='27000',
                        help='port of the slow control server (deafult: 27000)')
    parser.add_argument('-m', '-module', metavar='module', dest='module',
                        default='tcl_mymdpp32',
                        help='MDPP module name created in the slow control server (default: tcl_mymdpp32)')
    parser.add_argument('-f', '-file',   metavar='file', dest='settingData', type=argparse.FileType('r'),
                        help='JSON setting file (optional)');

    args = parser.parse_args()
    if args.settingData != None:
        settingData = json.load(args.settingData)
    else:
        settingData = None

    app = QtWidgets.QApplication(sys.argv)
    tcl = MDPPProxy(args.module, args.server, args.port)
    window = Window(tcl.getProxy(), settingData)
    app.exec_()
