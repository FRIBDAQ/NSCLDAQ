from chan_dsp_widget import ChanDSPWidget

class Tau(ChanDSPWidget):
    """Tau DSP tab (ChanDSPWidget)."""
    
    def __init__(self, *args, nchannels=16, **kwargs):
        """Tau class constructor.

        Parameters
        -----------------
        *args : tuple
            Positional arguments passed to parent ChanDSPWidget.
        nchannels : int, default=16
            Channel count from factory create method.
        **kwargs : dict
            Keyword arguments passed to parent ChanDSPWidget.
        """
        
        # XIA API parameter names"
        
        param_names = ["TAU"]
        
        # Parameter labels on the GUI"
        
        param_labels = ["Tau [us]"]

        # Create instance of the parent class with these variables"
        
        super().__init__(param_names, param_labels, nchannels, *args, **kwargs)

class TauBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """TauBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        Tau
            Instance of the DSP class widget.
        """                    
        return Tau(*args, **kwargs)
