from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QTabWidget, QWidget, QScrollArea

class ChanDSPLayout(QTabWidget):
    """Layout of channel DSP parameters.

    Create the underlying widgets the users interact with the set and read
    channel parameters. The underlying widget is a QWidget wrapped in a
    QScrollArea. Each of these widgets is added as a tab to a module tab in
    the main channel parameter window. This is the lowest level of the nested
    tabs for channel parameters.
    """
    
    def __init__(self, factory, nchannels, *args, **kwargs):
        """ChanDSPLayout class constructor.
        
        Parameters
        ----------
        factory : WidgetFactory
            Factory object for creating channel DSP widgets.
        nchannels : int
            Number of channels for this module tab.
        """        
        super().__init__(*args, **kwargs)
        
        # Setup tabs for each module from the following list. The factory
        # method is parameterized by the tab name and raises and exception
        # when an unknown create method is called.
        
        tabs = [
            #"AnalogSignal",
            "TriggerFilter",
            #"EnergyFilter",
            #"CFD",
            #"Tau",
            #"Trace",
            #"CSRA",
            #"Baseline",
            #"MultCoincidence",
            #"TimingControl",
            #"Histogram"
        ]

        # Define layout:
        
        for i, tab in enumerate(tabs):
            widget = factory.create(tab, nchannels=nchannels)
            scrollable = QScrollArea()
            scrollable.setWidgetResizable(True)
            scrollable.setWidget(widget)
            scrollable.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)
            scrollable.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
            self.insertTab(i, scrollable, tab)
