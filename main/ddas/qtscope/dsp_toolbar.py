from PyQt5.QtWidgets import QToolBar, QPushButton, QSpinBox, QWidget, QSizePolicy

import colors


class DSPToolBar(QToolBar):
    """Toolbar for configuring module DSP.

    Attributes
    ----------
    b_apply : QPushButton
        Button to apply parameters.
    b_load : QPushButton
        Button to load parameters.
    b_copy_mod : QPushButton
        Button to copy module DSP.
    copy_mod : QSpinBox
        Module selection spinbox.
    b_copy_chan : QPushButton
        Button to copy channel DSP.
    copy_chan : QSpinBox
        Channel selection spinbox.
    b_cancel : QPushButton
        Button to close the window.
    copy_mod_action : QAction
        Command interface for copy module button.
    copy_mod_sb_action : QAction
        Command interface for module spinbox.
    copy_chan_action : QAction
        Command interface for copy channel button.
    copy_chan_sb_action : QAction
        Command interface for channel spinbox.

    Methods
    -------
    disable()
        Disable all toolbar widgets.
    enable()
        Enable all toolbar widgets.
    enable_mod_dsp()
        Enable widgets for module DSP.
    set_visible()
        Set which widgets are visible.
    set_module_spinbox_range()
        Set the range of the module spinbox.
    set_channel_spinbox_range()
        Set the range of the channel spinbox for the currently selected module.
    """

    def __init__(self, *args, **kwargs):
        """DSPToolBar class constructor."""
        super().__init__(*args, **kwargs)

        self.setMovable(False)

        # Widget definitions:

        self.b_apply = QPushButton("Apply")
        self.b_load = QPushButton("Load")
        self.b_copy_mod = QPushButton("Copy mod.")
        self.copy_mod = QSpinBox()  # Range set on boot.
        self.b_copy_chan = QPushButton("Copy chan.")
        self.copy_chan = QSpinBox()  # Range set on boot.
        self.b_cancel = QPushButton("Cancel")

        self.b_apply.setStyleSheet(colors.CYAN)
        self.b_load.setStyleSheet(colors.CYAN)
        self.b_copy_mod.setStyleSheet(colors.BLUE)
        self.b_copy_chan.setStyleSheet(colors.BLUE)
        self.b_cancel.setStyleSheet(colors.RED)

        # Expanding blank space:

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        # Add widgets to the toolbar. Actions for the copy module and copy
        # channel widgets are used to change their visibility in the toolbar.
        # See https://doc.qt.io/qt-5/qtoolbar.html#addWidget:

        self.addWidget(self.b_apply)
        self.addWidget(self.b_load)
        self.copy_mod_action = self.addWidget(self.b_copy_mod)
        self.copy_mod_sb_action = self.addWidget(self.copy_mod)
        self.copy_chan_action = self.addWidget(self.b_copy_chan)
        self.copy_chan_sb_action = self.addWidget(self.copy_chan)
        self.addWidget(spacer)
        self.addWidget(self.b_cancel)

    def disable(self):
        """Disable every child widget in the toolbar."""
        for c in self.children():
            if c.isWidgetType():
                c.setEnabled(False)

    def enable(self):
        """Enable every child widget in the toolbar."""
        for c in self.children():
            if c.isWidgetType():
                c.setEnabled(True)

    def enable_mod_dsp(self):
        """Enable module-DSP-specific actions."""
        self.disable()
        self.copy_mod_action.setVisible(False)
        self.copy_mod_sb_action.setVisible(False)
        self.copy_chan_action.setVisible(False)
        self.copy_chan_sb_action.setVisible(False)
        self.b_apply.setEnabled(True)
        self.b_load.setEnabled(True)
        self.b_cancel.setEnabled(True)

    def set_visible(self, name):
        """Set widget visibility depending on the tab.

        Parameters
        ----------
        name : str
            Name of the DSP tab.
        """
        if name == "MultCoincidence":
            self.copy_chan_action.setVisible(False)
            self.copy_chan_sb_action.setVisible(False)
        else:
            self.copy_chan_action.setVisible(True)
            self.copy_chan_sb_action.setVisible(True)

    def set_module_spinbox_range(self, nmodules):
        """Set the range of the module spinbox.

        Parameters
        ----------
        nmodules : int
        Number of modules in the system.
        """
        self.copy_mod.setRange(0, nmodules - 1)

    def set_channel_spinbox_range(self, nchannels):
        """Set the range of the channel spinbox for the currently selected
        module. The system may be a mix of 16- and 32-channel boards.

        Parameters
        ----------
        nchannels : int
            Number of channels on the module.
        """
        self.copy_chan.setRange(0, nchannels - 1)


class DSPToolBarBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """DSPToolbarBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """Create an instance of the toolbar and return it to the caller.

        Returns
        -------
        DSPToolBar
            Instance of the toolbar class.
        """
        return DSPToolBar(*args, **kwargs)
