from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT

from PyQt5.QtWidgets import QPushButton, QCheckBox

import colors

class PlotToolBar(NavigationToolbar2QT):
    """
    ToolBar for matplotlib plotting widget. 

    Defualt feautres support basic plot manipulation (zoom region, pan, etc.). 
    Extra plotting featrues are enabled using additional widgets. Unwanted 
    default plot configuration options can be removed via their associated 
    action.

    Attributes:
        logscale (QCheckBox): Enable/disable logscale plotting.
        b_fit_panel (QPushButton): Button to display the fitting panel GUI.

    Methods:
        disable(): Disable all toolbar widgets.
        enable(): Enable toolbar widgets for system idle.
    """
    
    def __init__(self, *args, **kwargs):
        """PlotToolBar class constructor."""
        
        super().__init__(*args, **kwargs)
        
        self.logscale = QCheckBox("Log y-axis", self)
        self.b_fit_panel = QPushButton("Fit panel", self)
        
        self.b_fit_panel.setStyleSheet(colors.YELLOW)

        self.addWidget(self.logscale)
        self.addWidget(self.b_fit_panel)

        # Removing buttons from toolbar:
        
        unwanted_buttons = ["Back", "Forward"]
        for action in self.actions():
            if action.text() in unwanted_buttons:
                self.removeAction(action)

    def disable(self):
        """Disable child widgets in the plot toolbar."""

        self.logscale.setEnabled(False)
        self.b_fit_panel.setEnabled(False)
        
    def enable(self):
        """Enable child widgets in the plot toolbar."""

        self.logscale.setEnabled(True)
        self.b_fit_panel.setEnabled(True)


class PlotToolBarBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """PlotToolbarBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the toolbar and return it to the caller.

        Returns:
            PlotToolBar: Instance of the toolbar class.
        """
        
        return PlotToolBar(*args, **kwargs)
