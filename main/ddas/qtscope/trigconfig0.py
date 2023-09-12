import bitarray as ba
ver = [int(i) for i in ba.__version__.split(".")]
if bool(ver[0] >= 1 or (ver[0] == 1 and ver[1] >= 6)):
    from bitarray.util import ba2int, int2ba, zeros
else:
    from converters import ba2int, int2ba, zeros

from PyQt5.QtCore import Qt, QBitArray
from PyQt5.QtWidgets import QWidget, QPushButton, QVBoxLayout, QComboBox, QGridLayout, QLabel, QButtonGroup, QRadioButton

import colors
import xia_constants as xia

class TrigConfig0(QWidget):
    """Module TrigConfig0 widget.

    This class interacts only with the internal DSP settings and its own 
    display. param_names is a single parameter, TrigConfig0, param_labels 
    give abbreviated settings information with detailed settings info given 
    in the associated tooltips. See Pixie-16 User Manual Sec. 3.3.11.2, 
    Table 3-9 for more information.
    
    Attributes
    ----------
    param_names : list 
        List of DSP parameter names.
    param_labels : dict 
        Dictionary of DSP parameter GUI column titles and tooltips.
    nmodules : int 
        Number of installed modules in the crate.
    nchannels : int 
        Number of channels per moduele.
    grid : QWidget 
        Grid widget shown on the GUI.
    param_grid : QGridLayout 
        Gridded layout for the CSRB GUI.
    rb_group : QButtonGroup 
        Radio button group for common crate configurations.
    rb_dict : dict 
        Dictionary of TrigConfig0 settings for common trigger settings.
    b_show_config : QPushButton 
        Button to display the TrigConfig0 GUI (grid of TrigConfig0 settings 
        options).

    Methods
    -------
    configure(mgr)
        Initialize GUI.
    update_dsp(mgr)
        Update DSP from GUI.
    display_dsp(mgr, set_state=False)
        Display current DSP in GUI.
    set_param_grid(mgr)
        Setup TrigConfig0 for known trigger settings.
    """
    
    def __init__(self, nmodules=None, nchannels=16, *args, **kwargs):
        """
        TrigConfig0 class constructor.

        Arguments:
            nmodules (int): Number of modules installed in the system.
            nchannels (int): Number of channels per module (optional, 
                             default=16).
        """        
        super().__init__(*args, **kwargs)
        
        self.param_names = ["TrigConfig0"]
        self.nmodules = nmodules
        self.nchannels = nchannels
        
        ##
        # Display widget
        #
        
        widget = QWidget()
        rbgroup_layout = QVBoxLayout()
        widget.setLayout(rbgroup_layout)
        self.rbgroup = QButtonGroup(widget)

        # TrigConfig0 button options, default is self-triggering or custom.
        # Self-triggering means TrigConfig0 = 0. Can be extended to include
        # more options.
        
        self.rb_dict = {
            0: {"name": "Default (self-trigger)"},
            1: {"name": "Custom"}
        }
        
        for idx, info in self.rb_dict.items():
            rb = QRadioButton(info["name"])
            self.rbgroup.addButton(rb, idx) # i.e. button, id.
            rbgroup_layout.addWidget(rb)
        
        self.b_show_config = QPushButton("Display TrigConfig")
        self.b_show_config.setStyleSheet(colors.YELLOW)

        # Add subwidgets to the TrigConfig0 box:
        
        layout = QVBoxLayout()
        layout.addWidget(widget)
        layout.addWidget(self.b_show_config)
        self.setLayout(layout)
        
        ##
        # TrigConfig0 settings grid
        #
        
        # Parameter label dictionary. Key is the TrigConfig0 index, value is
        # dictionary containing information on the particular settings for each
        # configuration option. Label defines the column heading, options list
        # (possible trigger sources), bit_low and bit_high are the TrigConfig0
        # mask to set this trigger option, and a tooltip. Option order matters:
        # index in the options list is the value to set the masked TrigConfig0
        # bits; "chan" means a list of channels [0, nchannels-1].
        
        self.param_labels = {
            0: {"label": "IFTr", "bit_low": xia.TC0_INT_FAST_TRIG_OFFSET, "bit_high": xia.TC0_INT_FAST_TRIG_END, "tooltip": "Internal fast trigger signal from channel fast trigger"},
            1: {"label": "EFTr", "options": ["Ext_FastTrig_Sel", "Int_FastTrig_Sel", "FTIN_Or", "LVDS_FastTrig_FP", "ChanTrig_Sel"], "bit_low": xia.TC0_EXT_FAST_TRIG_OFFSET, "bit_high": xia.TC0_EXT_FAST_TRIG_END, "tooltip": "External fast trigger source"},
            2: {"label": "IVTr", "bit_low": xia.TC0_INT_VAL_TRIG_OFFSET, "bit_high": xia.TC0_INT_VAL_TRIG_END, "tooltip": "Internal validation trigger signal from channel fast trigger"},
            3: {"label": "TsGr", "options": ["TTL0", "TTL1"], "bit_low": xia.TC0_TEST_GROUP_OFFSET, "bit_high": xia.TC0_TEST_GROUP_END, "tooltip": "Group test signal sent to digital output for test/debug"},
            4: {"label": "TsEn", "options": ["Disabled", "Enabled"], "bit_low": xia.TC0_ENB_TEST_OFFSET, "bit_high": xia.TC0_ENB_TEST_END, "tooltip": "Send test signals to digital output for test/debug"},
            5: {"label": "TsCh", "bit_low": xia.TC0_CH_TEST_OFFSET, "bit_high": xia.TC0_CH_TEST_END,"tooltip": "Channel test signal to digital output for test/debug"},
            6: {"label": "TsDO", "options": ["ET_in", "FT_in", "DPMfull_BP", "sync_rdy_all", "ext_fasttrig_fp", "lvds_fasttrig_fp", "ext_validtrig_fp", "lvds_validtrig_fp", "chantrig_or"], "bit_low": xia.TC0_6TH_TEST_OFFSET, "bit_high": xia.TC0_6TH_TEST_END, "tooltip": "6th output of digital output for test/debug"},
            7: {"label": "MFTr", "options": ["Ext_FastTrig_In", "FT_LocalCrate_BP", "ET_In_BP", "ET_WiredOr"], "bit_low": xia.TC0_MOD_FAST_TRIG_OFFSET, "bit_high": xia.TC0_MOD_FAST_TRIG_END, "tooltip": "Module fast trigger source"},
            8: {"label": "MVTr", "options": ["Ext_FastTrig_In", "FT_LocalCrate_BP", "ET_In_BP", "ET_WiredOr"], "bit_low": xia.TC0_MOD_VAL_TRIG_OFFSET, "bit_high": xia.TC0_MOD_VAL_TRIG_END, "tooltip": "Module validation trigger source"},
            9: {"label": "EvTr", "options": ["Ext_ValidTrig_Sel", "Int_ValidTrig_Sel", "FTIN_Or", "LVDS_ValidTrig_FP", "ChanTrig_Sel"], "bit_low": xia.TC0_EXT_VAL_TRIG_OFFSET, "bit_high": xia.TC0_EXT_VAL_TRIG_END, "tooltip": "External validation trigger source"}
        }

        # Subwidget configuration. Define the grid and add widgets.
        
        self.grid = QWidget()
        self.grid.setWindowTitle("TrigConfig0 settings")
        self.param_grid = QGridLayout(self.grid)
        
        for idx, param in self.param_labels.items():
            w = QLabel(param["label"])
            w.setFixedHeight(20)
            w.setToolTip(param["tooltip"])
            self.param_grid.addWidget(w, 0, idx+1, Qt.AlignCenter)
        
        ##
        # Signal connections
        #
        
        self.b_show_config.clicked.connect(self._show_config)
        
    def configure(self, mgr):
        """Initialize and display widget settings from the DSP dataframe.

        Parameters
        ----------
        mgr : DSPManager
             Manager for internal DSP and interface for XIA API read/write 
             operations.
        """
        for i in range(self.nmodules): 
            self.param_grid.addWidget(QLabel("Mod. %i" %i), i+1, 0)
            for j, pdict in self.param_labels.items():
                cb = QComboBox()
                if "options" in pdict:
                    for option in pdict["options"]:
                        cb.insertItem(pdict["options"].index(option), option)
                else:
                    for ch in range(self.nchannels):
                        cb.insertItem(ch, str(ch))
                self.param_grid.addWidget(cb, i+1, j+1)
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
            tc = zeros(32, "little")
            for j, pdict in self.param_labels.items():                
                # Number of bits for this TrigConfig0 setting:                
                tc[pdict["bit_low"]:pdict["bit_high"]] = int2ba(int(self.param_grid.itemAtPosition(i+1, j+1).widget().currentIndex()), pdict["bit_high"]-pdict["bit_low"], "little")

            #print(f"Mod. {i} TrigConfig0: 0x{ba2int(tc):08x}")
            mgr.set_mod_par(i, self.param_names[0], ba2int(tc))
        
    def display_dsp(self, mgr, set_state=False):
        """Update GUI with dataframe values.

        Parameters
        ----------
        mgr : DSPManager
             Manager for internal DSP and interface for XIA API read/write 
             operations.
        set_state : bool, default=False
            Set the TrigConfig0 button state based on the current confiugration
            settings.
        """
        if set_state:
            self.rbgroup.button(self._get_crate_config(mgr)).setChecked(True)
        
        for i in range(self.nmodules):
            val = mgr.get_mod_par(i, self.param_names[0])
            tc = int2ba(int(val), 32, "little")            
            for j, pdict in self.param_labels.items():                    
                self.param_grid.itemAtPosition(i+1, j+1).widget().setCurrentIndex(ba2int(tc[pdict["bit_low"]:pdict["bit_high"]]))

        self._disable_settings()

    def set_param_grid(self, mgr):
        """Set the TrigConfig0 parameter grid based on the crate configuration.

        If the trigger settings are known, configure according to the rb_dict. 
        For custom TrigConfig0 show whatever is currently in the dataframe.
        
        Parameters
        ----------
        mgr : DSPManager 
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        """
        if self.rbgroup.checkedId() == 0:
            for i in range(self.nmodules):
                for j, _ in enumerate(self.param_labels):                    
                    self.param_grid.itemAtPosition(i+1, j+1).widget().setCurrentIndex(0)                    
            self._disable_settings()
        else:
            self.display_dsp(mgr)    
        
    ##
    # Private methods
    #
    
    def _show_config(self):
        """Display the TrigConfig0 GUI."""        
        self.grid.show()
            
    def _get_crate_config(self, mgr):
        """Get the trigger configuration settings from the internal DSP.

        Parameters
        ----------
        mgr : DSPManager
             Manager for internal DSP and interface for XIA API read/write 
             operations.

        Returns
        -------
        int
            Value corresponding to the crate configuration stored in the
            CSRB settings.
        """       
        config = -1
        
        # Assume the configuration is default until proven otherwise:        
        for idx, info in self.rb_dict.items():
            if info["name"] == "Default (self-trigger)":
                config = idx

        # Custom is any module having a TrigConfig0 value not equal to 0:
        for i in range(self.nmodules):
            if mgr.get_mod_par(i, self.param_names[0]) != 0:
                for idx, info in self.rb_dict.items():
                    if info["name"] == "Custom":
                        config = idx                
                        break

        return config

    def _disable_settings(self):
        """Mask TrigConfig0 settings for non-custom triggering."""        
        # Mask everything if all modules in the crate are self-triggering to
        # prevent users from incorrectly configuring TrigConfig0 settings.
        if self.rbgroup.checkedButton().text() == "Custom":
            for i in range(self.nmodules):
                for j in self.param_labels:
                    self.param_grid.itemAtPosition(i+1, j+1).widget().setEnabled(True)
        else:
            for i in range(self.nmodules):
                for j in self.param_labels:
                    self.param_grid.itemAtPosition(i+1, j+1).widget().setEnabled(False)

class TrigConfig0Builder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """TrigConfig0Builder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns
        -------
        TrigConfig0
            Instance of the DSP class widget.
        """            
        return TrigConfig0(*args, **kwargs)
