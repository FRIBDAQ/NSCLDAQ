from chan_dsp_widget import ChanDSPWidget


class TriggerFilter(ChanDSPWidget):
    """Trigger filter DSP tab (ChanDSPWidget)."""

    def __init__(self, *args, nchannels=16, **kwargs):
        """TriggerFilter class constructor.

        Parameters
        -----------------
        *args : tuple
            Positional arguments passed to parent ChanDSPWidget.
        nchannels : int, default=16
            Channel count from factory create method.
        **kwargs : dict
            Keyword arguments passed to parent ChanDSPWidget.
        """
        # XIA API parameter names:

        param_names = ["TRIGGER_RISETIME", "TRIGGER_FLATTOP", "TRIGGER_THRESHOLD"]

        # Parameter labels for the GUI:

        param_labels = ["TriggerRise [us]", "TriggerGap [us]", "Threshold [arb.]"]

        # Create instance of the parent class with these variables:

        super().__init__(param_names, param_labels, nchannels, *args, **kwargs)


class TriggerFilterBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """TriggerFilterBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        TriggerFilter
            Instance of the DSP class widget.
        """
        return TriggerFilter(*args, **kwargs)
