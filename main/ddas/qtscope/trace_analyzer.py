from dataclasses import dataclass
import inspect
import logging
import math

import numpy as np


@dataclass
class FilterParameters:
    xdt: float = 0.0
    fast_risetime: int = 0
    fast_gap: int = 0
    cfd_delay: int = 0
    cfd_scale: int = 0
    slow_risetime: int = 0
    slow_gap: int = 0
    tau: int = 0


class TraceAnalyzer:
    """TraceAnalyzer class.

    Provides an interface for calculating filter (time, CFD, energy) output
    for DDAS traces based on the channel DSP settings.

    Attributes
    ----------
    logger : Logger
        QtScope Logger object.
    dsp_mgr : DSPManager
        Manager for internal DSP and interface for XIA API read/write
        operations.
    trace : array
        Single channel ADC trace.
    fast_filter : list
        Fast filter output calculated from the trace.
    cfd : list
        CFD output calculated from the fast filter.
    slow_filter  : list
        Slow filter output calculated from the trace.
    """

    def __init__(self, mgr):
        """TraceAnalyzer constructor.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations. Stored as self.dsp_mgr.
        """
        self.dsp_mgr = mgr
        self.logger = logging.getLogger("qtscope_logger")

        self.trace = None
        self.fast_filter = None
        self.cfd = None
        self.slow_filter = None

    def analyze(self, mod, chan, trace):
        """Call other analyzers to calculate filter output.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.
        trace : array
            Single channel ADC trace.

        Raises
        ------
            Every exception back to the caller.
        """
        self.trace = trace
        try:
            self._compute_filters(mod, chan)
        except:
            raise

    ##
    # Private methods
    #

    def _compute_filters(self, mod, chan):
        """Compute fast, CFD, and slow filter output.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.

        Raises
        ------
        ValueError
            If the stored trace is empty.
        """
        if not self.trace.size:
            raise ValueError("Trace is empty, cannot compute filters")

        filter_params = self._get_filter_parameters(mod, chan)
        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: {filter_params.__repr__()}"
        )

        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating fast filter output for trace from Mod. {mod} Ch. {chan}"
        )
        self._compute_fast_filter(filter_params)

        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating CFD output for fast filter output from Mod. {mod} Ch. {chan}"
        )
        self._compute_cfd(filter_params)

        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating slow filter output for trace from Mod. {mod} Ch. {chan}"
        )
        self._compute_slow_filter(filter_params)

    def _compute_fast_filter(self, fp):
        """Compute the fast filter output.

        Compute fast (timing) filter for a single channel ADC trace.

        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """
        self.fast_filter = np.zeros(len(self.trace))

        FL = fp.fast_risetime
        FG = fp.fast_gap

        # Running sum for filter, first index by hand. The high limit of the
        # trace slice contains a +1 since the slice is not inclusive.
        i0 = 2 * FL + FG - 1
        s0 = np.sum(self.trace[i0 - 2 * FL - FG + 1 : i0 - FL - FG + 1])
        s1 = np.sum(self.trace[i0 - FL + 1 : i0 + 1])
        self.fast_filter[i0] = float(s1) - float(s0)

        # Then run over the rest of the trace. Note that we can drop the +1s
        # on both indices since we start at i0+1 and do not take slices:
        for i in range(i0 + 1, len(self.trace)):
            s0 -= self.trace[i - 2 * FL - FG]
            s0 += self.trace[i - FL - FG]
            s1 -= self.trace[i - FL]
            s1 += self.trace[i]
            self.fast_filter[i] = float(s1) - float(s0)

    def _compute_cfd(self, fp):
        """Compute the CFD.

        Compute the CFD from the fast filter of a single channel ADC trace.

        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """
        n = len(self.fast_filter)
        self.cfd = np.zeros(n)
        w = 1.0 - 0.125 * fp.cfd_scale
        D = fp.cfd_delay

        self.cfd[D:] = w * self.fast_filter[D:] - self.fast_filter[: n - D]

    def _compute_slow_filter(self, fp):
        """Compute the slow filter output.

        Notes
        -----
        Slow (energy) filter calculation for a single-channel ADC trace. For
        more information on the slow filter calculation, see [1]_.

        References
        ----------
        .. [1] H. Tan et al., "A Fast Digital Filter Algorithm for Gamma-Ray
        Spectroscopy With Double-Exponential Decaying Scintillators," IEEE
        T. Nucl. Sci. 51 1541 (2004).

        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """
        self.slow_filter = np.zeros(len(self.trace))

        # Guess a baseline value by averaging 5 samples at the start and end
        # of the trace and taking the minimum value:
        baseline = min(np.mean(self.trace[:5]), np.mean(self.trace[-5:]))

        FL = fp.slow_risetime
        FG = fp.slow_gap

        # Using notation from Tan unless otherwise noted, with time in samples:
        b1 = math.exp(-1 / fp.tau)  # Ratio for geometric series sum Eq. 1.
        bL = math.pow(b1, FL)

        # Coefficients of the inverse matrix Eq. 2 (example matrix elements
        # given on the bottom of pg. 1542):
        a0 = bL / (bL - 1)
        ag = 1
        a1 = 1 / (1 - bL)

        # Running sum for filter, first index by hand. The high limit of the
        # trace slice contains a +1 since the slice is not inclusive.
        i0 = 2 * FL + FG - 1
        s0 = np.sum(
            self.trace[i0 - 2 * FL - FG + 1 : i0 - FL - FG + 1] - baseline
        )  # Trailing
        sg = np.sum(self.trace[i0 - FL - FG + 1 : i0 - FL + 1] - baseline)  # Gap
        s1 = np.sum(self.trace[i0 - FL + 1 : i0 + 1] - baseline)  # Leading
        self.slow_filter[i0] = a0 * s0 + ag * sg + a1 * s1

        for i in range(i0 + 1, len(self.trace)):
            s0 -= self.trace[i - 2 * FL - FG]
            s0 += self.trace[i - FL - FG]
            sg -= self.trace[i - FL - FG]
            sg += self.trace[i - FL]
            s1 -= self.trace[i - FL]
            s1 += self.trace[i]
            self.slow_filter[i] = a0 * s0 + ag * sg + a1 * s1

            if i == len(self.trace) / 2:
                self.logger.debug(
                    f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Sums {s0:.1f} {sg:.1f} {s1:.1f} filter {self.slow_filter[i]:.1f}"
                )

    def _get_filter_parameters(self, mod, chan):
        """Read the filter parameters, convert to samples, and pack them.

        Reads the channel DSP filter parameters from the manager, converts
        each to the nearest integer number of samples, and packs them into a
        FilterParameters object.

        Parameters
        ----------
        mod : int
            Module number.
        chan : int
            Channel number.

        Returns
        -------
        FilterParameters
            The filter parameters for the module/channel, in samples.
        """
        # Load DSP needed to calculate filters:

        xdt = self.dsp_mgr.get_chan_par(mod, chan, "XDT")
        fast_risetime = self.dsp_mgr.get_chan_par(mod, chan, "TRIGGER_RISETIME")
        fast_gap = self.dsp_mgr.get_chan_par(mod, chan, "TRIGGER_FLATTOP")
        cfd_scale = self.dsp_mgr.get_chan_par(mod, chan, "CFDScale")
        cfd_delay = self.dsp_mgr.get_chan_par(mod, chan, "CFDDelay")
        slow_risetime = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_RISETIME")
        slow_gap = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_FLATTOP")
        tau = self.dsp_mgr.get_chan_par(mod, chan, "TAU")

        # Warn users that short filters may not display correctly:

        if 2 * fast_risetime + fast_gap <= xdt:
            print(
                f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: WARNING: Fast filter length {2*fast_risetime + fast_gap} <= XDT sampling {xdt}\n\tThe analyzed trace may not display properly!"
            )

        if 2 * slow_risetime + slow_gap <= xdt:
            print(
                f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: WARNING: Slow filter length {2*slow_risetime + slow_gap} <= XDT sampling {xdt}\n\tThe analyzed trace may not display properly!"
            )

        # Since we're stuck with XDT binning, round the filter parameters to
        # the nearest integer multiple of the XDT value to convert to length
        # in samples. Because channel DSP parameters are double we must convert
        # explicitly to integers. Minimum of 1 sample for filter risetimes and
        # CFD delay. Triangular fast filters (gap = 0 samples) are allowed.

        if fast_risetime < xdt:
            fast_risetime = int(1)
        else:
            fast_risetime = int(round(fast_risetime / xdt))
        fast_gap = int(round(fast_gap / xdt))
        if cfd_delay < xdt:
            cfd_delay = int(1)
        else:
            cfd_delay = int(round(cfd_delay / xdt))
        cfd_scale = int(cfd_scale)  # [0, ..., 7] read as a double.
        if slow_risetime < xdt:
            slow_risetime = int(1)
        else:
            slow_risetime = int(round(slow_risetime / xdt))
        slow_gap = int(round(slow_gap / xdt))
        if tau < xdt:
            tau = int(1)
        else:
            tau = int(round(tau / xdt))

        ns = xdt * 1000  # Convert from samples to time in ns.
        print(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Filter calculation requires parameters to\nbe an integer multiple of XDT. Parameters have not been changed for acquisition.\n\t XDT (ns): {ns:.0f}\n\t Trig. risetime (ns): {fast_risetime*ns:.0f}\n\t Trig. gap (ns): {fast_gap*ns:.0f}\n\t CFD scale: {cfd_scale:.0f}\n\t CFD delay (ns): {cfd_delay*ns:.0f}\n\t Ene. risetime (ns): {slow_risetime*ns:.0f}\n\t Ene. gap (ns): {slow_gap*ns:.0f}\n\t Tau (ns): {tau*ns:.0f}"
        )

        return FilterParameters(
            xdt=xdt,
            fast_risetime=fast_risetime,
            fast_gap=fast_gap,
            cfd_delay=cfd_delay,
            cfd_scale=cfd_scale,
            slow_risetime=slow_risetime,
            slow_gap=slow_gap,
            tau=tau,
        )
