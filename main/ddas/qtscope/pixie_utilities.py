from ctypes import *
import logging

import numpy as np

from converters import str2char
from run_type import RunType

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

lib = CDLL("libPixieUtilities.so")  # Must be in LD_LIBRARY_PATH.

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
    """

    def __init__(self):
        """SystemUtilities class constructor."""
        # Ctor:
        lib.CPixieSystemUtilities_new.restype = POINTER(c_char)

        # Boot:
        lib.CPixieSystemUtilities_Boot.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_Boot.restype = c_int

        # Save set file:
        lib.CPixieSystemUtilities_SaveSetFile.argtypes = [c_void_p, c_char_p]
        lib.CPixieSystemUtilities_SaveSetFile.restype = c_int

        # Load set file:
        lib.CPixieSystemUtilities_LoadSetFile.argtypes = [c_void_p, c_char_p]
        lib.CPixieSystemUtilities_LoadSetFile.restype = c_int

        # Exit system:
        lib.CPixieSystemUtilities_ExitSystem.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_ExitSystem.restype = c_int

        # Set boot mode:
        lib.CPixieSystemUtilities_SetBootMode.argtypes = [c_void_p, c_int]
        lib.CPixieSystemUtilities_SetBootMode.restype = c_void_p

        # Get boot mode:
        lib.CPixieSystemUtilities_GetBootMode.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetBootMode.restype = c_int

        # Get boot status:
        lib.CPixieSystemUtilities_GetBootStatus.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetBootStatus.restype = c_bool

        # Get number of modules:
        lib.CPixieSystemUtilities_GetNumModules.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetNumModules.restype = c_int

        # Get module MSPS:
        lib.CPixieSystemUtilities_GetModuleMSPS.argtypes = [c_void_p, c_int]
        lib.CPixieSystemUtilities_GetModuleMSPS.restype = c_int

        # Get number of channels:
        lib.CPixieSystemUtilities_GetModuleChannelCount.argtypes = [c_void_p, c_int]
        lib.CPixieSystemUtilities_GetModuleChannelCount.restype = c_int

        # Dtor:
        lib.CPixieSystemUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieSystemUtilities_new()
        self.logger = logging.getLogger("qtscope_logger")

    def boot(self):
        """Wrapper function to system boot.

        Raises
        ------
        RuntimeError
            If the boot fails.
        """
        try:
            retval = lib.CPixieSystemUtilities_Boot(self.obj)
            if retval < 0:
                raise RuntimeError(f"System boot failed with retval {retval}")
        except RuntimeError as e:
            self.logger.exception(
                "Failed to boot Pixie modules, see Pixie-16 "
                "log file for more information"
            )
            print(e)
        else:
            self.logger.info("System boot successful")

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
        try:
            retval = lib.CPixieSystemUtilities_SaveSetFile(self.obj, str2char(name))
            if retval < 0:
                raise RuntimeError(f"Save settings file failed with retval {retval}")
        except RuntimeError as e:
            self.logger.exception("Failed to save settings file")
            print(e)

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
        try:
            retval = lib.CPixieSystemUtilities_LoadSetFile(self.obj, str2char(name))
            if retval < 0:
                raise RuntimeError(f"Load settings file failed with retval {retval}")
        except RuntimeError as e:
            self.logger.exception("Failed to load settings file")
            print(e)

    def exit_system(self):
        """Wrapper for system exit.

        Releases resources used by the modules.

        Raises
        ------
        RuntimeError
            If a module fails to exit properly.
        """
        try:
            retval = lib.CPixieSystemUtilities_ExitSystem(self.obj)
            if retval < 0:
                raise RuntimeError(f"System exit failed with retval {retval}")
        except RuntimeError as e:
            self.logger.exception("System failed to exit properly")
            print(e)
        else:
            self.logger.info("Exit system successful")

    def boot_offline(self, offline=False):
        """Wrapper to set system boot mode.

        Boot modules in offline mode with no attached hardware or online
        mode with hardware. Offline boot mode is configured by reading the
        value of the envoironment variable QTSCOPE_OFFLINE at execution.

        Parameters
        ----------
        offline : bool, default=False
            If True, boot the system in offline mode.

        Note
        ----
        Offline boot only supported in XIA API 2 as of 5/26/23.
        """
        boot_mode = 0  # Value of OfflineMode in Pixie16InitSystem().
        if offline:
            boot_mode = 1  # 1 == OFFLINE
        return lib.CPixieSystemUtilities_SetBootMode(self.obj, boot_mode)

    def get_boot_mode(self):
        """Wrapper to get system boot mode (online or offline).

        Returns
        -------
        int
            Offline (1) or online (0) module boot mode flag.
        """
        return lib.CPixieSystemUtilities_GetBootMode(self.obj)

    def get_boot_status(self):
        """Wrapper to get the boot status of the system.

        Returns
        -------
        bool
            True if the system has been booted, otherwise False.
        """
        return lib.CPixieSystemUtilities_GetBootStatus(self.obj)

    def get_num_modules(self):
        """Wrapper to get the number of modules present in the system.

        Returns
        -------
        int
            Number of modules installed in the system.
        """
        return lib.CPixieSystemUtilities_GetNumModules(self.obj)

    def get_module_msps(self, module):
        """Wrapper to read ADC sampling rate in MSPS from a module.

        Returns
        -------
        int
            Sampling rate in MSPS.
        """
        try:
            retval = lib.CPixieSystemUtilities_GetModuleMSPS(self.obj, module)
            if retval < 0:
                raise RuntimeError(
                    f"Failed to read Mod. {module} MSPS with retval {retval}"
                )
            return retval
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module {module} MSPS")
            print(e)

    def get_module_channel_count(self, module):
        """Wrapper to read the module channel count.

        Returns
        -------
        unsigned
            Channels on the module.
        """
        try:
            retval = lib.CPixieSystemUtilities_GetModuleChannelCount(self.obj, module)
            if retval < 0:
                raise RuntimeError(
                    f"Failed to read Mod. {module} channel count with retval {retval}"
                )
            return retval
        except RuntimeError as e:
            self.logger.exception(f"Failed to read module {module} channel count")
            print(e)

    def __del__(self):
        """SystemUtilities class destructor."""
        return lib.CPixieSystemUtilities_delete(self.obj)


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
        lib.CPixieDSPUtilities_new.restype = POINTER(c_char)

        # Adjust offsets:
        lib.CPixieDSPUtilities_AdjustOffsets.argtypes = [c_void_p, c_int]
        lib.CPixieDSPUtilities_AdjustOffsets.restype = c_int

        # Write channel parameter:
        lib.CPixieDSPUtilities_WriteChanPar.argtypes = [
            c_void_p,
            c_int,
            c_int,
            c_char_p,
            c_double,
        ]
        lib.CPixieDSPUtilities_WriteChanPar.restype = c_int

        # Read channel parameter:
        lib.CPixieDSPUtilities_ReadChanPar.argtypes = [
            c_void_p,
            c_int,
            c_int,
            c_char_p,
            POINTER(c_double),
        ]
        lib.CPixieDSPUtilities_ReadChanPar.restype = c_int

        # Write module parameter:
        lib.CPixieDSPUtilities_WriteModPar.argtypes = [
            c_void_p,
            c_int,
            c_char_p,
            c_uint,
        ]
        lib.CPixieDSPUtilities_WriteModPar.restype = c_int

        # Read module parameter:
        lib.CPixieDSPUtilities_ReadModPar.argtypes = [
            c_void_p,
            c_int,
            c_char_p,
            POINTER(c_uint),
        ]
        lib.CPixieDSPUtilities_ReadModPar.restype = c_int

        # Dtor:
        lib.CPixieDSPUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieDSPUtilities_new()
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
            retval = lib.CPixieDSPUtilities_AdjustOffsets(self.obj, module)
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
            retval = lib.CPixieDSPUtilities_WriteChanPar(
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
            retval = lib.CPixieDSPUtilities_ReadChanPar(
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
            retval = lib.CPixieDSPUtilities_WriteModPar(
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
            retval = lib.CPixieDSPUtilities_ReadModPar(
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
        return lib.CPixieDSPUtilities_delete(self.obj)


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
        lib.CPixieRunUtilities_new.restype = POINTER(c_char)

        # Begin histogram data run:
        lib.CPixieRunUtilities_BeginHistogramRun.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_BeginHistogramRun.restype = c_int

        # End histogram data run:
        lib.CPixieRunUtilities_EndHistogramRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_EndHistogramRun.restype = c_int

        # Read histogram from module:
        lib.CPixieRunUtilities_ReadHistogram.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_ReadHistogram.restype = c_int

        # Begin baseline data run:
        lib.CPixieRunUtilities_BeginBaselineRun.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_BeginBaselineRun.restype = c_int

        # End baseline data run:
        lib.CPixieRunUtilities_EndBaselineRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_EndBaselineRun.restype = c_int

        # Read baseline from module:
        lib.CPixieRunUtilities_ReadBaseline.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_ReadBaseline.restype = c_int

        # Read run statistics from module:
        lib.CPixieRunUtilities_ReadModuleStats.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_ReadModuleStats.restype = c_int

        # Returns a pointer to the underlying histogram data from the vector:
        lib.CPixieRunUtilities_GetHistogramData.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetHistogramData.restype = POINTER(c_uint)

        # Returns a pointer to the underlying baseline data from the vector:
        lib.CPixieRunUtilities_GetBaselineData.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetBaselineData.restype = POINTER(c_uint)

        # Run active status:
        lib.CPixieRunUtilities_GetRunActive.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetRunActive.restype = c_bool

        # Use generator data:
        lib.CPixieRunUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        lib.CPixieRunUtilities_SetUseGenerator.restype = c_void_p

        # Get histogram length
        lib.CPixieRunUtilities_GetHistogramLength.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_GetHistogramLength.restype = c_int

        # Get max baselines
        lib.CPixieRunUtilities_GetMaxBaselines.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_GetMaxBaselines.restype = c_int

        # Dtor:
        lib.CPixieRunUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieRunUtilities_new()
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
                retval = lib.CPixieRunUtilities_BeginHistogramRun(
                    self.obj, module, channels
                )
                if retval < 0:
                    raise RuntimeError(
                        f"Begin histogram run in Mod. {module} failed with retval {retval}"
                    )
            elif run_type == RunType.BASELINE:
                retval = lib.CPixieRunUtilities_BeginBaselineRun(
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
                lib.CPixieRunUtilities_EndHistogramRun(self.obj, module)
            elif run_type == RunType.BASELINE:
                lib.CPixieRunUtilities_EndBaselineRun(self.obj, module)
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
                retval = lib.CPixieRunUtilities_ReadHistogram(self.obj, module, channel)
                if retval < 0:
                    raise RuntimeError(
                        f"Histogram read from Mod. {module}, Ch. {channel} failed with retval {retval}"
                    )
            elif run_type == RunType.BASELINE:
                retval = lib.CPixieRunUtilities_ReadBaseline(self.obj, module, channel)
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
            retval = lib.CPixieRunUtilities_ReadModuleStats(self.obj, module)
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
        size = lib.CPixieRunUtilities_GetHistogramLength(self.obj, module)

        if run_type == RunType.HISTOGRAM:
            d = lib.CPixieRunUtilities_GetHistogramData(self.obj)
        elif run_type == RunType.BASELINE:
            d = lib.CPixieRunUtilities_GetBaselineData(self.obj)

        return np.array(d[:size], dtype=np.uint32)

    def get_run_active(self):
        """Wrapper to get the active run status.

        Returns
        -------
        bool
            True if a run is active, False otherwise.
        """
        return lib.CPixieRunUtilities_GetRunActive(self.obj)

    def use_generator_data(self, mode):
        """Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """
        return lib.CPixieRunUtilities_SetUseGenerator(self.obj, mode)

    def get_histogram_length(self, module):
        """Wrapper to read the histogram length for a single module.

        Returns
        -------
        int
            Number of bins in the histogram or -1 if error.
        """
        try:
            nbins = lib.CPixieRunUtilities_GetHistogramLength(self.obj, module)
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
            max_baselines = lib.CPixieRunUtilities_GetMaxBaselines(self.obj, module)
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
        return lib.CPixieRunUtilities_delete(self.obj)


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
        lib.CPixieTraceUtilities_new.restype = POINTER(c_char)

        # Read trace from module:
        lib.CPixieTraceUtilities_ReadTrace.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieTraceUtilities_ReadTrace.restype = c_int

        # Read trace from module without signal validation:
        lib.CPixieTraceUtilities_ReadFastTrace.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieTraceUtilities_ReadFastTrace.restype = c_int

        # Returns a pointer to the underlying trace data from the vector:
        lib.CPixieTraceUtilities_GetTraceData.argtypes = [c_void_p]
        lib.CPixieTraceUtilities_GetTraceData.restype = POINTER(c_ushort)

        # Use generator data:
        lib.CPixieTraceUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        lib.CPixieTraceUtilities_SetUseGenerator.restype = c_void_p

        # Max trace length:
        lib.CPixieTraceUtilities_GetTraceLength.argtypes = [c_void_p, c_int]
        lib.CPixieTraceUtilities_GetTraceLength.restype = c_int

        # Dtor:
        lib.CPixieTraceUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieTraceUtilities_new()
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
            retval = lib.CPixieTraceUtilities_ReadTrace(self.obj, module, channel)
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
            retval = lib.CPixieTraceUtilities_ReadFastTrace(self.obj, module, channel)
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
        size = lib.CPixieTraceUtilities_GetTraceLength(self.obj, module)
        d = lib.CPixieTraceUtilities_GetTraceData(self.obj)

        return np.array(d[:size], dtype=np.uint16)

    def use_generator_data(self, mode):
        """Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """
        return lib.CPixieTraceUtilities_SetUseGenerator(self.obj, mode)

    def get_trace_length(self, module):
        """Wrapper to read the trace length for a single module.

        Returns
        -------
        int
            Number of samples in the trace or -1 if error.
        """
        try:
            trace_len = lib.CPixieTraceUtilities_GetTraceLength(self.obj, module)
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
        return lib.CPixieTraceUtilities_delete(self.obj)
