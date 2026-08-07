try:
    from bitarray.util import ba2int, int2ba
except ImportError:
    from converters import ba2int, int2ba
import numpy as np

from PyQt5.QtGui import QDoubleValidator
from PyQt5.QtWidgets import (
    QWidget,
    QLabel,
    QLineEdit,
    QVBoxLayout,
    QComboBox,
    QPushButton,
)

import colors
from extensions import MyGridLayout
import xia_constants as xia


class AnalogSignal(QWidget):
    """Analog signal tab widget.

    This class interacts only with the internal DSP settings and its own
    display. Configures channel polarity and gain settings via the CSRA bits.
    Dimensions of param_names and param_labels are different: param_names
    accesses CHANNEL_CSRA while param_labels separate the bit values associated
    with the gain and polarity into separate QComboBoxes.

    Attributes
    ----------
    param_names : list
        List of DSP parameter names.
    param_labels : list
        List of DSP parameter GUI column titles.
    nchannels : int
        Number of channels per module.
    has_extra_params : bool
        Extra parameter flag.
    param_grid : MyGridLayout
        Grid of QWidgets to display DSP parameters
    b_adjust_offsets : QPushButton
        Button for signal connection to adjust DC offsets in this module.

    Methods
    -------
    configure(mgr, mod):
        Initialize GUI.
    update_dsp(mgr, mod):
        Update DSP from GUI.
    display_dsp(mgr, mod):
        Display current DSP in GUI.
    copy_chan_dsp(mgr, mod):
        Copy DSP from channel idx in GUI.
    """

    def __init__(self, *args, nchannels=16, **kwargs):
        """AnalogSignal constructor.

        Initialize analog signal DSP widget, set parameter validators and
        labels. Channel DSP is displayed on an nchannel x nDSP grid of
        QLineEdit widgets. Note that the actual DSP parameters are 1-indexed
        while the DSP grid is 0-indexed.

        Parameters
        -----------------
        *args : tuple
            Positional arguments passed to parent QWidget.
        nchannels : int, default=16
            Channel count from factory create method.
        **kwargs : dict
            Keyword arguments passed to parent QWidget.
        """
        super().__init__(*args, **kwargs)

        # XIA API parameter names:

        self.param_names = ["VOFFSET", "CHANNEL_CSRA"]
        self.param_labels = ["Offset [V]", "Gain", "Polarity"]
        self.nchannels = nchannels
        self.has_extra_params = False

        # Subwidget configuration:

        dsp_grid = QWidget()
        self.param_grid = MyGridLayout(dsp_grid)

        self.param_grid.addWidget(QLabel("Ch."), 0, 0)
        for col, label in enumerate(self.param_labels, 1):
            self.param_grid.addWidget(QLabel(label), 0, col)

        for i in range(self.nchannels):
            # Remaining grid widgets "by hand" in this case:
            p_offset = QLineEdit()
            p_offset.setValidator(
                QDoubleValidator(
                    -999999, 999999, 3, notation=QDoubleValidator.StandardNotation
                )
            )

            p_gain = QComboBox()
            p_gain.insertItem(0, "1.0")
            p_gain.insertItem(1, "4.0")

            p_pol = QComboBox()
            p_pol.insertItem(0, "-")
            p_pol.insertItem(1, "+")

            # Add to the grid. Note row = i+1:
            self.param_grid.addWidget(QLabel("%i" % i), i + 1, 0)
            self.param_grid.addWidget(p_offset, i + 1, 1)
            self.param_grid.addWidget(p_gain, i + 1, 2)
            self.param_grid.addWidget(p_pol, i + 1, 3)

        # Add adjust offsets button. The signal connection for this button
        # is handled in the channel DSP manager.

        self.b_adjust_offsets = QPushButton("Adjust offsets")
        self.b_adjust_offsets.setStyleSheet(colors.CYAN)
        self.b_adjust_offsets.setFixedSize(96, 23)

        # Define layout:

        layout = QVBoxLayout()
        layout.addWidget(dsp_grid)
        layout.addStretch()
        layout.addWidget(self.b_adjust_offsets)
        layout.addStretch()
        self.setLayout(layout)

    def configure(self, mgr, mod):
        """
        Initialize and display widget settings from the DSP dataframe.

        Call template functions for the widget class and display the DSP
        settings from the argument dataframe.

        Parameters
        ----------
            mgr (DSPManager): Manager for internal DSP and interface for
                              XIA API read/write operations.
            mod (int): Module number.
        """
        self.display_dsp(mgr, mod)

    def update_dsp(self, mgr, mod):
        """Update dataframe from GUI values.

        Parameters
        ----------
        mgr : DSP-Manager
            Manager for internal DSP and interface for XIA API
            read/write operations.
        mod : int
            Module number.
        """
        for i in range(self.nchannels):
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )  # 32 bits, little-endian.
            csra[xia.CSRA_GAIN] = self.param_grid[i + 1, 2].currentIndex()
            csra[xia.CSRA_POLARITY] = self.param_grid[i + 1, 3].currentIndex()

            mgr.set_chan_par(mod, i, "VOFFSET", float(self.param_grid[i + 1, 1].text()))
            mgr.set_chan_par(mod, i, "CHANNEL_CSRA", float(ba2int(csra)))

    def display_dsp(self, mgr, mod):
        """Update GUI with dataframe values.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API
            read/write operations.
        mod : int
            Module number.
        """
        for i in range(self.nchannels):
            offset = np.format_float_positional(
                mgr.get_chan_par(mod, i, "VOFFSET"), precision=3, unique=False
            )
            self.param_grid[i + 1, 1].setText(offset)

            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )  # 32-bit little endian
            self.param_grid[i + 1, 2].setCurrentIndex(csra[xia.CSRA_GAIN])
            self.param_grid[i + 1, 3].setCurrentIndex(csra[xia.CSRA_POLARITY])

    def copy_chan_dsp(self, idx):
        """Copy channel DSP parameters.

        Copy channel parameters from a single channel (row) to all other
        channels on the module. Do not modify the underlying dataframe.

        Parameters
        ----------
        idx : int
            Channel index to copy parameters from.
        """
        offset = self.param_grid[idx + 1, 1].text()
        gain = self.param_grid[idx + 1, 2].currentIndex()
        pol = self.param_grid[idx + 1, 3].currentIndex()

        for i in range(self.nchannels):
            self.param_grid[i + 1, 1].setText(offset)
            self.param_grid[i + 1, 2].setCurrentIndex(gain)
            self.param_grid[i + 1, 3].setCurrentIndex(pol)


class AnalogSignalBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """AnalogSignalBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        AnalogSignal
            Instance of the DSP class widget.
        """
        return AnalogSignal(*args, **kwargs)
