from PyQt5.QtGui import QIntValidator, QDoubleValidator
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QComboBox,
    QPushButton,
    QLabel,
    QLineEdit,
    QTextEdit,
    QGridLayout,
    QGroupBox,
)

import colors

## Number of fit parameters
NPARAMS = 6


class FitPanel(QWidget):
    """Fit panel GUI class with a similar interface to QtPy.

    Attributes
    ----------
    function_list : QComboBox
        List of available fitting functions.
    b_fit : QPushButton
        Perform the fit when button is clicked.
    b_clear : QPushButton
        Clear the previous fit results and reset the fit panel.
    b_cancel : QPushButton
        Button to close the FitPanel popup window.
    range_min : QLineEdit
        Fitting range lower limit.
    range_max : QLineEdit
        Fitting range upper limit.
    params : list of QLineEdit objects
        The fitting parameter widgets.
    results : QTextEdit
        Display widget for the fit results.

    Methods
    -------
    reset()
        Reset fit panel display.
    """

    def __init__(self, *args, **kwargs):
        """FitPanel class constructor."""
        super().__init__()

        self.setWindowTitle("QtScope fit panel")

        # Widget definitions:

        self._function_box = QGroupBox("Fit function")
        self.function_list = QComboBox()
        self.function_formula = QLabel("formula")

        self._config_box = QGroupBox("Fit range and parameters")
        self._range_label_min = QLabel("Min x")
        self._range_label_max = QLabel("Max x")
        self.range_min = QLineEdit()
        self.range_max = QLineEdit()
        for field in (self.range_min, self.range_max):
            field.setValidator(QIntValidator(0, 2**31 - 1))

        self.params = [QLineEdit() for _ in range(NPARAMS)]
        for field in self.params:
            field.setValidator(QDoubleValidator())
        param_labels = [QLabel(f"p{i}") for i in range(NPARAMS)]

        self._results_box = QGroupBox("Fit output")
        self.results = QTextEdit()
        self.results.setReadOnly(True)

        self.b_fit = QPushButton("Fit")
        self.b_clear = QPushButton("Clear")
        self.b_cancel = QPushButton("Cancel")

        self.b_fit.setStyleSheet(colors.BLUE)
        self.b_clear.setStyleSheet(colors.BLUE)
        self.b_cancel.setStyleSheet(colors.RED)

        self.reset()

        # Widget layout:

        range_label_layout = QHBoxLayout()
        range_label_layout.addWidget(self._range_label_min)
        range_label_layout.addWidget(self._range_label_max)

        range_value_layout = QHBoxLayout()
        range_value_layout.addWidget(self.range_min)
        range_value_layout.addWidget(self.range_max)

        param_grid_layout = QGridLayout()
        for i, (label, field) in enumerate(zip(param_labels, self.params)):
            row, pair = divmod(i, 2)  # Two per row
            param_grid_layout.addWidget(label, row, pair * 2)
            param_grid_layout.addWidget(field, row, pair * 2 + 1)

        function_layout = QVBoxLayout()
        function_layout.addWidget(self.function_list)
        function_layout.addWidget(self.function_formula)
        self._function_box.setLayout(function_layout)

        config_layout = QVBoxLayout()
        config_layout.addLayout(range_label_layout)
        config_layout.addLayout(range_value_layout)
        config_layout.addLayout(param_grid_layout)
        self._config_box.setLayout(config_layout)

        results_layout = QVBoxLayout()
        results_layout.addWidget(self.results)
        self._results_box.setLayout(results_layout)

        button_layout = QHBoxLayout()
        button_layout.addWidget(self.b_fit)
        button_layout.addWidget(self.b_clear)
        button_layout.addWidget(self.b_cancel)

        layout = QVBoxLayout()
        layout.addWidget(self._function_box)
        layout.addWidget(self._config_box)
        layout.addWidget(self._results_box)
        layout.addLayout(button_layout)

        self.setLayout(layout)

    def reset(self):
        """Reset fit panel display.

        Clear fit ranges, reset parameter values to zero, and clear the
        fit results.
        """
        self.range_min.clear()
        self.range_max.clear()
        for field in self.params:
            field.setText("0")
        self.results.clear()
