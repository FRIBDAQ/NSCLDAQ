try:
    from bitarray.util import ba2int, int2ba, zeros
except ImportError:
    from converters import ba2int, int2ba, zeros

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QRadioButton,
    QPushButton,
    QButtonGroup,
    QLabel,
    QCheckBox,
)

import colors
from extensions import MyGridLayout


class CSRB(QWidget):
    """Module CSRB grid widget.

    This class interacts only with the internal DSP settings and its own
    display. param_names is a single parameter, MODULE_CSRB, param_labels
    give abbreviated settings information, with detailed settings info given
    in the associated tooltips. See Pixie-16 User Manual Sec. 3.3.7, Table 3-1
    for more information.

    Attributes
    ----------
    param_names : list
        List of DSP parameter names.
    param_labels : dict
        Dictionary of DSP parameter GUI column titles and tooltips.
    disabled : list
        Disabled settings on the CSRB GUI .
    grid : QWidget
        Parent widget for the CSRB GUI display.
    param_grid : MyGridLayout
        Gridded layout for the CSRB GUI.
    nmodules : int
        Number of installed modules in the crate.
    rb_group : QButtonGroup
        Radio button group for common crate configurations.
    rb_dict : dict
        Dictionary of CSRB settings for common crate configurations.
    b_show_csrb : QPushButton
        Button to display the CSRB GUI (grid of CSRB bits).

    Methods:
    --------
    configure(mgr)
        Initialize GUI.
    update_dsp(mgr)
        Update DSP from GUI.
    display_dsp(mgr, set_state=False)
        Display current DSP in GUI.
    set_param_grid(mgr)
        Set CSRB values for known crate configurations.
    """

    def __init__(self, *args, nmodules=None, channel_map=None, **kwargs):
        """CSRB class constructor.

        Initialize CSRB widget and set labels. Module DSP is displayed on an
        nchannel x nDSP grid of QCheckBox widgets. Radio buttons allow for
        quick setup for common crate configurations. Note that the actual DSP
        parameters are 1-indexed while the grid is 0-indexed.

        Parameters
        -----------------
        *args : tuple
            Positional arguments passed to parent ChanDSPWidget.
        nmodules : int, default=None
            Module count from factory create method.
        channel_map : list, default=None
            List of channels per module (unused by this class).
        **kwargs : dict
            Keyword arguments passed to parent ChanDSPWidget.
        """
        super().__init__(*args, **kwargs)

        self.param_names = ["MODULE_CSRB"]
        self.nmodules = nmodules

        ##
        # Display widget
        #

        widget = QWidget()
        rbgroup_layout = QVBoxLayout()
        widget.setLayout(rbgroup_layout)
        self.rbgroup = QButtonGroup(widget)

        # Dictionary for the radio button configuration. Key is the button ID,
        # value is a dictionary containing the name and module CSRB settings
        # (mod0 is CSRB for module 0, modx for all other installed modules) for
        # the configuration.

        self.rb_dict = {
            0: {"name": "Single crate", "mod0": 65, "modx": 0},
            1: {"name": "Multicrate (director)", "mod0": 2129, "modx": 2048},
            2: {"name": "Multicrate (chassis)", "mod0": 2113, "modx": 2048},
            3: {"name": "Custom", "mod0": None, "modx": None},
        }

        # Add buttons defined in the dictionary to the button group:

        for idx, info in self.rb_dict.items():
            rb = QRadioButton(info["name"])
            self.rbgroup.addButton(rb, idx)
            rbgroup_layout.addWidget(rb)

        # Display button:

        self.b_show_csrb = QPushButton("Display CSRB")
        self.b_show_csrb.setStyleSheet(colors.YELLOW)

        # Define layout:

        layout = QVBoxLayout()
        layout.addWidget(widget)
        layout.addWidget(self.b_show_csrb)
        self.setLayout(layout)

        ##
        # CSRB settings grid
        #

        # Parameter label dictionary. Key is the CSRB bit number (and column
        # location on the x-axis, offset by one for the module label), value is
        # dictionary defining the column label and associated tooltip.

        self.param_labels = {
            0: {
                "label": "PlUp",
                "tooltip": "Enable pullups (enable only for crate master module)",
            },
            1: {"label": "Rsv", "tooltip": "Reserved"},
            2: {"label": "Rsv", "tooltip": "Reserved"},
            3: {"label": "Rsv", "tooltip": "Reserved"},
            4: {
                "label": "SDir",
                "tooltip": "Set as system director module (=1) (enable for only one module in multi-crate setup)",
            },
            5: {"label": "Rsv", "tooltip": "Reserved"},
            6: {
                "label": "CMas",
                "tooltip": "Set as crate master module (enable for only crate master module)",
            },
            7: {
                "label": "FTrS",
                "tooltip": "External fast trigger from validation trigger (=1) or fast trigger (=0)",
            },
            8: {
                "label": "EVTr",
                "tooltip": "External validation trigger from fast trigger (=1) or validation trigger (=0)",
            },
            9: {"label": "Rsv", "tooltip": "Reserved"},
            10: {
                "label": "EInS",
                "tooltip": "Enable (=1) or disable (=0) external inhibit",
            },
            11: {
                "label": "MCM",
                "tooltip": "Multi- (=1) or single (=0) crate mode. Enable for each module in a multi-crate system",
            },
            12: {
                "label": "Sort",
                "tooltip": "Enable (=1) or disable (=0) event sorting prior to storage in FIFO",
            },
            13: {
                "label": "BPTr",
                "tooltip": "Enable (=1) or disable (=0) sending triggers to local backplane",
            },
        }

        # List of disabled bits by param_label:

        self.disabled = ["Rsv"]

        # Subwidget configuration. Define the grid and add widgets:

        self.grid = QWidget()
        self.grid.setWindowTitle("CSRB settings")
        self.param_grid = MyGridLayout(self.grid)

        for bit, pdict in self.param_labels.items():
            w = QLabel(pdict["label"])
            w.setFixedHeight(20)
            w.setToolTip(pdict["tooltip"])
            self.param_grid.addWidget(w, 0, bit + 1, Qt.AlignCenter)

        ##
        # Signal connections
        #

        self.b_show_csrb.clicked.connect(self._show_csrb)

    def configure(self, mgr):
        """Initialize and display widget settings from the DSP dataframe.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        """
        for i in range(self.nmodules):
            self.param_grid.addWidget(QLabel("Mod. %i" % i), i + 1, 0)
            for bit in self.param_labels:
                self.param_grid.addWidget(QCheckBox(), i + 1, bit + 1, Qt.AlignCenter)
        self.display_dsp(mgr, set_state=True)

    def update_dsp(self, mgr):
        """Update dataframe from GUI values.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        """
        for i in range(self.nmodules):
            csrb = zeros(32, "little")
            for bit in self.param_labels:
                csrb[bit] = self.param_grid[i + 1, bit + 1].isChecked()
            mgr.set_mod_par(i, self.param_names[0], ba2int(csrb))

    def display_dsp(self, mgr, set_state=False):
        """Update GUI with dataframe values

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        set_state : bool, default=False
            Set the CSRB button state based on the current confiugration
            settings.
        """
        if set_state:
            self.rbgroup.button(self._get_crate_config(mgr)).setChecked(True)

        for i in range(self.nmodules):
            val = mgr.get_mod_par(i, self.param_names[0])
            csrb = int2ba(int(val), 32, "little")
            for bit, _ in zip(self.param_labels, csrb):
                self.param_grid[i + 1, bit + 1].setChecked(csrb[bit])

        self._disable_settings()

    def set_param_grid(self, mgr):
        """Set the CSRB parameter grid based on the crate configuration.

        If the crate configuration is one of the known settings, configure
        according to the rb_dict settings for module 0 and other installed
        modules. For a custom config show whatever is currently in the
        dataframe.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        """
        config = self.rbgroup.checkedId()
        if self.rb_dict[config]["mod0"]:
            mod0 = self.rb_dict[config]["mod0"]
            csrb = int2ba(self.rb_dict[config]["mod0"], 32, "little")
            for bit, _ in zip(self.param_labels, csrb):
                self.param_grid[1, bit + 1].setChecked(csrb[bit])
            for i in range(1, self.nmodules):
                csrb = int2ba(self.rb_dict[config]["modx"], 32, "little")
                for bit, _ in zip(self.param_labels, csrb):
                    self.param_grid[i + 1, bit + 1].setChecked(csrb[bit])
            self._disable_settings()
        else:
            self.display_dsp(mgr)

    ##
    # Private methods
    #

    def _show_csrb(self):
        """Display the CSRB GUI."""
        self.grid.show()

    def _get_crate_config(self, mgr):
        """Get the crate configuration from the internal DSP settings.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.

        Returns
        -------
        config: int
            Value corresponding to the crate configuration stored in the
            CSRB settings.
        """
        config = -1
        custom = -1

        # Assume the configuration is custom until proven otherwise:
        for idx, info in self.rb_dict.items():
            if info["name"] == "Custom":
                config = idx
                custom = idx

        for idx, info in self.rb_dict.items():
            # Check first module CSRB value:
            mod0 = mgr.get_mod_par(0, self.param_names[0])
            if mod0 == info["mod0"]:
                config = idx
                # Check all other modules:
                for i in range(1, self.nmodules):
                    modx = mgr.get_mod_par(i, self.param_names[0])
                    if modx != info["modx"]:
                        config = custom
                        break

        return config

    def _disable_settings(self):
        """Mask CSRB settings in non-custom crate configurations."""
        # Mask everything if the crate configuration is not custom to prevent
        # users from incorrectly configuring CSRB settings for common setups:
        if self.rbgroup.checkedButton().text() == "Custom":
            for i in range(self.nmodules):
                for bit, pdict in self.param_labels.items():
                    if pdict["label"] in self.disabled:
                        self.param_grid[i + 1, bit + 1].setEnabled(False)
                    else:
                        self.param_grid[i + 1, bit + 1].setEnabled(True)
        else:
            for i in range(self.nmodules):
                for bit in self.param_labels:
                    self.param_grid[i + 1, bit + 1].setEnabled(False)


class CSRBBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """CSRBBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        CSRB
            Instance of the DSP class widget.
        """
        return CSRB(*args, **kwargs)
