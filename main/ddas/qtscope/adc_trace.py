try:
    from bitarray.util import ba2int, int2ba
except ImportError:
    from converters import ba2int, int2ba

from PyQt5.QtWidgets import QCheckBox, QWidget, QHBoxLayout

from chan_dsp_widget import ChanDSPWidget
import xia_constants as xia


class Trace(ChanDSPWidget):
    """Trace DSP tab (ChanDSPWidget).

    Methods
    -------
    configure(mgr, mod)
        Configure the Trace class. Overridden from base class.
    update(mgr, mod)
        Update DSP from GUI. Overridden from base class.
    display_dsp(mgr, mod)
        Display DSP from the dataframe. Overridden from base class.
    """

    def __init__(self, *args, nchannels=16, **kwargs):
        """Trace class constructor.

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

        param_names = ["TRACE_LENGTH", "TRACE_DELAY"]

        # Parameter labels on the GUI:

        param_labels = ["TraceLength [us]", "TraceDelay [us]"]

        # Create instance of the parent class with these variables. Module
        # number gets passed via kwargs.

        super().__init__(param_names, param_labels, nchannels, *args, **kwargs)

        # Add trace capture CSRA checkbox:

        self.has_extra_params = True
        self.extra_params = ["CHANNEL_CSRA"]

        self.cb_enabled = QCheckBox("Record traces", self)
        widget = QWidget()
        hbox = QHBoxLayout()
        hbox.addWidget(self.cb_enabled)
        widget.setLayout(hbox)
        widget.setFixedWidth(150)

        layout = self.layout()
        layout.addWidget(widget)
        self.setLayout(layout)

    ##
    # Overriden class methods
    #

    def configure(self, mgr, mod):
        """
        Overridden configure options.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        mod : int
            Module number.

        Raises
        ------
        ValueError:
            If trace enable bits are not consistent for all channels on
            the module.
        """
        enb_list = []
        for i in range(self.nchannels):
            csra = int2ba(int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little")
            enb = csra[xia.CSRA_TRACE_ENABLE]
            enb_list.append(enb)

        try:
            if not all(enb == enb_list[0] for enb in enb_list):
                raise ValueError(
                    f"Inconsistent trace enable CSRA bits read on Mod. {mod}"
                )
        except ValueError as e:
            self.logger.exception(
                f"Inconsistent trace enable CSRA bits Mod. {mod}: {enb_list}"
            )
            print(f"{e}:\n\tCheck your settings file, it may be corrupt.")
        finally:
            super().configure(mgr, mod)

    def update_dsp(self, mgr, mod):
        """Overridden update operations.

        Update DSP storage CSRA from based on the trace enable setting from
        the GUI.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        mod : int
            Module number.
        """
        enb = self.cb_enabled.isChecked()
        for i in range(self.nchannels):
            csra = int2ba(int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little")
            csra[xia.CSRA_TRACE_ENABLE] = enb
            mgr.set_chan_par(mod, i, "CHANNEL_CSRA", float(ba2int(csra)))

        super().update_dsp(mgr, mod)

    def display_dsp(self, mgr, mod):
        """Overridden display operations.

        Display trace enable bit from CSRA. Consistency of trace enable bits
        across channels is checked on configure and individual channels cannot
        be manipulated on the GUI, so we assume here that CSRA trace enable bit
        on channel 0 is shared by all other channels.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        mod : int
            Module number.
        """
        csra = int2ba(int(mgr.get_chan_par(mod, 0, "CHANNEL_CSRA")), 32, "little")
        enb = csra[xia.CSRA_TRACE_ENABLE]
        self.cb_enabled.setChecked(enb)
        super().display_dsp(mgr, mod)


class TraceBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """TraceBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        Trace
            Instance of the DSP class widget.
        """
        return Trace(*args, **kwargs)
