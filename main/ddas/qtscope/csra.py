import inspect

import bitarray as ba
ver = [int(i) for i in ba.__version__.split(".")]
if bool(ver[0] >= 1 or (ver[0] == 1 and ver[1] >= 6)):
    from bitarray.util import ba2int, int2ba, zeros
else:
    from converters import ba2int, int2ba, zeros

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget, QGridLayout, QLabel, QLineEdit, QVBoxLayout, QCheckBox, QFrame
)

class CSRA(QWidget):
    """Channel CSRA grid widget.
    
    This class interacts only with the internal DSP settings and its own 
    display. param_names is a single parameter, CHANNEL_CSRA, param_labels 
    give abbreviated settings information, detailed settings info is given 
    in the associated tooltips. See Pixie-16 User Manual sec. 3.3.7, Table 3-2
    for more information.

    Attributes
    ----------
    param_names : list 
        List of DSP parameter names.
    param_labels : dict 
        Dictionary of DSP parameter GUI column titles and tooltips.
    nchannels : int 
        Number of channels per module.
    has_extra_params : bool 
        Extra parameter flag.
    param_grid : QGridLayout 
        Grid of QWidgets to display DSP parameters.
    
    Methods
    -------
    configure(mgr, mod)
        Initialize GUI.
    update_dsp(mgr, mod)
        Update DSP from GUI.
    display_dsp(mgr, mod)
        Display current DSP in GUI.
    copy_mod_dsp(mgr, mod)
        Copy another module's DSP to this module's GUI.
    copy_chan_dsp(idx)
        Copy DSP from channel idx in GUI.
    """
    
    def __init__(self, module=None, nchannels=16, *args, **kwargs):
        """CSRA class constructor.
        
        Initialize CSRA widget and set labels. Channel DSP is displayed on an 
        nchannel x nDSP grid of QCheckBox widgets. Note that the actual DSP 
        parameters are 1-indexed while the grid is 0-indexed. 
        
        Parameters
        ----------
        module : int
            Module number from factory create method.
        nchannels : int, default=16
            Number of channels per module.
        """
        super().__init__(*args, **kwargs)
        
        self.nchannels = nchannels
        self.has_extra_params = False
        self.param_names = ["CHANNEL_CSRA"]
        
        # Parameter label dictionary. Key is the CSRA bit number (and column
        # location on x-axis, offset by one for the channel label), values are
        # dictionary defnining the column label and associated tooltip:
        
        self.param_labels = {
            0: {"label": "FTrS", "tooltip": "Fast trigger from system FPGA (=1) or local (=0)"},
            1: {"label": "ETrS", "tooltip": "Validation trigger from LVDS (=1) or sys. FPGA (=0)"},
            2: {"label": "Good", "tooltip": "Enable (=1) or disable (=0) channel"},
            3: {"label": "CTrS", "tooltip": "Channel validation from LVDS (=1) or sys. FPGA (=0)"},
            4: {"label": "Sync", "tooltip": "Disable acquisition in this channel if any channel DPM on this module is full (=1) or only if this channel DPM is full (=0)"},
            5: {"label": "SPol", "tooltip": "Signal polarity (set on AnalogSignal tab)"},
            6: {"label": "Veto", "tooltip": "Enable (=1) or disable (=0) channel veto"},
            7: {"label": "HstE", "tooltip": "Histogram energy (not used)"},
            8: {"label": "Trce", "tooltip": "Trace acquisition (set on Trace tab)"},
            9: {"label": "QDC", "tooltip": "Enable (=1) or disable (=0) QDC sums"},
            10: {"label": "CFD", "tooltip": "Enable (=1) or disable (=0) CFD"},
            11: {"label": "MTrV", "tooltip": "Enable (=1) or disable (=0) module validation trigger"},
            12: {"label": "ES", "tooltip": "Enable (=1) or disable (=0) energy sums"},
            13: {"label": "CTrV", "tooltip": "Enable (=1) or disable (=0) channel validation trigger"},
            14: {"label": "HiG", "tooltip": "Gain (set on AnalogSignal tab)"},
            15: {"label": "PC1", "tooltip": "Pileup rejection, see manual Sec. 3.3.7, Table 3-2"},
            16: {"label": "PC2", "tooltip": "Inverse pileup rejection, see manual Sec. 3.3.7, Table 3-2"},
            17: {"label": "ECut", "tooltip": "Enable (=1) or disable (=0) energy cut trace rejection"},
            18: {"label": "GTrS", "tooltip": "Channel fast trigger selection, depends on bit 0 see manual Sec. 3.3.7 Table 3-2"},
            19: {"label": "CVtS", "tooltip": "Channel veto selection from system FPGA (=1) or LVDS (=0)"},
            20: {"label": "MVtS", "tooltip": "Module veto selection from system FPGA (=1) or LVDS (=0)"},
            21: {"label": "EnTS", "tooltip": "Enable (=1) or disable (=0) recording of timestamp in event header"}
        }
        
        # List of disabled bits by param_label:
        
        disabled = ["SPol", "HstE", "Trce", "HiG", "GTrS", "ECut"]
        
        # Subwidget configuration. Define the grid and add widgets. Each
        # configurable CSRA bit gets its own checkbox with the column given
        # by bit+1 where bit is the CSRA bit value (key of the parameter
        # label dict):
        
        grid = QWidget()
        self.param_grid = QGridLayout(grid)

        self.param_grid.addWidget(QLabel("Ch."),0,0)        
        for bit, pdict in self.param_labels.items():
            w = QLabel(pdict["label"])
            w.setFixedHeight(20)
            w.setToolTip(pdict["tooltip"])
            self.param_grid.addWidget(w, 0, bit+1, Qt.AlignCenter)        

        for i in range(self.nchannels):
            self.param_grid.addWidget(QLabel("%i" %i), i+1, 0)
            for bit, pdict in self.param_labels.items():
                cb = QCheckBox()
                if pdict["label"] in disabled:
                    cb.setEnabled(False)
                self.param_grid.addWidget(cb, i+1, bit+1, Qt.AlignCenter)

        # Horizontal line to separate individual channels from the bit toggle:
        
        row = self.nchannels + 2  # +1 each for title and for this.
        span = len(self.param_labels) + 1  # +1 for Ch. column.
        hline = QFrame()
        hline.setFrameShape(QFrame.HLine)
        self.param_grid.addWidget(hline, row, 0, 1, span)

        # Checkbox to configure all values in the column at once:

        self.param_grid.addWidget(QLabel("SetAll"), row+1, 0)
        for bit, pdict in self.param_labels.items():
            cb = QCheckBox()
            if pdict["label"] in disabled:
                cb.setEnabled(False)
            cb.stateChanged.connect(
                lambda state, bit=bit: self._select_all(bit, state)
            )
            self.param_grid.addWidget(cb, row+1, bit+1, Qt.AlignCenter)
                          
        # Define layout:
        
        layout = QVBoxLayout()             
        layout.addWidget(grid)
        self.setLayout(layout)

    def configure(self, mgr, mod):
        """Initialize and display widget settings from the DSP dataframe.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API 
            read/write operations.
        mod : int 
            Module number.
        """
        self.display_dsp(mgr, mod)

    def update_dsp(self, mgr, mod):
        """Update dataframe from GUI values.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API 
            read/write operations.
        mod : int 
            Module number. 
        """        
        for i in range(self.nchannels):
            csra = zeros(32, "little")
            for bit in self.param_labels:
                csra[bit] = self.param_grid.itemAtPosition(i+1, bit+1).widget().isChecked()
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
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )
            
            for bit, val in zip(self.param_labels, csra):
                self.param_grid.itemAtPosition(i+1, bit+1).widget().setChecked(val)
                
    def copy_chan_dsp(self, idx):
        """Copy channel DSP parameters.

        Copy channel parameters from a single channel (row) to all other 
        channels on the module. Do not modify the underlying dataframe.

        Parameters
        ----------
        idx : int
            Channel (row) index to copy parameters from.
        """        
        copy_params = []       
        for col, _ in enumerate(self.param_labels, 1):
            copy_params.append(
                self.param_grid.itemAtPosition(idx+1, col).widget().isChecked()
            )
        for i in range(self.nchannels):
            for col, p in enumerate(copy_params, 1):
                self.param_grid.itemAtPosition(i+1, col).widget().setChecked(p)

    ##
    # Private methods
    #
    
    def _select_all(self, bit, state):
        """Toggle the values of a single bit for all channels on a module.

        Parameters
        ----------
        bit : int
            CSRA parameter bit index.
        state : int
            The bit state to set for all channels.
        """
        for i in range(self.nchannels):
            self.param_grid.itemAtPosition(i+1, bit+1).widget().setChecked(state)
         
class CSRABuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """CSRABuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        CSRA
            Instance of the DSP class widget.
        """            
        return CSRA(*args, **kwargs)
