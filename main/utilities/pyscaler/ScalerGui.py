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
This module provides the top level Python Scaler display GUI.
It consists of a RunInfo widget in which, as the name implies,
is displayed information about the current run state and run, if active.

The bottom part is a QTabWidget in which each tab contains
a ScalerPageView and associated model.

@todo  We need to add an optional pane at the bottom of the widget in wihch
stripchart graphs are displayed if requested.

'''



from PyQt5.QtWidgets import QTabWidget, QWidget, QVBoxLayout

from ScalerPage import ScalerPageView, ScalerPageModel
from RunInfo import RunInfo

from operator import itemgetter
class ScalerDisplay(QWidget):
    '''
        This class creates the scaler display described in the
        class comment header.
        Note that we integrate the models needed to run the
        pages.
        
        All attributes of the RunInfo widget are exposed.
        The title attribute for the various tabs are exposed
        as are the models associated with each tab.
    '''
    def __init__(self, *args):
        super().__init__(*args)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._info = RunInfo(self)
        self._tabs = QTabWidget(self)
        
        self._layout.addWidget(self._info)
        self._layout.addWidget(self._tabs)
        
        self._models = {}     # indexed by page name has page model and definition.
        
    def addPage(self, definition):
        '''
            Adds a page to the tabbed widget.  The page added
            will be a ScalerPageView. A model will also
            be created for that view.  
            The models will be saved in a dict indexed by page name.
            
            @param definition This is a page definition from the
                 configfile module  To review, each page is a dict with the keys.
                 - name - name of the page - will be the tab text.
                 - title - Title of the page - will be the title attribute of the ScalerPageView
                 - lines - The lines to display.  This is an interable containing dicts with the keys:
                   number - the line number.  
                   type   - The line type (single, pair or ratio)
                   scalers  - array of scaler names.
            @note - we assume the definition might be out of line order.
            @note - any skipped lines are put in the associated model as type:empty
            
        '''
        page = ScalerPageView(self._tabs)
        self._tabs.addTab(page, definition['name'])
        page.setTitle(definition['title'])
        
        model = ScalerPageModel()
        page.setModel(model)
        
        # Fill in the model sort by line number.
        
        line_num=1
        lines = sorted(definition['lines'], key=itemgetter('number'))
        for line in lines:
            while line['number'] > line_num:
                model.addLine({'type': 'empty'})         # Fill in empty lines:
                line_num += 1
            model.addLine(line)
            line_num +=1
        self._models[definition['name']] = (model, definition)   # Keep track of the models and defs.
     
    def lineModel(self, name):
        return self._models[name][0]   
    def lineDefinition(self, name):
        return self._models[name][1]