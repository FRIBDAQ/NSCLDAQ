# Constants useful for working with Pixie-16 DSP bitmaps. Set useful constants
# here and import this module when needed to avoid using magic numbers related
# to the API or the parameters. Might be nice to actually include these from
# the API header file as a Python module or something... Otherwise they will
# eventually become out of date and very difficult to handle.

# Data sizes:

MAX_HISTOGRAM_LENGTH = 32768
MAX_ADC_TRACE_LEN = 8192
MAX_NUM_BASELINES = 3640

# Control register bits:

CSRA_GOOD_CHAN = 2
CSRA_POLARITY = 5
CSRA_TRACE_ENABLE = 8
CSRA_CHAN_VALIDATION = 13
CSRA_GAIN = 14

# Multiplicity and coincidence settings:

MULT_OFFSET = 22 # Starting bit for high mask.
MULT_NBITS = 3   # Number of bits to store multiplicity.
MULT_END = MULT_OFFSET + MULT_NBITS

# TrigConfig0 bits:

TC0_INT_FAST_TRIG_OFFSET = 0                     # Internal fast trigger
TC0_INT_FAST_TRIG_END = 4
TC0_EXT_FAST_TRIG_OFFSET = TC0_INT_FAST_TRIG_END # External trigger.
TC0_EXT_FAST_TRIG_END = 8
TC0_INT_VAL_TRIG_OFFSET = TC0_EXT_FAST_TRIG_END  # Internal validation.
TC0_INT_VAL_TRIG_END = 12
TC0_TEST_GROUP_OFFSET = TC0_INT_VAL_TRIG_END     # Test group option.
TC0_TEST_GROUP_END = 15
TC0_ENB_TEST_OFFSET = TC0_TEST_GROUP_END         # Digital output enable.
TC0_ENB_TEST_END = 16
TC0_CH_TEST_OFFSET = TC0_ENB_TEST_END            # Channel test output signal.
TC0_CH_TEST_END = 20
TC0_6TH_TEST_OFFSET = TC0_CH_TEST_END            # 6th digital output.
TC0_6TH_TEST_END = 24
TC0_MOD_FAST_TRIG_OFFSET = TC0_6TH_TEST_END      # Module fast trigger.
TC0_MOD_FAST_TRIG_END = 26
TC0_MOD_VAL_TRIG_OFFSET = TC0_MOD_FAST_TRIG_END  # Module validation trigger.
TC0_MOD_VAL_TRIG_END = 28
TC0_EXT_VAL_TRIG_OFFSET = TC0_MOD_VAL_TRIG_END   # External validation trigger.
TC0_EXT_VAL_TRIG_END = 32

# XIA channel parameter names:

CHAN_PARS = [
    "TRIGGER_RISETIME",
    "TRIGGER_FLATTOP",
    "TRIGGER_THRESHOLD",
    "ENERGY_RISETIME",
    "ENERGY_FLATTOP",
    "TAU",
    "TRACE_LENGTH",
    "TRACE_DELAY",
    "VOFFSET",
    "XDT",
    "BASELINE_PERCENT",    
    "CHANNEL_CSRA",
    "BLCUT",
    "FASTTRIGBACKLEN",
    "CFDDelay",
    "CFDScale",
    "CFDThresh",
    "QDCLen0",
    "QDCLen1",
    "QDCLen2",
    "QDCLen3",
    "QDCLen4",
    "QDCLen5",
    "QDCLen6",
    "QDCLen7",
    "ExtTrigStretch",
    "MultiplicityMaskL",
    "MultiplicityMaskH",
    "ExternDelayLen",
    "FtrigoutDelay",
    "ChanTrigStretch",
    "BINFACTOR",
    "EMIN"
]

# XIA module parameter names:

MOD_PARS = [
    "MODULE_CSRB",
    "SLOW_FILTER_RANGE",
    "CrateID",
    "TrigConfig0",
    "TrigConfig1",
    "TrigConfig2",
    "TrigConfig3"
]
