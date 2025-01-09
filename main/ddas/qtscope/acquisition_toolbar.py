from PyQt5.QtWidgets import (
    QToolBar, QPushButton, QSpinBox, QCheckBox, QHBoxLayout,
    QGroupBox, QComboBox, QLabel
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
    binning : QComboBox
        Selection box for histogram binning. N ADC units/bin where N is the 
        value displayed in this box.
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

        self.binning = QComboBox()
        self.binning.insertItems(0, ["1", "2", "4", "8", "16"])
        label = QLabel("ADC units/bin")
        
        self.run_type = QComboBox()
        self.run_type.insertItem(RunType.HISTOGRAM.value, "Energy hist.")
        self.run_type.insertItem(RunType.BASELINE.value, "Baseline")
        
        run_control_layout.addWidget(self.b_read_data)
        run_control_layout.addWidget(self.b_run_control)
        run_control_layout.addWidget(self.run_type)
        run_control_layout.addWidget(self.binning)
        run_control_layout.addWidget(label)
        
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
        self.current_mod.setRange(0, 99) # Set when booting system.
        self.current_chan = QSpinBox()
        self.current_chan.setPrefix("Chan. ")        
        self.current_chan.setRange(0, 99) # Set when booting system.
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
                if(gc.isWidgetType()):
                    gc.setEnabled(False)
                    gc.repaint()

    def enable(self):
        """Enable widgets for system idle state."""        
        for c in self.children():
            for gc in c.children():
                if(gc.isWidgetType()):
                    gc.setEnabled(True)
                    gc.repaint()
        self.b_read_data.setEnabled(False)
            
    def enable_run_active(self):
        """Enable widgets for system running state."""        
        self.enable()
        self.b_read_data.setEnabled(True)
        self.b_read_trace.setEnabled(False)
        self.b_analyze_trace.setEnabled(False)
        self.fast_acq.setEnabled(False)
        self.run_type.setEnabled(False)
        self.current_mod.setEnabled(False)

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
