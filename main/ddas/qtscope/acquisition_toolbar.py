from PyQt5.QtWidgets import (
    QToolBar,
    QPushButton,
    QSpinBox,
    QCheckBox,
    QHBoxLayout,
    QGroupBox,
    QComboBox,
)

import colors
from run_type import RunType


class AcquisitionToolBar(QToolBar):
    """Acquisition-level toolbar.

    Attributes
    ----------
    b_read_trace : QPushButton
        Read and plot trace data from the module.
    b_analyze_trace : QPushButton
        Calculate and plot trace filter output.
    fast_acq : QCheckBox
        Enable/disable fast acquire traces (no signal validation).
    b_read_data : QPushButton
        Read and plot run data from the module.
    b_run_control : QPushButton
        Button to begin and end runs.
    run_type : QComboBox
        Selection box for run type, list-mode histogram or baseline.
    current_mod : QSpinBox
        Module selection for acquisition.
    current_chan : QSpinBox
        Channel selection for acquisition.
    read_all : QCheckBox
        Enable/disable acquire data from all channels on the selected module.

    Methods
    -------
    disable()
        Disable all toolbar widgets.
    enable()
        Enable toolbar widgets for system idle.
    enable_run_active()
        Enable toolbar widgets for an active run.
    set_module_spinbox_range()
        Set the range of the module spinbox.
    set_channel_spinbox_range()
        Set the range of the channel spinbox for the current module.
    """

    def __init__(self, *args, **kwargs):
        """AcquisitionToolBar class constructor."""
        super().__init__(*args, **kwargs)

        self.setMovable(False)

        # Acquire traces widgets:

        trace_acq_box = QGroupBox("Trace acquisition")
        trace_acq_layout = QHBoxLayout()

        self.b_read_trace = QPushButton("Read trace")
        self.b_analyze_trace = QPushButton("Analyze trace")
        self.fast_acq = QCheckBox("Fast acquire")

        self.b_read_trace.setStyleSheet(colors.CYAN)
        self.b_analyze_trace.setStyleSheet(colors.CYAN)

        trace_acq_layout.addWidget(self.b_read_trace)
        trace_acq_layout.addWidget(self.b_analyze_trace)
        trace_acq_layout.addWidget(self.fast_acq)

        trace_acq_box.setLayout(trace_acq_layout)

        # Run control widgets:

        run_control_box = QGroupBox("Run control")
        run_control_layout = QHBoxLayout()

        self.b_read_data = QPushButton("Read data")
        self.b_read_data.setStyleSheet(colors.CYAN)

        self.b_run_control = QPushButton("Begin run")
        self.b_run_control.setStyleSheet(colors.CYAN)

        self.run_type = QComboBox()
        self.run_type.insertItem(RunType.HISTOGRAM.value, "Energy hist.")
        self.run_type.insertItem(RunType.BASELINE.value, "Baseline")

        run_control_layout.addWidget(self.b_read_data)
        run_control_layout.addWidget(self.b_run_control)
        run_control_layout.addWidget(self.run_type)

        run_control_box.setLayout(run_control_layout)

        # Channel selection widgets:

        selection_box = QGroupBox("Channel selection")
        selection_layout = QHBoxLayout()

        # See issue #216: Configure the spin box widgets with two digits
        # to ensure they are created "big enough" to correctly size the
        # main window on creation. Number of modules and number of channels
        # per module are assumed no larger than two digits.

        self.current_mod = QSpinBox()
        self.current_mod.setPrefix("Mod. ")
        self.current_chan = QSpinBox()
        self.current_chan.setPrefix("Chan. ")
        self.read_all = QCheckBox("Read all")

        selection_layout.addWidget(self.current_mod)
        selection_layout.addWidget(self.current_chan)
        selection_layout.addWidget(self.read_all)

        selection_box.setLayout(selection_layout)

        # Define layout:

        self.addWidget(trace_acq_box)
        self.addWidget(run_control_box)
        self.addWidget(selection_box)

        # Set initial states:

        self.disable()

    def disable(self):
        """Disable every child widget in the toolbar group boxes."""
        for c in self.children():
            for gc in c.children():
                if gc.isWidgetType():
                    gc.setEnabled(False)
                    gc.repaint()

    def enable(self):
        """Enable widgets for system idle state."""
        for c in self.children():
            for gc in c.children():
                if gc.isWidgetType():
                    gc.setEnabled(True)
                    gc.repaint()
        self.b_read_data.setEnabled(False)

    def enable_run_active(self):
        """Enable widgets for system running state."""
        # Trace acquisition: all disabled
        self.b_read_trace.setEnabled(False)
        self.b_analyze_trace.setEnabled(False)
        self.fast_acq.setEnabled(False)
        # Run control: read, control enabled, type disabled
        self.b_read_data.setEnabled(True)
        self.b_run_control.setEnabled(True)
        self.run_type.setEnabled(False)
        # Channel selection: module disabled, channel, read all enabled
        self.current_mod.setEnabled(False)
        self.current_chan.setEnabled(True)
        self.read_all.setEnabled(True)

    def set_module_spinbox_range(self, nmodules):
        """Set the range of the module spinbox.

        Parameters
        ----------
        nmodules : int
        Number of modules in the system.
        """
        self.current_mod.setRange(0, nmodules - 1)

    def set_channel_spinbox_range(self, nchannels):
        """Set the range of the channel spinbox for the currently selected
        module. The system may be a mix of 16- and 32-channel boards.

        Parameters
        ----------
        nchannels : int
            Number of channels on the module.
        """
        self.current_chan.setRange(0, nchannels - 1)


class AcquisitionToolBarBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """AcquisitionToolbarBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the toolbar and return it to the caller.

        Returns
        -------
        AcquisitionToolBar
            Instance of the toolbar class.
        """
        return AcquisitionToolBar(*args, **kwargs)
