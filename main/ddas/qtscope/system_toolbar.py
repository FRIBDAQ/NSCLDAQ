from PyQt5.QtWidgets import QToolBar, QPushButton, QMessageBox, QWidget, QSizePolicy

import colors


class SystemToolBar(QToolBar):
    """
    System-level function toolbar (QToolBar).

    Attributes
    ----------
    b_boot : QPushButton
        Button for system boot.
    b_chan_gui : QPushButton
        Button to open channel DSP GUI.
    b_mod_gui : QPushButton
        Button to open module DSP GUI.
    b_load : QPushButton
        Button to load a settings file.
    b_save : QPushButton
        Button to save a settings file.
    b_about : QPushButton
        Button to display program information and Qt acknowledgments
    b_exit : QPushButton
        Button to exit the application.

    Methods
    -------
    disable()
        Disable all toolbar widgets.
    enable()
        Enable all toolbar widgets.
    enable_booted()
        Enable toolbar widgets following a successful system boot.
    _about()
        Display program information and Qt acknowledgments.
    """

    def __init__(self, *args, **kwargs):
        """SystemToolBar class constructor."""
        super().__init__(*args, **kwargs)

        self.setMovable(False)

        # Widget definitions:

        self.b_boot = QPushButton("Boot system")
        self.b_chan_gui = QPushButton("Channel DSP")
        self.b_mod_gui = QPushButton("Module DSP")
        self.b_load = QPushButton("Load settings")
        self.b_save = QPushButton("Save settings")
        self.b_about = QPushButton("About")
        self.b_exit = QPushButton("Exit")

        self.b_boot.setStyleSheet(colors.RED_TEXT)
        self.b_chan_gui.setStyleSheet(colors.YELLOW)
        self.b_mod_gui.setStyleSheet(colors.YELLOW)
        self.b_load.setStyleSheet(colors.ORANGE)
        self.b_save.setStyleSheet(colors.ORANGE)
        self.b_about.setStyleSheet(colors.YELLOW)
        self.b_exit.setStyleSheet(colors.RED)

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        # Add widgets to the toolbar:

        self.addWidget(self.b_boot)
        self.addWidget(self.b_chan_gui)
        self.addWidget(self.b_mod_gui)
        self.addWidget(self.b_load)
        self.addWidget(self.b_save)
        self.addWidget(spacer)
        self.addWidget(self.b_about)
        self.addWidget(self.b_exit)

        # Set initial states:

        self.disable()
        self.b_boot.setEnabled(True)

        # The about button pops up a message box:

        self.b_about.clicked.connect(self._about)

    def disable(self):
        """Disable every child widget in the toolbar."""
        for c in self.children():
            if c.isWidgetType():
                c.setEnabled(False)

        # About and exit buttons are always enabled:
        self.b_about.setEnabled(True)
        self.b_exit.setEnabled(True)

    def enable(self):
        """Enable every child widget in the toolbar."""
        for c in self.children():
            if c.isWidgetType():
                c.setEnabled(True)

    def enable_booted(self):
        """Enable widgets for the booted system state. The boot button is colored green and disabled."""
        self.b_boot.setText("Booted")
        self.b_boot.setStyleSheet(colors.GREEN)
        self.b_boot.setEnabled(False)
        self.b_chan_gui.setEnabled(True)
        self.b_mod_gui.setEnabled(True)
        self.b_load.setEnabled(True)
        self.b_save.setEnabled(True)
        self.b_about.setEnabled(True)
        self.b_exit.setEnabled(True)

    def _about(self):
        """Popup a QMessageBox containing the relevant info."""
        msg = """QtScope is the slow control program for NSCL DDAS which is used to configure DSP settings on XIA Pixie modules.\n\nVersion: 1.0\n\nQtScope makes use of PyQt5, which in turn makes use of Qt 5. We use the open-source license of Qt and thus must also provide a means to download the code for Qt as well as the source code of this program.\n\nInstructions for obtaining the Qt source code can be found at: https://wiki.qt.io/Building_Qt_5_from_Git#Getting_the_source_code\n\nThis project is in the main/ddas/qtscope directory of the NSCLDAQ  project: https://github.com/FRIBDAQ/NSCLDAQ\n\nAuthor:\n\tAaron Chester\n\tFacility for Rare Isotope Beams\n\tMichigan State University\n\tEast Lansing, MI 48824"""
        msg_box = QMessageBox()
        msg_box.setWindowTitle("About QtScope")
        msg_box.setText(msg)
        msg_box.setIcon(QMessageBox.Information)
        msg_box.exec()


class SystemToolBarBuilder:
    """Builder method for factory creation."""

    def __init__(self, *args, **kwargs):
        """SystemToolbarBuilder class constructor."""

    def __call__(self, *args, **kwargs):
        """
        Create an instance of the toolbar and return it to the caller.

        Returns
        -------
        SystemToolBar
            Instance of the toolbar class.
        """
        return SystemToolBar(*args, **kwargs)
