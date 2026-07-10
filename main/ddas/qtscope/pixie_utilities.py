from ctypes import *
import logging

import numpy as np

from converters import str2char
from run_type import RunType

_lib = CDLL("libPixieUtilities.so")  # Must be in LD_LIBRARY_PATH.
_logger = logging.getLogger("qtscope_logger")


class PixieError(RuntimeError):
    """Raised for FRIBDAQ/hardware/API failures. Already logged at origin."""


def _check_retval(retval, operation, last_error):
    """Validate a shim return code. Shared by all utility wrappers.

    Parameters
    ----------
    retval : int
        Return code from the ctypes call. Can come from DDAS code,
        XIA API calls or unhandled exceptions in the shim.
    operation : str
        Short description used in the log and exception message.
    last_error : callable
        Zero-argument callable returning the C++-side reason text.
        Only invoked on failure.

    Returns
    -------
    int
        The (non-negative) return code, passed through.

    Raises
    ------
    RuntimeError
        If retval is negative. The message carries the C++-side reason
        and is logged before the exception is raised.
    """
    if retval < 0:
        msg = f"{operation} failed with retval {retval}: {last_error()}"
        _logger.error(msg)
        raise PixieError(msg)
    return retval


"""pixie_utilities.py

The libPixieUtilities.so library contains a set of utilities to read and write 
DSP parameters, start and stop data runs, acquire traces, etc. on the modules 
using the XIA API. This module defines a set of Python classes which interact 
with elements of the XIA Pixie-16 API via the provided shared library using the
Python ctypes interface.

Classes
-------
SystemUtilities 
    Python wrapper for running a 'system' of modules: boot, load/save settings,
    exit, etc. and reading system configuration information.
DSPUtilities
    Python wrapper for reading and writing DSP settings to modules.
RunUtilities
    Python wrapper for managing run states and getting run data from modules.
TraceUtilities
    Python wrapper for reading and analyzing trace data.

"""

##########################################################################
# SystemUtilities
#


class SystemUtilities:
    """Python SystemUtilities.

    Python wrapper for running a 'system' of modules: boot, load/save settings,
    exit, etc. and reading system configuration information.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the SystemUtilities object.
    logger : Logger
        QtScope Logger instance.

    Methods:
    boot()
        Boot the system.
    save_set_file(name)
        Save an XIA settings to file called name.
    load_set_file(name)
        Load XIA settings file called name.
    exit_system()
        Release resources used by the modules prior to exit.
    boot_offline(mode)
        Set the system boot mode.
    get_boot_mode()
        Get the system boot mode.
    get_boot_status()
        Get the system boot status.
    get_num_modules()
        Get the number of installed modules.
    get_module_msps(mod)
        Get the sampling rate in MSPS for module.
    get_module_channel_count(mod)
        Get the module channel count.
    get_last_error_message()
        Get the last error message from the system (DDAS, XIA, ctypes shim...).
    """

    def __init__(self):
        """SystemUtilities class constructor."""
        # Ctor:
        _lib.CPixieSystemUtilities_new.restype = POINTER(c_char)

        # Boot:
        _lib.CPixieSystemUtilities_Boot.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_Boot.restype = c_int

        # Save set file:
        _lib.CPixieSystemUtilities_SaveSetFile.argtypes = [c_void_p, c_char_p]
        _lib.CPixieSystemUtilities_SaveSetFile.restype = c_int

        # Load set file:
        _lib.CPixieSystemUtilities_LoadSetFile.argtypes = [c_void_p, c_char_p]
        _lib.CPixieSystemUtilities_LoadSetFile.restype = c_int

        # Exit system:
        _lib.CPixieSystemUtilities_ExitSystem.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_ExitSystem.restype = c_int

        # Set boot mode:
        _lib.CPixieSystemUtilities_SetBootMode.argtypes = [c_void_p, c_int]
        _lib.CPixieSystemUtilities_SetBootMode.restype = None

        # Get boot mode:
        _lib.CPixieSystemUtilities_GetBootMode.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_GetBootMode.restype = c_int

        # Get boot status:
        _lib.CPixieSystemUtilities_GetBootStatus.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_GetBootStatus.restype = c_bool

        # Get number of modules:
        _lib.CPixieSystemUtilities_GetNumModules.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_GetNumModules.restype = c_int

        # Get module MSPS:
        _lib.CPixieSystemUtilities_GetModuleMSPS.argtypes = [c_void_p, c_int]
        _lib.CPixieSystemUtilities_GetModuleMSPS.restype = c_int

        # Get number of channels:
        _lib.CPixieSystemUtilities_GetModuleChannelCount.argtypes = [c_void_p, c_int]
        _lib.CPixieSystemUtilities_GetModuleChannelCount.restype = c_int

        # Get last error message:
        _lib.CPixieSystemUtilities_GetLastErrorMessage.argtypes = [c_void_p]
        _lib.CPixieSystemUtilities_GetLastErrorMessage.restype = c_char_p

        # Dtor:
        _lib.CPixieSystemUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = _lib.CPixieSystemUtilities_new()

    def boot(self):
        """Wrapper function to system boot.

        Raises
        ------
        RuntimeError
            If the boot fails.
        """
        _check_retval(
            _lib.CPixieSystemUtilities_Boot(self.obj),
            "System boot",
            self.get_last_error_message,
        )
        _logger.info("System boot successful")

    def save_set_file(self, name):
        """Wrapper function to save an XIA settings file.

        Parameters
        ----------
        name : str
            Name of the file to save.

        Raises
        ------
        RuntimeError
            If the save operation fails.
        """
        _check_retval(
            _lib.CPixieSystemUtilities_SaveSetFile(self.obj, str2char(name)),
            "Save settings file",
            self.get_last_error_message,
        )
        _logger.info(f"Settings file saved to {name}")

    def load_set_file(self, name):
        """Wrapper function to load an XIA settings file.

        Parameters
        ----------
        name : str
            Name of the file to load.

        Raises
        ------
        RuntimeError
            If the load operation fails.
        """
        _check_retval(
            _lib.CPixieSystemUtilities_LoadSetFile(self.obj, str2char(name)),
            "Load settings file",
            self.get_last_error_message,
        )
        _logger.info(f"Settings file loaded from {name}")

    def exit_system(self):
        """Wrapper for system exit.

        Releases resources used by the modules.

        Raises
        ------
        RuntimeError
            If a module fails to exit properly.
        """
        _check_retval(
            _lib.CPixieSystemUtilities_ExitSystem(self.obj),
            "System exit",
            self.get_last_error_message,
        )
        _logger.info("System exit successful")

    def boot_offline(self, offline=False):
        """Wrapper to set system boot mode.

        Boot modules in offline mode with no attached hardware or online
        mode with hardware. Offline boot mode is configured by reading the
        value of the envoironment variable QTSCOPE_OFFLINE at execution.

        Parameters
        ----------
        offline : bool, default=False
            If True, boot the system in offline mode.
        """
        boot_mode = 1 if offline else 0
        _lib.CPixieSystemUtilities_SetBootMode(self.obj, boot_mode)

    def get_boot_mode(self):
        """Wrapper to get system boot mode (online or offline).

        Returns
        -------
        int
            Offline (1) or online (0) module boot mode flag.
        """
        return _check_retval(
            _lib.CPixieSystemUtilities_GetBootMode(self.obj),
            "Get boot mode",
            self.get_last_error_message,
        )

    def get_boot_status(self):
        """Wrapper to get the boot status of the system.

        Returns
        -------
        bool
            True if the system has been booted, otherwise False.
        """
        return _lib.CPixieSystemUtilities_GetBootStatus(self.obj)

    def get_num_modules(self):
        """Wrapper to get the number of modules present in the system.

        Returns
        -------
        int
            Number of modules installed in the system.
        """
        return _check_retval(
            _lib.CPixieSystemUtilities_GetNumModules(self.obj),
            "Get number of modules",
            self.get_last_error_message,
        )

    def get_module_msps(self, module):
        """Wrapper to read ADC sampling rate in MSPS from a module.

        Returns
        -------
        int
            Sampling rate in MSPS.
        """
        return _check_retval(
            _lib.CPixieSystemUtilities_GetModuleMSPS(self.obj, module),
            "Get module MSPS",
            self.get_last_error_message,
        )

    def get_module_channel_count(self, module):
        """Wrapper to read the module channel count.

        Returns
        -------
        unsigned
            Channels on the module.
        """
        return _check_retval(
            _lib.CPixieSystemUtilities_GetModuleChannelCount(self.obj, module),
            "Get module channel count",
            self.get_last_error_message,
        )

    def get_last_error_message(self):
        """Wrapper to get the last error message from the system.

        Returns
        -------
        str
            Last error message from the system (DDAS, XIA, ctypes shim...).
        """
        return _lib.CPixieSystemUtilities_GetLastErrorMessage(self.obj).decode("utf-8")

    def __del__(self):
        """SystemUtilities class destructor."""
        _lib.CPixieSystemUtilities_delete(self.obj),


##########################################################################
# DSP Utilities
#


class DSPUtilities:
    """Python DSPUtilities.

    Python wrapper for reading and writing DSP settings to modules.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the DSPUtilities object.
    logger : Logger
        QtScope Logger instance.

    Methods
    -------
    adjust_offsets(module)
        Adjust DC offsets on a single module.
    write_chan_par(module, channel, name, val)
        Write a channel parameter.
    read_chan_par(module, channel, name)
        Read a channel parameter.
    write_mod_par(module, name, val)
        Write a module parameter.
    read_mod_par(module, name)
        Read a module parameter.
    """

    def __init__(self):
        """DSPUtilities class constructor"""
        # Ctor:
        _lib.CPixieDSPUtilities_new.restype = POINTER(c_char)

        # Adjust offsets:
        _lib.CPixieDSPUtilities_AdjustOffsets.argtypes = [c_void_p, c_int]
        _lib.CPixieDSPUtilities_AdjustOffsets.restype = c_int

        # Write channel parameter:
        _lib.CPixieDSPUtilities_WriteChanPar.argtypes = [
            c_void_p,
            c_int,
            c_int,
            c_char_p,
            c_double,
        ]
        _lib.CPixieDSPUtilities_WriteChanPar.restype = c_int

        # Read channel parameter:
        _lib.CPixieDSPUtilities_ReadChanPar.argtypes = [
            c_void_p,
            c_int,
            c_int,
            c_char_p,
            POINTER(c_double),
        ]
        _lib.CPixieDSPUtilities_ReadChanPar.restype = c_int

        # Write module parameter:
        _lib.CPixieDSPUtilities_WriteModPar.argtypes = [
            c_void_p,
            c_int,
            c_char_p,
            c_uint,
        ]
        _lib.CPixieDSPUtilities_WriteModPar.restype = c_int

        # Read module parameter:
        _lib.CPixieDSPUtilities_ReadModPar.argtypes = [
            c_void_p,
            c_int,
            c_char_p,
            POINTER(c_uint),
        ]
        _lib.CPixieDSPUtilities_ReadModPar.restype = c_int

        # Dtor:
        _lib.CPixieDSPUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = _lib.CPixieDSPUtilities_new()
        self.logger = logging.getLogger("qtscope_logger")

    def adjust_offsets(self, module):
        """Wrapper to adjust DC offsets for all channels on a given module.

        Parameters
        ----------
        module : int
            Module number.

        Raises
        ------
        RuntimeError
            If the offset adjustment fails.
        """
        try:
            retval = _lib.CPixieDSPUtilities_AdjustOffsets(self.obj, module)
            if retval < 0:
                raise RuntimeError(
                    f"Failed to adjust offsets in Mod. {module} with "
                    f"retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to adjust offsets")
            print(e)

    def write_chan_par(self, module, channel, name, val):
        """Wrapper to write a channel parameter to a module.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.
        name : str
            Parameter name.
        val : float
            Channel parameter value.

        Raises
        ------
        RuntimeError
            If the write operation fails.
        """
        try:
            retval = _lib.CPixieDSPUtilities_WriteChanPar(
                self.obj, module, channel, str2char(name), val
            )
            if retval < 0:
                raise RuntimeError(
                    f"Failed to write parameter {name} to Mod. {module}, "
                    f"Ch. {channel} with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to write channel parameter")
            print(e)

    def read_chan_par(self, module, channel, name):
        """Wrapper to read a channel parameter from a module.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.
        name : str
            Parameter name.

        Returns
        -------
        float
            Value of the read parameter if success.
        None
            If exception is raised.

        Raises
        ------
        RuntimeError
            If the read operation fails.
        """
        read_param = c_double()
        try:
            retval = _lib.CPixieDSPUtilities_ReadChanPar(
                self.obj, module, channel, str2char(name), byref(read_param)
            )
            if retval < 0:
                raise RuntimeError(
                    f"Failed to read parameter {name} from Mod. {module}, "
                    f"Ch. {channel} with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to read channel parameter")
            print(e)
            return None
        else:
            return read_param.value

    def write_mod_par(self, module, name, val):
        """Wrapper to write a module parameter.

        The parameter name is converted from Python string to char*, parameter
        value is converted to an int.

        Parameters
        ----------
        module : int
            Module number.
        name : str
            Parameter name.
        val : float
            Channel parameter value.

        Raises
        ------
        RuntimeError
            If the write operation fails.
        """
        try:
            retval = _lib.CPixieDSPUtilities_WriteModPar(
                self.obj, module, str2char(name), int(val)
            )
            if retval < 0:
                raise RuntimeError(
                    f"Failed to write parameter {name} to Mod. {module} "
                    f"with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to write module parameter")
            print(e)

    def read_mod_par(self, module, name):
        """Wrapper to read a module parameter.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int
            Module number.
        name : str
            Parameter name.

        Returns
        -------
        int
            Value of the read parameter if success.
        None
            If exception is raised.

        Raises
        ------
        RuntimeError
            If the read operation fails.
        """
        read_param = c_uint()
        try:
            retval = _lib.CPixieDSPUtilities_ReadModPar(
                self.obj, module, str2char(name), byref(read_param)
            )
            if retval < 0:
                raise RuntimeError(
                    f"Failed to read paramter {name} from Mod. {module} "
                    f"with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module parameter")
            print(e)
            return None
        else:
            return read_param.value

    def __del__(self):
        """DSPUtilities destructor."""
        return _lib.CPixieDSPUtilities_delete(self.obj)


##########################################################################
# RunUtilities
#


class RunUtilities:
    """Python wrapper for managing run states and getting run data.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the RunUtilities object.
    logger : Logging
        QtScope Logging instance.

    Methods
    -------
    begin_run(module, channels, run_type)
        Begin a histogram or baseline run in a single module.
    end_run(module, run_type)
        End histogram or baseline run in a single module.
    read_data(module, channel, run_type)
        Read data histograms for a single module.
    read_stats(module)
        Read run statistics from the specified module.
    get_data(run_type)
        Get single channel histogram or baseline data.
    get_run_active()
        Get the active run status of the system.
    use_generator_data(mode)
        Set ParameterManager offline mode.
    get_histogram_length(module)
        Get the histogram length for a single module.
    get_max_baselines(module)
        Get the maximum number of baselines for a single module.
    """

    def __init__(self):
        """RunUtilities class constructor."""
        # Ctor:
        _lib.CPixieRunUtilities_new.restype = POINTER(c_char)

        # Begin histogram data run:
        _lib.CPixieRunUtilities_BeginHistogramRun.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieRunUtilities_BeginHistogramRun.restype = c_int

        # End histogram data run:
        _lib.CPixieRunUtilities_EndHistogramRun.argtypes = [c_void_p, c_int]
        _lib.CPixieRunUtilities_EndHistogramRun.restype = c_int

        # Read histogram from module:
        _lib.CPixieRunUtilities_ReadHistogram.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieRunUtilities_ReadHistogram.restype = c_int

        # Begin baseline data run:
        _lib.CPixieRunUtilities_BeginBaselineRun.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieRunUtilities_BeginBaselineRun.restype = c_int

        # End baseline data run:
        _lib.CPixieRunUtilities_EndBaselineRun.argtypes = [c_void_p, c_int]
        _lib.CPixieRunUtilities_EndBaselineRun.restype = c_int

        # Read baseline from module:
        _lib.CPixieRunUtilities_ReadBaseline.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieRunUtilities_ReadBaseline.restype = c_int

        # Read run statistics from module:
        _lib.CPixieRunUtilities_ReadModuleStats.argtypes = [c_void_p, c_int]
        _lib.CPixieRunUtilities_ReadModuleStats.restype = c_int

        # Returns a pointer to the underlying histogram data from the vector:
        _lib.CPixieRunUtilities_GetHistogramData.argtypes = [c_void_p]
        _lib.CPixieRunUtilities_GetHistogramData.restype = POINTER(c_uint)

        # Returns a pointer to the underlying baseline data from the vector:
        _lib.CPixieRunUtilities_GetBaselineData.argtypes = [c_void_p]
        _lib.CPixieRunUtilities_GetBaselineData.restype = POINTER(c_uint)

        # Run active status:
        _lib.CPixieRunUtilities_GetRunActive.argtypes = [c_void_p]
        _lib.CPixieRunUtilities_GetRunActive.restype = c_bool

        # Use generator data:
        _lib.CPixieRunUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        _lib.CPixieRunUtilities_SetUseGenerator.restype = c_void_p

        # Get histogram length
        _lib.CPixieRunUtilities_GetHistogramLength.argtypes = [c_void_p, c_int]
        _lib.CPixieRunUtilities_GetHistogramLength.restype = c_int

        # Get max baselines
        _lib.CPixieRunUtilities_GetMaxBaselines.argtypes = [c_void_p, c_int]
        _lib.CPixieRunUtilities_GetMaxBaselines.restype = c_int

        # Dtor:
        _lib.CPixieRunUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = _lib.CPixieRunUtilities_new()
        self.logger = logging.getLogger("qtscope_logger")

    def begin_run(self, module, channels, run_type):
        """Wrapper to begin a histogram run in a single module.

        Parameters
        ----------
        module : int
            Module number.
        channels : int
            Channels on this module.
        run_type : Enum member
            Type of run to begin.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        RuntimeError
            If the start run operation fails.
        """
        try:
            if run_type == RunType.HISTOGRAM:
                retval = _lib.CPixieRunUtilities_BeginHistogramRun(
                    self.obj, module, channels
                )
                if retval < 0:
                    raise RuntimeError(
                        f"Begin histogram run in Mod. {module} failed with retval {retval}"
                    )
            elif run_type == RunType.BASELINE:
                retval = _lib.CPixieRunUtilities_BeginBaselineRun(
                    self.obj, module, channels
                )
                if retval < 0:
                    raise RuntimeError(
                        f"Begin baseline run in Mod. {module} failed with retval {retval}"
                    )
            else:
                raise ValueError(
                    f"Unable to begin run in Mod. {module}, run type {run_type} is not a valid type of data run"
                )
        except ValueError as e:
            self.logger.exception("Attempted to begin unrecognized run type")
            print(e)
        except RuntimeError as e:
            self.logger.exception("Failed to begin run")
            print(e)

    def end_run(self, module, run_type):
        """Wrapper to end a histogram run in a single module.

        Parameters
        ----------
        module : int
            Module number.
        run_type : Enum member
            Type of run to begin.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        """
        try:
            if run_type == RunType.HISTOGRAM:
                _lib.CPixieRunUtilities_EndHistogramRun(self.obj, module)
            elif run_type == RunType.BASELINE:
                _lib.CPixieRunUtilities_EndBaselineRun(self.obj, module)
            else:
                raise ValueError(
                    f"Unable to end run in Mod. {module} with unknown run type {run_type}"
                )
        except ValueError as e:
            self.logger.exception(f"Failed to end data run")
            print(e)

    def read_data(self, module, channel, run_type):
        """Wrapper to read run data from a single channel.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.
        run_type : Enum member
            Type of run data to read.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        RuntimeError
            If the API data read fails.
        """
        try:
            if run_type == RunType.HISTOGRAM:
                retval = _lib.CPixieRunUtilities_ReadHistogram(
                    self.obj, module, channel
                )
                if retval < 0:
                    raise RuntimeError(
                        f"Histogram read from Mod. {module}, Ch. {channel} failed with retval {retval}"
                    )
            elif run_type == RunType.BASELINE:
                retval = _lib.CPixieRunUtilities_ReadBaseline(self.obj, module, channel)
                if retval < 0:
                    raise RuntimeError(
                        f"Baseline read from Mod. {module}, Ch. {channel} failed with retval {retval}"
                    )
            else:
                raise ValueError(
                    f"Unable to read data from Mod. {module} for unknownrun type {run_type}"
                )
        except ValueError as e:
            self.logger.exception(f"Encountered unknown run type")
            print(e)
        except RuntimeError as e:
            self.logger.exception(f"Failed to read run data")
            print(e)

    def read_stats(self, module):
        """Wrapper to read the run statistics from a single module.

        Parameters
        ----------
        module : int
            Module number.

        Raises
        ------
        RuntimeError
            If the stats read fails.
        """
        try:
            retval = _lib.CPixieRunUtilities_ReadModuleStats(self.obj, module)
            if retval < 0:
                raise RuntimeError(
                    f"Reading statistics from Mod. {module} failed with "
                    f"retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to read run statistics")
            print(e)

    def get_data(self, module, run_type):
        """
        Wrapper to provide access the acquired energy histogram data.

        Parameters
        ----------
        module : int
            Module number.
        run_type : Enum member
            Type of run data to retrieve.

        Returns
        -------
        list
            Python list containing the list-mode run histogram or baseline
            histogram data with default 1 ADC unit/channel binning.
        """
        size = _lib.CPixieRunUtilities_GetHistogramLength(self.obj, module)

        if run_type == RunType.HISTOGRAM:
            d = _lib.CPixieRunUtilities_GetHistogramData(self.obj)
        elif run_type == RunType.BASELINE:
            d = _lib.CPixieRunUtilities_GetBaselineData(self.obj)

        return np.array(d[:size], dtype=np.uint32)

    def get_run_active(self):
        """Wrapper to get the active run status.

        Returns
        -------
        bool
            True if a run is active, False otherwise.
        """
        return _lib.CPixieRunUtilities_GetRunActive(self.obj)

    def use_generator_data(self, mode):
        """Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """
        return _lib.CPixieRunUtilities_SetUseGenerator(self.obj, mode)

    def get_histogram_length(self, module):
        """Wrapper to read the histogram length for a single module.

        Returns
        -------
        int
            Number of bins in the histogram or -1 if error.
        """
        try:
            nbins = _lib.CPixieRunUtilities_GetHistogramLength(self.obj, module)
            if nbins < 0:
                raise RuntimeError(
                    f"Failed to read Mod. {module} histogram length with retval {nbins}"
                )
            return nbins
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module {module} histogram length")
            print(e)
            return -1

    def get_max_baselines(self, module):
        """Wrapper to get the maximum number of baselines for a single module.

        Parameters
        ----------
        module : int
            Module number.

        Returns
        -------
        int
            Maximum number of baselines or -1 if error.
        """
        try:
            max_baselines = _lib.CPixieRunUtilities_GetMaxBaselines(self.obj, module)
            if max_baselines < 0:
                raise RuntimeError(
                    f"Failed to read Mod. {module} maximum baselines with retval {max_baselines}"
                )
            return max_baselines
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module {module} maximum baselines")
            print(e)
            return -1

    def __del__(self):
        """RunUtilities class destructor."""
        return _lib.CPixieRunUtilities_delete(self.obj)


##########################################################################
# TraceUtilities
#


class TraceUtilities:
    """Python wrapper for reading and analyzing trace data.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the TraceUtilities object.

    Methods
    -------
    read_trace(module, channel)
        Read trace from module/channel.
    read_fast_trace(module, channel)
        Read unvalidated trace from module/channel.
    get_trace_data(module)
        Access the trace data.
    use_generator_data(mode)
        Set use of trace data generator to bool value for testing.
    get_trace_length(module)
        Get the trace length for a single module.
    """

    def __init__(self):
        """TraceUtilities constructor."""
        # Ctor:
        _lib.CPixieTraceUtilities_new.restype = POINTER(c_char)

        # Read trace from module:
        _lib.CPixieTraceUtilities_ReadTrace.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieTraceUtilities_ReadTrace.restype = c_int

        # Read trace from module without signal validation:
        _lib.CPixieTraceUtilities_ReadFastTrace.argtypes = [c_void_p, c_int, c_int]
        _lib.CPixieTraceUtilities_ReadFastTrace.restype = c_int

        # Returns a pointer to the underlying trace data from the vector:
        _lib.CPixieTraceUtilities_GetTraceData.argtypes = [c_void_p]
        _lib.CPixieTraceUtilities_GetTraceData.restype = POINTER(c_ushort)

        # Use generator data:
        _lib.CPixieTraceUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        _lib.CPixieTraceUtilities_SetUseGenerator.restype = c_void_p

        # Max trace length:
        _lib.CPixieTraceUtilities_GetTraceLength.argtypes = [c_void_p, c_int]
        _lib.CPixieTraceUtilities_GetTraceLength.restype = c_int

        # Dtor:
        _lib.CPixieTraceUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = _lib.CPixieTraceUtilities_new()
        self.logger = logging.getLogger("qtscope_logger")

    def read_trace(self, module, channel):
        """Wrapper to read a trace from a single channel.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.

        Raises
        ------
        RuntimeError
            If the trace cannot be read.
        """
        try:
            retval = _lib.CPixieTraceUtilities_ReadTrace(self.obj, module, channel)
            if retval < 0:
                raise RuntimeError(
                    f"Read trace from Mod. {module} Ch. {channel} failed "
                    f"with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to read ADC trace data")
            print(e)

    def read_fast_trace(self, module, channel):
        """Wrapper to read an unvalidated trace from an single channel.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.

        Raises
        ------
        RuntimeError
            If the trace cannot be read.
        """
        try:
            retval = _lib.CPixieTraceUtilities_ReadFastTrace(self.obj, module, channel)
            if retval < 0:
                raise RuntimeError(
                    f"Read trace from Mod. {module} Ch. {channel} failed "
                    f"with retval {retval}"
                )
        except RuntimeError as e:
            self.logger.exception(f"Failed to read ADC trace data")
            print(e)

    def get_trace_data(self, module):
        """Wrapper to provide access the acquired trace data.

        Returns
        -------
        NumPy array of trace data.
        """
        size = _lib.CPixieTraceUtilities_GetTraceLength(self.obj, module)
        d = _lib.CPixieTraceUtilities_GetTraceData(self.obj)

        return np.array(d[:size], dtype=np.uint16)

    def use_generator_data(self, mode):
        """Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """
        return _lib.CPixieTraceUtilities_SetUseGenerator(self.obj, mode)

    def get_trace_length(self, module):
        """Wrapper to read the trace length for a single module.

        Returns
        -------
        int
            Number of samples in the trace or -1 if error.
        """
        try:
            trace_len = _lib.CPixieTraceUtilities_GetTraceLength(self.obj, module)
            if trace_len < 0:
                raise RuntimeError(
                    f"Failed to read Mod. {module} trace length with retval {trace_len}"
                )
            return trace_len
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module {module} trace length")
            print(e)
            return -1

    def __del__(self):
        """TraceUtilities destructor. Deletes itself."""
        return _lib.CPixieTraceUtilities_delete(self.obj)
