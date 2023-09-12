from chan_dsp_widget import ChanDSPWidget

class Baseline(ChanDSPWidget):
    """Baseline DSP tab (ChanDSPWidget)."""
    
    def __init__(self, *args, **kwargs):
        """Baseline class constructor. """        
        # XIA API parameter names:
        
        param_names = [
            "BLCUT",
            "BASELINE_PERCENT"
        ]
        
        # Parameter labels for the GUI:
        
        param_labels = [
            "BaselineCut",
            "Baseline %"
        ]
        
        # Create instance of the parent class with these variables:
        
        super().__init__(param_names, param_labels, *args, **kwargs)

class BaselineBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """BaselineBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        Baseline 
            Instance of the DSP class widget.
        """            
        return Baseline(*args, **kwargs)
