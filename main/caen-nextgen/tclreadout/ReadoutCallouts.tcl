#/*
#*-------------------------------------------------------------
# 
# CAEN SpA 
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#------------------------------------------------------------
#
#**************************************************************************
#* @note TERMS OF USE:
#* This program is free software; you can redistribute it and/or modify it under
#* the terms of the GNU General Public License as published by the Free Software
#* Foundation. This program is distributed in the hope that it will be useful, 
#* but WITHOUT ANY WARRANTY; without even the implied warranty of 
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
#* software, documentation and results solely at his own risk.
#*
#* @file      ReadoutCallouts.tcl
#* @brief     Sample customization script to setup the event builder use with tclreadout demo
#* @author   Ron Fox
#*
#*/
package require evbcallouts

::EVBC::useEventBuilder

proc OnStart {} {
    EVBC::initialize -restart true -glombuild true -glomdt 10 -destring evbout
}

EVBC::registerRingSource tcp://localhost/ron "" 1 CAEN_ADC1 1 1 10 0
