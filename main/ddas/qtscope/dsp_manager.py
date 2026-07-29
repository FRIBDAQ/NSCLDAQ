import inspect
import logging

import pandas as pd

from pixie_utilities import DSPUtilities
import xia_constants as xia

_logger = logging.getLogger("qtscope_logger")


class DSPManager:
    """Internal DSP parameter management class.

    DSP parameters are stored internally in a dictionary keyed by the module
    number. Each dictionary value corresponds to a complete set of DSP
    parameters which are addressable by the code. The dataframe chan_par is an
    nchannel x nChanDSP dataframe of channel DSP parameters and the dataframe
    mod_par is a 1 x nModDSP dataframe of module DSP parameters. Channel and
    module DSP parameters are defined by XIA.The rest of the system uses this
    management class to modify the internal DSP parameter values stroed in the
    dataframe as well as to read/write values to/from the modules via
    getters/setters and the  DSPUtilities class.

    Attributes
    ----------
    _dsp : dict
        Internal storage of DSP parameters. DSP parameters are stored in a
        nested dictionary of dataframes.
    _utils : DSPUtilities
        XIA API interface utilities to read/write information to/from modules.
    _dsp_deps : dict
        Dictionary of parameter values which depend on the value of other
        parameters. Key is the i.d. parameter name and the value a list of
        dependant parameter names.
    _nmodules : int
        Number of modules installed in the crate.
    _channel_map : list
        List of number of channels per module (zero-)indexed by module number.
    _logger : Logger
        QtScope Logger object.

    Methods
    -------
    initialize_dsp(nmod, channel_map):
        Create and fill the DSP dictionary.
    get_chan_par(mod, chan, pname):
        Get a channel parameter value from the dataframe.
    set_chan_par(mod, chan, pname, value):
        Set a channel parameter value in the dataframe.
    get_mod_par(mod, pname):
        Get a module parameter value from the dataframe.
    set_mod_par(mod, pname, value):
        Set a module parameter value in the dataframe.
    read(mod, pnames):
        Read DSP parameter(s) into the dataframe.
    write(mod, pnames):
        Write DSP parameter(s) from the dataframe.
    adjust_offsets(mod):
        Adjust the DC offset of all channels on a single module.
    load_new_dsp():
        Load new DSP settings into the dataframe.
    """

    def __init__(self):
        """DSPManager class constructor."""
        self._utils = DSPUtilities()

        # Set dependent paramters e.g. TRIGGER_THRESHOLD is automatically
        # adjusted by the API when TRIGGER_RISETIME is. Prevents Principle
        # of Least Astonishment violations.
        self._dsp_deps = {
            "TRIGGER_RISETIME": ["TRIGGER_THRESHOLD"],
            "SLOW_FILTER_RANGE": ["ENERGY_RISETIME", "ENERGY_FLATTOP"],
        }

    def initialize_dsp(self, nmod, channel_map):
        """Initialize the DSP dataframe.

        Read DSP parameters from the modules into the dataframe storage.

        Parameters
        ----------
        nmod : int
            Number of modules installed in the system.
        channel_map : list
            Map of channels per module.
        """
        self._dsp = {}
        self._nmodules = nmod
        self._channel_map = channel_map

        for i in range(self._nmodules):
            self._dsp[i] = {}  # dict of dicts. Key is module number.

            # Load channel parameters:
            cpars = {}
            for pname in xia.CHAN_PARS:
                p = []
                for j in range(self._channel_map[i]):
                    p.append(self._utils.read_chan_par(i, j, pname))
                cpars[pname] = p
            self._dsp[i]["chan_par"] = pd.DataFrame.from_dict(cpars)
            # Load module parameters:
            mpars = {}
            for pname in xia.MOD_PARS:
                mpars[pname] = [self._utils.read_mod_par(i, pname)]
            self._dsp[i]["mod_par"] = pd.DataFrame.from_dict(mpars)

    def get_chan_par(self, mod, chan, pname):
        """Get a channel parameter value from the dataframe.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number on the module.
        pname : str
            Channel parameter name.

        Returns
        -------
        float
            Channel parameter value.
        None
            Channel parameter name is unknown.

        Raises
        ------
        ValueError
            Channel parameter name is unknown.
        """
        if pname not in xia.CHAN_PARS:
            raise ValueError(f"{pname} is not a channel parameter name")

        return self._dsp[mod]["chan_par"].at[chan, pname]

    def set_chan_par(self, mod, chan, pname, value):
        """Set a channel parameter value in the dataframe.

        Ensure the passed value is floating point before writing.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number on the module.
        pname : str
            Channel parameter name.
        value : float
            Channel parameter value to set.

        Raises
        ------
        ValueError
            Channel parameter name is unknown.
        """
        if not isinstance(value, float):
            _logger.warning(
                f"{pname} value {value} is type {type(value)}, converting to float"
            )
            value = float(value)
        if pname not in xia.CHAN_PARS:
            raise ValueError(f"{pname} is not a channel paramter name")
        self._dsp[mod]["chan_par"].at[chan, pname] = value

    def get_mod_par(self, mod, pname):
        """Get a module parameter value from the dataframe.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Module parameter name.

        Returns
        -------
        int
            Module parameter value.
        None
            Module parameter name is unknown.

        Raises
        ------
        ValueError
            Module parameter name is unknown.
        """
        if pname not in xia.MOD_PARS:
            raise ValueError(f"{pname} is not a module paramter name")

        return self._dsp[mod]["mod_par"].at[0, pname]

    def set_mod_par(self, mod, pname, value):
        """Set a module parameter value in the dataframe.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Module parameter name.
        value : int
            Module parameter value to set.

        Raises
        ------
        ValueError
            Module parameter name is unknown.
        """
        if not isinstance(value, int):
            _logger.warning(
                f"{pname} value {value} is type {type(value)}, converting to int"
            )
            value = int(value)
        if pname not in xia.MOD_PARS:
            raise ValueError(f"{pname} is not a module paramter name")
        self._dsp[mod]["mod_par"].at[0, pname] = value

    def read(self, mod, pnames):
        """Read DSP settings into internal storage.

        Parameters
        ----------
        mod : int
            Module number.
        pnames : list
            List of XIA DSP parameter names.

        Raises
        ------
        PixieError
            If a hardware read fails. Already logged at origin.
        ValueError
            If a name is not a known module or channel parameter.
        """
        for p in pnames:
            self._read_and_set_par(mod, p)

    def write(self, mod, pnames):
        """Write DSP settings from internal storage.

        Parameters
        ----------
        mod : int
            Module number.
        pnames : list
            List of XIA DSP parameter names.

        Raises
        ------
        PixieError
            If a hardware write fails. Already logged at origin.
        ValueError
            If a name is not a known module or channel parameter.
        """
        for p in pnames:
            self._get_and_write_par(mod, p)

    def adjust_offsets(self, mod):
        """Adjust DC offsets for all channels on a single module.

        Parameters
        ----------
        mod : int
            Module number.
        """
        self._utils.adjust_offsets(mod)

    def load_new_dsp(self):
        """Load a new set of DSP settings into the dataframe.

        Reloads DSP settings from the module into the dataframe. Most common
        use case is the user loads a new settings file after booting the
        system. DSP settings are loaded on the modules without the need for a
        reboot, but the dataframe will hold the old values until this function
        is called.
        """
        for mod in range(self._nmodules):
            self.read(mod, [*xia.CHAN_PARS, *xia.MOD_PARS])

    ##
    # Private methods
    #

    def _read_and_set_par(self, mod, pname):
        """Read a DSP parameter value from a single module and set the value
        in the dataframe. The parameter to read and set can be either a module
        or channel parameter.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Module or channel parameter name.

        Raises
        ------
        ValueError
            If the parameter name is not known.
        """
        if pname in xia.CHAN_PARS:
            for i in range(self._channel_map[mod]):
                self._read_and_set_chan_par(mod, i, pname)
        elif pname in xia.MOD_PARS:
            self._read_and_set_mod_par(mod, pname)
        else:
            raise ValueError(f"{pname} is not a valid module or channel parameter name")

    def _read_and_set_chan_par(self, mod, chan, pname):
        """Read a single channel parameter value and set it in the dataframe.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.
        pname : str
            Channel parameter name.
        """
        val = self._utils.read_chan_par(mod, chan, pname)
        _logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Read Mod. {mod} Chan. {chan} {pname} {val}"
        )
        self.set_chan_par(mod, chan, pname, val)

    def _read_and_set_mod_par(self, mod, pname):
        """Read a module parameter value and set it in the dataframe.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Module parameter name.
        """
        val = self._utils.read_mod_par(mod, pname)
        _logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Read Mod. {mod} {pname} {val}"
        )
        self.set_mod_par(mod, pname, val)

    def _get_and_write_par(self, mod, pname):
        """Get a parameter value from the dataframe and write it to a module.
        The parameter to get and write can be either a module or channel
        parameter.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Module or channel parameter name.

        Raises
        ------
        ValueError
            If the parameter name is not known.
        """
        if pname in xia.CHAN_PARS:
            for i in range(self._channel_map[mod]):
                self._get_and_write_chan_par(mod, i, pname)
                self._check_and_write_dependent_pars(mod, i, pname)
        elif pname in xia.MOD_PARS:
            self._get_and_write_mod_par(mod, pname)
            for i in range(self._channel_map[mod]):
                self._check_and_write_dependent_pars(mod, i, pname)
        else:
            raise ValueError(f"{pname} is not a valid module or channel parameter name")

    def _get_and_write_chan_par(self, mod, chan, pname):
        """Get a single channel parameter value from the dataframe and write
        it to a module.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.
        pname : str
            Channel parameter name.
        """
        val = self.get_chan_par(mod, chan, pname)
        self._utils.write_chan_par(mod, chan, pname, val)
        _logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Write Mod. {mod} Chan. {chan} {pname} {val}"
        )
        # Read back and set parameters:
        self._read_and_set_chan_par(mod, chan, pname)

    def _get_and_write_mod_par(self, mod, pname):
        """Get a module parameter value from the dataframe and write it to
        a module.

        Parameters
        ----------
        mod : int
            Module number.
        pname : str
            Channel parameter name.
        """
        val = self.get_mod_par(mod, pname)
        if pname == "SLOW_FILTER_RANGE" and self._utils.read_mod_par(mod, pname) != val:
            print(
                f"Module {mod}: New energy filter range = {val} -- filter parameters may have changed!"
            )
        self._utils.write_mod_par(mod, pname, val)
        _logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Write Mod. {mod} {pname} {val}"
        )
        self._read_and_set_mod_par(mod, pname)

    def _check_and_write_dependent_pars(self, mod, chan, pname):
        """Check if the passed parameter name has any dependent parameters.
        If so, get them from the dataframe and write them to the module.
        Dependent parameters are, for now, assumed to be channel parameters.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.
        pname : str
            Channel parameter name.
        """
        if pname in self._dsp_deps:
            for dep_par in self._dsp_deps[pname]:
                _logger.debug(
                    f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Get and write dependent {dep_par} for {pname}"
                )
                self._get_and_write_chan_par(mod, chan, dep_par)
