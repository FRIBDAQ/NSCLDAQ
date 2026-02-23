from PyQt5.QtWidgets import QPushButton

from chan_dsp_widget import ChanDSPWidget
import colors


class TimingControl(ChanDSPWidget):
    """Timing control DSP tab (ChanDSPWidget).

    Methods
    -------
    configure(mgr, mod)
        Configure the TimingControl widget. Overridden from the base class.
    """

    def __init__(self, *args, nchannels=16, **kwargs):
        """TimingControl class constructor.

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

        param_names = [
            "ExternDelayLen",
            "FtrigoutDelay",
            "FASTTRIGBACKLEN",
            "ChanTrigStretch",
            "ExtTrigStretch",
        ]

        # Parameter labels on the GUI:

        param_labels = [
            "FastTrigDelay [us]",
            "ORDelay [us]",
            "ORWidth [us]",
            "ChanCoincWidth [us]",
            "ExtTrigWidth [us]",
        ]

        # Create instance of the parent class with these variables:

        super().__init__(param_names, param_labels, nchannels, *args, **kwargs)

        # Add the show diagram button. The signal connection is handled in the
        # channel DSP manager (mostly to "easily" prevent many of these from
        # being opened at once without trying to keep track of what all the
        # other subwidgets are doing...)

        self.b_show_diagram = QPushButton("Show timing diagram")
        self.b_show_diagram.setStyleSheet(colors.YELLOW)
        self.b_show_diagram.setFixedSize(160, 23)

        # Define layout:

        layout = self.layout()
        layout.addStretch()
        layout.addWidget(self.b_show_diagram)
        layout.addStretch()
        self.setLayout(layout)

    ##
    # Overridden class methods
    #

    def configure(self, mgr, mod):
        """Overridden template configuration operations.

        Display but disable ChanCoincWidth on this tab.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for
                              XIA API read/write operations.
            mod (int): Module number.
        """

        for i in range(self.nchannels):
            self.param_grid[i + 1, 4].setEnabled(False)
        super().configure(mgr, mod)


class TimingControlBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """TimingControlBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns:
        TimingControl
            Instance of the DSP class widget.
        """
        return TimingControl(*args, **kwargs)
