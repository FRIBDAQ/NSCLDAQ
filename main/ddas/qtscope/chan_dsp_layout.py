from PyQt5.QtWidgets import QTabWidget, QWidget

class ChanDSPLayout(QTabWidget):
    """Layout of channel DSP parameters.

    Uses the chan_dsp_factory to configure a set of tabs. Each of these tabbed 
    widgets belong to a module tab; this is the lowest level of the nested 
    tab interface for configuring channel DSP settings.
    """
    
    def __init__(self, factory, module, *args, **kwargs):
        """ChanDSPLayout class constructor.
        
        Parameters
        ----------
        factory : WidgetFactory
            Factory object for creating channel DSP widgets.
        module : int
            Module number from ChanDSPManager. 
        """        
        super().__init__(*args, **kwargs)
        
        # Setup tabs for each module from the following list. The factory
        # method is parameterized by the tab name and raises and exception
        # when an unknown create method is called.
        
        tabs = [
            "AnalogSignal",
            "TriggerFilter",
            "EnergyFilter",
            "CFD",
            "Tau",
            "Trace",
            "CSRA",
            "Baseline",
            "MultCoincidence",
            "TimingControl",
            "Histogram"
        ]

        # Define layout:
        
        for i, tab in enumerate(tabs):
            self.insertTab(i, factory.create(tab), tab)
