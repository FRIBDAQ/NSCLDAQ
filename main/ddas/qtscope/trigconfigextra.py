from PyQt5.QtWidgets import QWidget, QLabel, QLineEdit, QPushButton, QVBoxLayout
from PyQt5.QtGui import QIntValidator

import colors
from extensions import MyGridLayout

class TrigConfigExtra(QWidget):
    def __init__(self, *args, nmodules=None, channel_map=None, **kwargs):
        super().__init__(*args, **kwargs)

        self.param_names = ["TrigConfig1", "TrigConfig2", "TrigConfig3"]
        self.nmodules = nmodules
        self.channel_map = channel_map
        
        self.b_show_config = QPushButton("Display TrigConfig[1-3]")
        self.b_show_config.setStyleSheet(colors.YELLOW)
        
        layout = QVBoxLayout()
        layout.addWidget(QLabel("TrigConfig[1-3]"))
        layout.addWidget(self.b_show_config)
        self.setLayout(layout)
        
        self.grid = QWidget()
        self.grid.setWindowTitle("TrigConfig[1-3] settings")
        self.param_grid = MyGridLayout(self.grid)
        
        for col, label in enumerate(self.param_names, 1):
            self.param_grid.addWidget(QLabel(label), 0, col)
        
        for i in range(self.nmodules):
            self.param_grid.addWidget(QLabel("Mod. %i" %i), i+1, 0)
            for col, _ in enumerate(self.param_names, 1):
                tcx = QLineEdit()
                self.param_grid.addWidget(tcx, i+1, col)

        self.b_show_config.clicked.connect(self._show_config)
        
    def configure(self, mgr):
        self.display_dsp(mgr)
            
    def update_dsp(self, mgr):
        for i in range(self.nmodules):
            for col, name in enumerate(self.param_names, 1):
                val = float(self.param_grid[i+1, col].text())
                mgr.set_mod_par(i, name, val)
    
    def display_dsp(self, mgr):
        for i in range(self.nmodules):
            for col, name in enumerate(self.param_names, 1):
                val = mgr.get_mod_par(i, name)
                self.param_grid[i+1, col].setText(val)

    ##
    # Private methods
    #
    
    def _show_config(self):
        """Display the TrigConfig[1-3] GUI."""        
        self.grid.show()
        
class TrigConfigExtraBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """TrigConfigExtraBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns
        -------
        TrigConfigExtra
            Instance of the DSP class widget.
        """            
        return TrigConfigExtra(*args, **kwargs)        