#!/usr/bin/env python3
import logging
import os
import sys

daqroot = os.getenv("DAQROOT")
if not daqroot:
    sys.exit(
        "ERROR: DAQROOT is undefined, source appropriate daqsetup.bash and run QtScope as $DAQBIN/qtscope"
    )
qtscope_path = os.path.join(daqroot, "ddas", "qtscope")
if not os.path.isdir(qtscope_path):
    sys.exit(
        f"ERROR: {qtscope_path} does not exist. Check that DAQROOT ({daqroot}) points at a valid NSCLDAQ installation"
    )
sys.path.append(qtscope_path)
os.environ["NO_PROXY"] = ""
os.environ["XDG_RUNTIME_DIR"] = os.getcwd()

_logger = logging.getLogger("qtscope_logger")
_logger.setLevel(logging.INFO)  # Default; overridden from env in main()
_logger.propagate = False  # Own our logging; never touch the root logger
_handler = logging.FileHandler("qtscope.log")
_handler.setFormatter(logging.Formatter("%(levelname)s - %(asctime)s: %(message)s"))
_logger.addHandler(_handler)

from PyQt5 import QtWidgets, QtCore

from fit_factory import FitFactory
from widget_factory import WidgetFactory

from adc_trace import TraceBuilder
from analog_signal import AnalogSignalBuilder
from baseline import BaselineBuilder
from cfd import CFDBuilder
from csra import CSRABuilder
from energy_filter import EnergyFilterBuilder
from histogram import HistogramBuilder
from mult_coincidence import MultCoincidenceBuilder
from qdclen import QDCLenBuilder
from tau import TauBuilder
from timing_control import TimingControlBuilder
from trigger_filter import TriggerFilterBuilder

from crate_id import CrateIDBuilder
from csrb import CSRBBuilder
from trigconfig0 import TrigConfig0Builder
from trigconfigextra import TrigConfigExtraBuilder

from acquisition_toolbar import AcquisitionToolBarBuilder
from dsp_toolbar import DSPToolBarBuilder
from plot_toolbar import PlotToolBarBuilder
from system_toolbar import SystemToolBarBuilder

from fit_exp_creator import ExpFitBuilder
from fit_gauss_creator import GaussFitBuilder
from fit_gauss_p1_creator import GaussP1FitBuilder
from fit_gauss_p2_creator import GaussP2FitBuilder

from gui import MainWindow


def main():
    """QtScope main. Create factories and start the GUI."""

    # Read environment variables and configure global settings for this
    # instance of QtScope. Environment variables QTSCOPE_OFFLINE and
    # QTSCOPE_LOG_LEVEL set whether or not to run QtScope in offline mode
    # without any hardware and control the program logging output.

    try:
        log_level = os.getenv("QTSCOPE_LOG_LEVEL", "INFO").upper()
        if log_level not in logging._levelToName.values():
            allowed = logging._levelToName.values()
            raise ValueError(f"QTSCOPE_LOG_LEVEL={log_level} not in {allowed}")
    except Exception:
        _logger.exception("Error occurred while configuring logger")
        print("Failed to configure logger. See qtscope.log for details.")
        sys.exit()
    else:
        _logger.setLevel(log_level)
        _logger.info(f"PATH: {sys.path}")
        _logger.debug(f"Environ: {os.environ}")
        sys.excepthook = _log_uncaught  # Any uncaught errors are logged

    try:
        offline = int(os.getenv("QTSCOPE_OFFLINE", 0))
    except Exception as e:
        logger.exception("Failed to read QTSCOPE_OFFLINE from env")
        sys.exit(f"QtScope main caught an exception:\n\t{e}.")
    else:
        if offline:
            print("\n-----------------------------------")
            print("QtScope running in offline mode!!!")
            print("-----------------------------------\n")

    # Create the factories:

    logger.info("Creating factory methods and registering builders")
    cdf = create_chan_dsp_factory()
    mdf = create_mod_dsp_factory()
    tbf = create_toolbar_factory()
    ftf = create_fit_factory()

    # Start application and open the main GUI window:

    logger.info("Factory creation complete, starting GUI")
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling, True)
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps, True)
    app = QtWidgets.QApplication(sys.argv)
    gui = MainWindow(cdf, mdf, tbf, ftf, 4, offline)
    gui.show()
    sys.exit(app.exec_())


def create_chan_dsp_factory():
    """Create a widget factory and register channel DSP builders with it.

    Returns
    -------
    WidgetFactory
        Factory with registered channel DSP widgets.
    """
    factory = WidgetFactory()
    _logger.("qtscope_logger").debug("Registering channel DSP")
    factory.register_builder("AnalogSignal", AnalogSignalBuilder())
    factory.register_builder("TriggerFilter", TriggerFilterBuilder())
    factory.register_builder("EnergyFilter", EnergyFilterBuilder())
    factory.register_builder("CFD", CFDBuilder())
    factory.register_builder("Trace", TraceBuilder())
    factory.register_builder("Tau", TauBuilder())
    factory.register_builder("CSRA", CSRABuilder())
    factory.register_builder("MultCoincidence", MultCoincidenceBuilder())
    factory.register_builder("TimingControl", TimingControlBuilder())
    factory.register_builder("Baseline", BaselineBuilder())
    factory.register_builder("QDCLen", QDCLenBuilder())
    factory.register_builder("Histogram", HistogramBuilder())

    return factory


def create_mod_dsp_factory():
    """Create a widget factory and register module DSP builders with it.

    Returns
    -------
    WidgetFactory
        Factory with registered module DSP widgets.
    """
    factory = WidgetFactory()
    _logger.("qtscope_logger").debug("Registering module DSP")
    factory.register_builder("CrateID", CrateIDBuilder())
    factory.register_builder("CSRB", CSRBBuilder())
    factory.register_builder("TrigConfig0", TrigConfig0Builder())
    factory.register_builder("TrigConfigExtra", TrigConfigExtraBuilder())

    return factory


def create_toolbar_factory():
    """Create a widget factory and register module DSP builders with it.

    Returns
    -------
    WidgetFactory
        Factory with registered toolbar widgets.
    """
    factory = WidgetFactory()
    _logger.("qtscope_logger").debug("Registering toolbars")
    factory.register_builder("sys", SystemToolBarBuilder())
    factory.register_builder("acq", AcquisitionToolBarBuilder())
    factory.register_builder("dsp", DSPToolBarBuilder())
    factory.register_builder("plot", PlotToolBarBuilder())

    return factory


def create_fit_factory():
    """Create a fit factory and register builders with it.

    Returns
    -------
    FitFactory
        Instance of the fit factory.
    """
    # Fitting functions will do their best to guess parameters from the data
    # range specified in the fitter. Unless otherwise noted these dictionaries
    # which define the initial guesses are not used. This does guarantee
    # however that we _always_ initialize the fitting functions with valid
    # parameter values.
    config_fit_exp = {
        "params": [1, -0.003, 1],  # k = -0.003 approx. 20 us in 60 ns samples.
        "form": "f(x) = p[0]*exp(p[1]*x) + p[2]",
        "count_data": False,
    }

    config_fit_gauss = {
        "params": [1, 0, 1],
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))",
        "count_data": True,
    }

    config_fit_gauss_p1 = {
        "params": [1, 0, 1, 0, 0],
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))\n\t+ p[3] + p[4]*x",
        "count_data": True,
    }

    config_fit_gauss_p2 = {
        "params": [1, 0, 1, 0, 0, 0],
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))\n\t+ p[3] + p[4]*x + p[5]*x^2",
        "count_data": True,
    }

    # Register fit factory classes:

    factory = FitFactory()
    _logger.("qtscope_logger").debug("Registering fit functions")
    factory.register_builder("Exponential", ExpFitBuilder(), config_fit_exp)
    factory.register_builder("Gaussian", GaussFitBuilder(), config_fit_gauss)
    factory.register_builder(
        "Gaussian + linear", GaussP1FitBuilder(), config_fit_gauss_p1
    )
    factory.register_builder(
        "Gaussian + quadratic", GaussP2FitBuilder(), config_fit_gauss_p2
    )

    return factory


def _log_uncaught(exc_type, exc_value, exc_tb):
    """Last-resort backstop: log any uncaught exception before exiting."""
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_tb)   # Ctrl-C is normal; don't log it
        return
    _logger.("qtscope_logger").critical(
        "Uncaught exception", exc_info=(exc_type, exc_value, exc_tb)
    )
    sys.__excepthook__(exc_type, exc_value, exc_tb)  # Keep the normal behavior too


# Run main when executed as a script, the way we intend to do this:

if __name__ == "__main__":
    main()
