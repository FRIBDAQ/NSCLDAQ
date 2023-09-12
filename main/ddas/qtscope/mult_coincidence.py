import numpy as np
import inspect
import logging

import bitarray
ver = [int(i) for i in bitarray.__version__.split(".")]
if bool(ver[0] >= 1 or (ver[0] == 1 and ver[1] >= 6)):
    from bitarray.util import ba2int, int2ba, zeros
else:
    from converters import ba2int, int2ba, zeros

from PyQt5.QtGui import QDoubleValidator
from PyQt5.QtWidgets import (
    QWidget, QHBoxLayout, QVBoxLayout, QRadioButton, QButtonGroup,
    QSpinBox, QGroupBox, QLineEdit, QCheckBox, QLabel, QSizePolicy
)

import xia_constants as xia
import colors

class MultCoincidence(QWidget):
    """Multiplicity and coincidence tab widget.

    Tab QWidget for configuring multiplicity and coincidence settings, as well 
    as toggle the necessary CSRA bits to enable coincidence triggering. The 
    channel coincidence width is set here rather than on the TimingControl tab 
    because the coincidence channel masking, multiplicity threshold, and 
    coincidence width are set on the same page as the on/off. Note there is no 
    copy_chan_dsp method for this class because settings are applied to all 
    channels. There is, however, still a copy_mod_dsp method.

    Attributes
    ----------
    param_names : list
        List of DSP parameter names.
    nchannels : int
        Number of channels per module.
    has_extra_params : bool
        Extra parameter flag.
    rbgroup : QButtonGroup
        Radio button group for channel coincidence mode.
    mode_dict : dict
        Dictionary of coincidence mode settings selectable using rbgroup.
    cb_enabled : QCheckBox
        Checkbox for setting all channel validation CSRA bits to enable 
        coincidences on the selected module.
    coinc_width : QLineEdit 
        Channel coincidence width in microseconds set for all channels on the 
        selected module.
    multiplicity_threshold : QSpinBox
        Minimum multiplicity required to trigger for the selected channel 
        coincidence mode set for all channels on the selected module.
    logger : Logger
        QtScope Logging instance.

    Methods
    -------
    configure(mgr, mod)
        Initialize GUI.
    update_dsp(mgr, mod)
        Update DSP from GUI.
    display_dsp(mgr, mod)
        Display current DSP in GUI.
    copy_mod_dsp(mgr, mod)
        Display copy_dsp in GUI.
    """
    
    def __init__(self, module=None, nchannels=16, *args, **kwargs):
        """Multiplicity and coincidence class constructor.

        Initialize the widget, set parameter validators and group configuration
        settings into UI boxes. Note that settings on this tab apply to all 
        channels on the selected module.

        Parameters
        ----------
        module : int
            Module number from factory create method.
        nchannels : int, default=16
            Number of channels per module.
        """        
        super().__init__(*args, **kwargs)

        self.logger = logging.getLogger("qtscope_logger")
        
        self.param_names = [
            "MultiplicityMaskL",
            "MultiplicityMaskH",
            "ChanTrigStretch",
            "CHANNEL_CSRA"
        ]
        self.nchannels = nchannels
        self.has_extra_params = False
        
        ##
        # Channel group box
        #
        
        chan_group_box = QGroupBox("Channel grouping")

        # Channel grouping widget and its subwidgets:
        
        chan_group_widget = QWidget()
        rbgroup_layout = QVBoxLayout()
        chan_group_widget.setLayout(rbgroup_layout)
        self.rbgroup = QButtonGroup(chan_group_widget)

        # The button group dictionary. Key is button id, dictionary containing
        # channel grouping information is value. Channel grouping dicionary
        # keys are the group name, the number of bits to shift, and the channel
        # mask. Shift and mask for unknown is meaningless, and if coincidence
        # is disabled, the mask should be equal to 0.
        
        self.mode_dict = {
            0: {"name": "Off", "shift": 0, "mask": 0x0, "max_mult": 0},
            1: {"name": "8 x 2", "shift": 2, "mask": 0x3, "max_mult": 2},
            2: {"name": "5 x 3", "shift": 3, "mask": 0x7, "max_mult": 3},
            3: {"name": "4 x 4", "shift": 4, "mask": 0xf, "max_mult": 4},
            4: {"name": "2 x 8", "shift": 8, "mask": 0xff, "max_mult": 7},
            5: {"name": "1 x 16", "shift": 16, "mask": 0xffff, "max_mult": 7},
            6: {"name": "Unknown", "shift": None, "mask": None, "max_mult": 0}
        }
        
        for idx, mode in self.mode_dict.items():
            rb = QRadioButton(mode["name"])
            self.rbgroup.addButton(rb, idx) # i.e. button, id.
            rbgroup_layout.addWidget(rb)            
            # User cannot select Unknown:            
            if mode["name"] == "Unknown":
                self.rbgroup.button(idx).setEnabled(False)

        # Add subwidgets to the channel group box:
        
        vbox = QVBoxLayout()
        vbox.addWidget(chan_group_widget)
        chan_group_box.setLayout(vbox)

        ##
        # Settings box
        #
        
        settings_box = QGroupBox("Coincidence settings")
        
        # Settings widget and its subwidgets:
        
        settings_widget = QWidget()
        settings_layout = QVBoxLayout()
        settings_widget.setLayout(settings_layout)
        
        # CSRA channel validation display:
        
        self.status = QLabel("<b>Disabled</b>")
        self.status.setStyleSheet(colors.RED_TEXT)
        status_text = QLabel("Channel validation is: ")
        status_layout = QHBoxLayout()
        status_layout.addWidget(status_text)
        status_layout.addWidget(self.status)
        status_widget = QWidget()
        status_widget.setLayout(status_layout)
        settings_layout.addWidget(status_widget)
        
        # Channel coincidence width (shown on TimingControl):
        
        self.coinc_width = QLineEdit()
        self.coinc_width.setValidator(
            QDoubleValidator(
                0, 999999, 3, notation=QDoubleValidator.StandardNotation
            )
        )
        settings_layout.addWidget(QLabel("Channel coincidence width [us]"))
        settings_layout.addWidget(self.coinc_width)
        
        # Multiplicity to trigger:
        
        self.multiplicity_threshold = QSpinBox()
        settings_layout.addWidget(QLabel("Minimum multiplicity to trigger"))
        settings_layout.addWidget(self.multiplicity_threshold)
        
        # Add subwidgets to the settings box:
        
        vbox = QVBoxLayout()
        vbox.addWidget(settings_widget)
        settings_box.setLayout(vbox)
        
        # Layout for all the consituent widgets above the toolbar:
        
        top_widget = QWidget()
        hbox = QHBoxLayout()
        hbox.addWidget(chan_group_box)
        hbox.addWidget(settings_box)
        hbox.addStretch()
        top_widget.setLayout(hbox)

        # Define layout:
        
        layout = QVBoxLayout()
        layout.addWidget(top_widget)
        layout.addStretch()
        self.setLayout(layout)
        
        ##
        # Signal connections
        #
        
        self.rbgroup.buttonClicked.connect(
            self._configure_multiplicity_threshold
        )

    def configure(self, mgr, mod):
        """Initialize and display widget settings from the DSP dataframe. 

        Checks that channel validation CSRA bit, channel coincidence window 
        and multiplicity threshold are the same for all channels on the module.
        Warns the user that inconsistent or custom values are detected.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API 
            read/write operations.
        mod : int 
            Module number.

        Raises
        ------
        ValueError 
            If the channel validation CSRA bits are inconsistent, if the 
            channel trigger stretch (coincidence window width) values are 
            inconsistent, or if the multiplicity thresholds are inconsistent.
        """
        # Read in channel validation, coincidence window, and multiplicity
        # threshold values to check:        
        enb_list = []
        win_list = []
        mult_list = []
        for i in range(self.nchannels):
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )
            enb = csra[xia.CSRA_CHAN_VALIDATION]
            
            mask = int2ba(
                int(mgr.get_chan_par(mod, i, "MultiplicityMaskH")),
                32, "little"
            )
            mult = mask[xia.MULT_OFFSET:xia.MULT_END]

            enb_list.append(enb)
            win_list.append(mgr.get_chan_par(mod, i, "ChanTrigStretch"))
            mult_list.append(mult)

        # Check the channel validation values. It is possible that users may
        # have a mix of coincidence-triggering channels and free triggering
        # on the same module. Alert the user that this was noticed as it _may_
        # not be their intention.        
        try:
            if not all(enb == enb_list[0] for enb in enb_list):
                raise ValueError(
                    f"Custom CSRA settings detected on Mod. {mod}"
                )
            if enb_list[0]:
                self.status.setText("<b>Enabled</b>")
                self.status.setStyleSheet(colors.GREEN_TEXT)                
        except ValueError as e:
            self.logger.warning(
                f"Custom CSRA settings on Mod. {mod}: {enb_list}"
            )
            print(f"{e}:\n\tThis may be intended, verify your CSRA and MultCoincidence settings.")
            self.status.setText("<b>Custom</b>")
            self.status.setStyleSheet(colors.ORANGE_TEXT)

        # Check the coincidence window values. Coincidence windows _have_ to
        # be the same across the entire module:        
        try:
            if not all (win == win_list[0] for win in win_list):
                raise ValueError(f"Inconsistent channel coincidence width values read on Mod. {mod}")
        except ValueError as e:
            self.logger.exception(
                f"Channel coincidence widths on Mod. {mod}: {win_list}"
            )
            print(f"{e}:n\tVerify MultCoincidence settings and re-apply.\n\tCheck your settings file, it may be corrupt.")

        # Check the threshold. Thresholds _have_ to be the same as well:
        try:
            if not all(mult == mult_list[0] for mult in mult_list):
                raise ValueError(f"Inconsistent multiplicity threshold values on Mod. {mod}")
        except ValueError as e:
            self.logger.exception(
                f"Multiplicity thresholds on Mod. {mod}: {mult_list}"
            )
            print(f"{e}:\n\tVerify MultCoincidence settings and re-apply.\n\tCheck your settings file, it may be corrupt.")
            
        # Whatever happens, display the DSP:        
        self.display_dsp(mgr, mod)
            
    def update_dsp(self, mgr, mod):
        """Update dataframe from GUI values.

        Unlike many other update functions, because the bitmasks have to be 
        configured for each channel based on the GUI settings, this updates 
        parameters "by hand" rather than looping over param_names.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        mod : int
            Module number.
        """        
        # Coincidence width is the same for all channels of the module
        # based on what is set on this tab.        
        width = float(self.coinc_width.text())
        for i in range(self.nchannels):
            mgr.set_chan_par(mod, i, "ChanTrigStretch", width)

        # Set channel grouping from the low multiplicity mask:        
        self._set_channel_group(mgr, mod)

        # Set the multiplicity threshold from the high multiplicity mask:
        mult_bits = int2ba(
            self.multiplicity_threshold.value(),
            xia.MULT_NBITS, "little"
        )                
        for i in range(self.nchannels):
            mask = int2ba(
                int(mgr.get_chan_par(mod, i, "MultiplicityMaskH")),
                32, "little"
            )
            mask[xia.MULT_OFFSET:xia.MULT_END] = mult_bits
            #print(f"Chan. {i} MultiplicityMaskH: 0x{ba2int(tc):08x}")
            mgr.set_chan_par(mod, i, "MultiplicityMaskH", float(ba2int(mask)))

    def display_dsp(self, mgr, mod):
        """Update GUI with dataframe values.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        mod : int
            Module number.
        """        
        # Channel trigger validation and coincidence width are the same for
        # all channels of the module based on what is set on this tab.
        # Setup channel validation checkbox from channel 0 CSRA.        
        enb_list = []
        for i in range(self.nchannels):
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )
            enb_list.append(csra[xia.CSRA_CHAN_VALIDATION])

        if not all(enb == enb_list[0] for enb in enb_list):
            self.status.setText("<b>Custom</b>")
            self.status.setStyleSheet(colors.ORANGE_TEXT)
        elif enb_list[0]:
            self.status.setText("<b>Enabled</b>")
            self.status.setStyleSheet(colors.GREEN_TEXT)
        else:
            self.status.setText("<b>Disabled</b>")
            self.status.setStyleSheet(colors.RED_TEXT)
        
        # Coincidence window from channel 0:        
        val = np.format_float_positional(
            mgr.get_chan_par(mod, 0, "ChanTrigStretch"),
            precision=3,
            unique=False
        )
        self.coinc_width.setText(val)
        
        # Get the channel multiplicity group mode and set the button:        
        self.rbgroup.button(self._get_channel_group(mgr, mod)).setChecked(True)

        # Different coincidence settings will have different maximum
        # allowed multiplicity thresholds so the spin box has to be
        # reconfigured every time the settings are changed. Order __matters__
        # (coincidence grouping first!).        
        self.multiplicity_threshold.setRange(
            0, self.mode_dict[self.rbgroup.checkedId()]["max_mult"]
        )
        
        # Once the group is known, parse the high multiplicity mask from
        # channel 0 to extract the multiplicity threshold required to trigger:
        mask = int2ba(
            int(mgr.get_chan_par(mod, 0, "MultiplicityMaskH")), 32, "little"
        )
        mult = mask[xia.MULT_OFFSET:xia.MULT_END]
        self.multiplicity_threshold.setValue(ba2int(mult))
        
    ##
    # Private methods
    #

    def _get_channel_group(self, mgr, mod):
        """Get the channel group from the low multiplicity mask.
    
        Parses the low multiplicity mask and checks against known channel 
        multiplicity mask settings to determine the type of channel coincidence
        condition from the settings. 5 x 3 coincidence is handled as a special
        case since the last channel on the module doesn't participate. If no 
        known mode is encountered, the function will issue a warning and set 
        the mode to Unknown.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        mod : int
            Module number.

        Returns
        -------
        int 
            Value corresponding to the button id of the identified 
            coincidence mode.
        """        
        for idx, mode in self.mode_dict.items():
            found = True  # Have we found the coincidence group mode yet?

            # Only check for known configurations:            
            if mode["name"] != "Unknown":                
                # Multiplicity mask slice indices:                
                shift = mode["shift"]
                start = 0

                for i in range(self.nchannels):
                    channel_mask = int2ba(
                        int(mgr.get_chan_par(mod, i, "MultiplicityMaskL")),
                        32, "little"
                    )

                    # Mask for this channel for the current channel grouping.
                    # We compare the read in channel_mask to this value.
                    mask = zeros(32, "little")
                    
                    # found == False except when the correct coincidence mode
                    # is identified. found == True means all channels on the
                    # module have the correct low mask for a given coincidence
                    # mode (after proper shifts are applied). For each chunk
                    # of shift channels the mask is shifted to the left by
                    # shift more bits e.g. 000011 --> 001100 for a shift of 2.
                    # 5 x 3 is handled as a special case since channel 15 does
                    # not participate in the 5 x 3 coincidence mask.
                    
                    if shift > 0:                        
                        if mode["name"] == "5 x 3" and i == 15:
                            pass  # Ignore it.
                        else:                            
                            # Update start every shift channels:
                            if i > 0 and i%shift == 0:
                                start = start + shift

                            mask[start:start+shift] = int2ba(mode["mask"])   
                            found = found and channel_mask == mask
                    else:                        
                        # Off mask is all zeroes:                        
                        found = found and channel_mask == mask

                if found:
                    return idx

        # If nothing was found, return the Unknown id value:        
        print(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Encountered unknown channel multiplicity state!")
        key_list = self.mode_dict.keys()
        val_list = self.mode_dict.values()
        return key_list(val_list.index("Unknown"))
    
    def _set_channel_group(self, mgr, mod):
        """Set the channel group from the selected GUI button.
    
        Set the channel coincidence group specified by the selected button. 
        5 x 3 coincidence is handled as a special case since the last channel 
        on the module doesn't participate.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        mod : int
            Module number.
        """        
        # Get the configuration and its settings based on the selected button:
        mode = self.mode_dict[self.rbgroup.checkedId()]

        try:            
            if mode["name"] == "Unknown":
                raise ValueError(f"Attempting to set multiplicity mask on Mod. {mod} for unknown channel multiplicity group")            
        except ValueError as e:
            self.logger.warning(
                f"Attempting to set unknown multiplicity mode on Mod. {mod}"
            )
            print(f"{e}:\n\tPlease select a known multiplicty group and click 'Apply' to update your settings.")
        else:            
            # Multiplicity mask slice indices:            
            shift = mode["shift"]
            start_bit = 0
            
            for i in range(self.nchannels):
                mask = zeros(32, "little")
                
                # For each chunk of shift channels the mask is shifted to
                # the left by shift more bits e.g. 000011 --> 001100 for a
                # shift of 2. 5 x 3 is handled as a special case since
                # channel 15 does not participate in the 5 x 3 coincidence.
                
                if shift > 0:                        
                    if mode["name"] == "5 x 3" and i == 15:
                        mask[15:16] = 1 # It is enabled
                    else:                        
                        # Update start and end every shift channels:
                        if i > 0 and i%shift == 0:
                            start_bit = start_bit + shift
                            
                        mask[start_bit:start_bit+shift] = int2ba(mode["mask"])

                # Write the mask:
                mgr.set_chan_par(
                    mod, i, "MultiplicityMaskL", float(ba2int(mask))
                )

    def _configure_multiplicity_threshold(self):
        """Configure the multiplicity threshold spinbox.

        The maximum threshold is based on the currently selected channel 
        coincidence group. If the old threshold is greater than the new maximum
        threshold for the selected channel grouping, raise an exception with a 
        warning and set the new threshold to the maximum allowed value.

        Raises
        ------
        ValueError
            If the multiplicity threshold is greater than the maxmimum 
            multiplicity threshold for the selected channel grouping.
        """
        mode = self.mode_dict[self.rbgroup.checkedId()]["name"]
        mult = self.multiplicity_threshold.value()
        max_mult = self.mode_dict[self.rbgroup.checkedId()]["max_mult"]        
        try:
            self.multiplicity_threshold.setRange(0, max_mult)
            
            if mult > max_mult:
                raise ValueError(f"Old multiplicity threshold value {mult} is greater than maximum allowed multiplicity threshold {max_mult} for mode {mode}")
        except ValueError as e:
            self.logger.info(f"Resetting displayed mult {mult} to allowed max mult {max_mult} for mode {mode}. Settings have not been applied.")
            print(f"{e}:\n\tSetting maximum multiplicity to {max_mult}. Click 'Apply' to update your settings.")
            self.multiplicity_threshold.setValue(max_mult)
            
    def print(self, mgr, mod):
        """Print the channel multiplicity and coincidence settings."""        
        # Coincidence channel grouping and name:        
        name = self.mode_dict[self.rbgroup.checkedId()]["name"]

        # Grab the data from the dataframe:        
        for i in range(self.nchannels):            
            low_mask = int2ba(
                int(mgr.get_chan_par(mod, i, "MultiplicityMaskL")),
                32, "little"
            )
            high_mask = int2ba(
                int(mgr.get_chan_par(mod, i, "MultiplicityMaskH")),
                32, "little"
            )
            width = float(self.coinc_width.text())
            mult = ba2int(high_mask[xia.MULT_OFFSET:xia.MULT_END])

            print(f"----- {self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Mod. {mod}, Ch. {i} -----")
            print(f"Mask low: {low_mask}")
            print(f"Mask high: {high_mask}")
            print(f"Coincidence width: {width} [us]") 
            print(f"Mult. to trigger: {mult}")
            print(f"Mode: {name}")

class MultCoincidenceBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """MultCoincidenceBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the widget and return it to the caller.

        Returns
        -------
        MultCoincidence 
            Instance of the DSP class widget.
        """            
        return MultCoincidence(*args, **kwargs)
