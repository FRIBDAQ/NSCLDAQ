import numpy as np

from PyQt5.QtGui import QIntValidator

from chan_dsp_widget import ChanDSPWidget

class Histogram(ChanDSPWidget):
    """Histograming DSP tab.
    
    Methods
    -------
    configure(mgr, mod)
        Configure the histogramming display. Overridden from the base class.
    display_dsp(mgr, mod) 
        Display DSP settings from the dataframe. Overridden from base class.
    """

    def __init__(self, *args, **kwargs):
        """Histogram class constructor."""
    
        # XIA API parameter names:
        
        param_names = [
            "EMIN",
            "BINFACTOR"
        ]
        
        # Parameter labels on the GUI:
        
        param_labels = [
            "EMin",
            "BinFactor"
        ]
        
        # Create instance of the parent class with these variables:
        
        super().__init__(param_names, param_labels, *args, **kwargs)

    ##
    # Overridden class methods
    #
    
    def configure(self, mgr, mod):
        """Overridden template configuration operations.

        Setup integer validators for EMin and BinFactor.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API 
            read/write operations.
        mod : int 
            Module number.
        """        
        col1 = self.param_names.index("EMIN") + 1
        col2 = self.param_names.index("BINFACTOR") + 1        
        for row in range(1, self.nchannels+1):
            w1 = self.param_grid.itemAtPosition(row, col1).widget()
            w1.setValidator(QIntValidator(0, 65535))
            w2 = self.param_grid.itemAtPosition(row, col2).widget()
            w2.setValidator(QIntValidator(1, 16))
        super().configure(mgr, mod)

    def display_dsp(self, mgr, mod):
        """Overridden display_dsp.
        
        Limits precision of EMin and BinFactor to integer.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API 
            read/write operations.
        mod : int 
            Module number.
        """        
        for i in range(self.nchannels):
            for col, name in enumerate(self.param_names, 1):
                val = np.format_float_positional(
                    mgr.get_chan_par(mod, i, name),
                    precision=1,
                    unique=False, trim="-"
                )
                self.param_grid.itemAtPosition(i+1, col).widget().setText(val)

class HistogramBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """HistogramBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        Histogram 
            Instance of the DSP class widget.
        """
        return Histogram(*args, **kwargs)
