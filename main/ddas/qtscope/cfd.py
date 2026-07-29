import numpy as np

from PyQt5.QtGui import QIntValidator

from chan_dsp_widget import ChanDSPWidget


class CFD(ChanDSPWidget):
    """CFD DSP tab.

    Methods
    -------
    configure(mgr, mod)
        Configure the CFD display. Overridden from the base class.
    display_dsp(mgr, mod)
        Display DSP settings from the dataframe. Overridden from base class.
    """

    def __init__(self, *args, nchannels=16, **kwargs):
        """CFD class constructor.

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

        param_names = ["CFDDelay", "CFDScale", "CFDThresh"]

        # Parameter labels on the GUI:

        param_labels = ["Delay [us]", "Scale", "Threshold [arb.]"]

        # Create instance of the parent class with these variables:

        super().__init__(param_names, param_labels, nchannels, *args, **kwargs)

    ##
    # Overridden class methods
    #

    def configure(self, mgr, mod):
        """Overridden template configuration operations.

        Setup integer validator for CFDScale parameter. Disable CFD
        settings for 500 MSPS modules.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API
            read/write operations.
        mod : int
            Module number.
        """
        col = self.param_names.index("CFDScale") + 1
        for row in range(1, self.nchannels + 1):
            self.param_grid[row, col].setValidator(QIntValidator(0, 7))
        if mgr.get_module_msps(mod) == 500:
            self._disable_settings()
        super().configure(mgr, mod)

    def display_dsp(self, mgr, mod):
        """Overridden display_dsp.

        Limits precision of CFDScale display to integers.

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
                if name == "CFDScale":
                    val = np.format_float_positional(
                        mgr.get_chan_par(mod, i, name),
                        precision=1,
                        unique=False,
                        trim="-",
                    )
                else:
                    val = np.format_float_positional(
                        mgr.get_chan_par(mod, i, name), precision=3, unique=False
                    )
                self.param_grid[i + 1, col].setText(val)

    def _disable_settings(self):
        """Disable CFD delay and scale.

        Used to disable CFD settings for 500 MSPS modules which are not
        configurable. See Pixie-16 User's Manual Sec. 3.3.8.2 for details.
        """
        for i in range(self.nchannels):
            self.param_grid[i + 1, 1].setEnabled(False)
            self.param_grid[i + 1, 2].setEnabled(False)


class CFDBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """CFDBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        CFD
            Instance of the DSP class widget.
        """
        return CFD(*args, **kwargs)
