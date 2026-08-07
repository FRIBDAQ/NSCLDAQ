import numpy as np

from PyQt5.QtGui import QDoubleValidator
from PyQt5.QtWidgets import QWidget, QLabel, QLineEdit, QVBoxLayout

from extensions import MyGridLayout


class ChanDSPWidget(QWidget):
    """Channel DSP tab widget.

    Generic channel DSP tab widget intended to be subclassed for particular
    families of DSP parameters. This class interacts only with the internal
    DSP settings and its own display. Note that the channels are 0-indexed
    while the DSP grid is 1-indexed (grid row 0 holds the column headers, so
    channel i occupies grid row i + 1). Provides template methods for
    subclasses which are called in the appropriate class methods.

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
        Grid of QWidgets to display DSP parameters.

    Methods
    -------
    configure(mgr, mod)
        Initialize GUI.
    update_dsp(mgr, mod)
        Update DSP from GUI.
    display_dsp(mgr, mod)
        Display current DSP in GUI.
    copy_chan_dsp(idx)
        Copy DSP from channel idx to all channels in GUI.
    """

    def __init__(
        self, param_names=None, param_labels=None, nchannels=16, *args, **kwargs
    ):
        """ChanDSPWidget class constructor.

        Initialize generic channel DSP widget, set parameter validators and
        labels. Channel DSP is displayed on an nchannel x nDSP grid of
        QLineEdit widgets. Note that the actual DSP parameters are 0-indexed
        while the grid is 1-indexed.

        Parameters
        ----------
        param_names : list, default=None
            DSP parameter names for API read/write calls.
        param_labels : list, default=None
            Column labels for the GUI.
        nchannels : int, default=16
            Number of channels per module.
        """
        super().__init__(*args, **kwargs)
        self.param_names = param_names
        self.param_labels = param_labels
        self.nchannels = nchannels
        self.has_extra_params = False

        # Subwidget configuration:

        dsp_grid = QWidget()
        self.param_grid = MyGridLayout(dsp_grid)

        self.param_grid.addWidget(QLabel("Ch."), 0, 0)
        for col, label in enumerate(self.param_labels, 1):
            self.param_grid.addWidget(QLabel(label), 0, col)

        for i in range(self.nchannels):
            self.param_grid.addWidget(QLabel("%i" % i), i + 1, 0)
            for col, _ in enumerate(self.param_labels, 1):
                w = QLineEdit()
                # Default channel parameter validator is double:
                w.setValidator(
                    QDoubleValidator(
                        0, 999999, 3, notation=QDoubleValidator.StandardNotation
                    )
                )
                self.param_grid.addWidget(w, i + 1, col)

        # Define layout and add widgets:

        layout = QVBoxLayout()
        layout.addWidget(dsp_grid)
        layout.addStretch()
        self.setLayout(layout)

    def configure(self, mgr, mod):
        """Initialize and display widget settings from the DSP dataframe.

        We just display the contents of the internal DSP, though some
        widgets inheriting from this class may perform additional actions
        by overriding this function if necessary.

        Parameters
        ----------
        mgr : DSPManager
            DSP manager for calls to XIA API.
        mod : int
            Currently selected module tab index.
        """
        self.display_dsp(mgr, mod)

    def update_dsp(self, mgr, mod):
        """Update dataframe from GUI values.

        Parameters
        ----------
        mgr : DSPManager
            DSP manager for calls to XIA API.
        mod : int
            Currently selected module tab index.
        """
        for i in range(self.nchannels):
            for col, name in enumerate(self.param_names, 1):
                val = float(self.param_grid[i + 1, col].text())
                mgr.set_chan_par(mod, i, name, val)

    def display_dsp(self, mgr, mod):
        """Update GUI with dataframe values.

        Parameters
        ----------
        mgr : DSPManager
            DSP manager for calls to XIA API.
        mod : int
            Currently selected module tab index.
        """
        for i in range(self.nchannels):
            for col, name in enumerate(self.param_names, 1):
                val = np.format_float_positional(
                    mgr.get_chan_par(mod, i, name), precision=3, unique=False
                )
                self.param_grid[i + 1, col].setText(val)

    def copy_chan_dsp(self, idx):
        """Copy channel parameters to all other channels on the module.

        Does not modify the underlying dataframe, new parameters must be
        applied to overwrite the underlying dataframe storage.

        Parameters
        ----------
        idx : int
            Channel index to copy parameters from.
        """
        for i in range(self.nchannels):
            for col, p in enumerate(self.param_names, 1):
                val = self.param_grid[idx + 1, col].text()
                self.param_grid[i + 1, col].setText(val)
