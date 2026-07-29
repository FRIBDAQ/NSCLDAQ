from PyQt5.QtWidgets import QTabWidget, QGridLayout
from PyQt5.QtGui import QValidator


class MyTabWidget(QTabWidget):
    """Extended QTabWidget with convenient index access."""

    def __getitem__(self, index):
        return self.widget(index)

    def __len__(self):
        return self.count()


class MyGridLayout(QGridLayout):
    """Extended QGridLayout with position-based widget access.

    Methods
    -------
    widget_at(row, col)
        Get the widget at the specified row, col of the grid layout.
    """

    class _GridRow:
        """Helper class to enable layout[row][col] indexing."""

        def __init__(self, layout, row):
            self.layout = layout
            self.row = row

        def __getitem__(self, col):
            """Get widget at (row, col)."""
            return self.layout.widget_at(self.row, col)

    def widget_at(self, row, col):
        """Get widget at (row, col) or None if empty. It is the responsibility
        of the caller to ensure that the row and column index are sensible.

        Parameters
        ----------
        row : int
            Row index.
        col : int
            Column index.

        Returns
        -------
        The widget located at row, col or None if no widget exists.
        """
        item = self.itemAtPosition(row, col)
        return item.widget() if item else None

    def __getitem__(self, key):
        """Enable both layout[row][col] and layout[row, col] syntax."""
        if isinstance(key, tuple):
            # Handle layout[row, col]
            if len(key) == 2:
                row, col = key
                return self.widget_at(row, col)
            raise IndexError("Grid index must be (row, col)")
        else:
            # Handle layout[row][col] by returning a row proxy
            return self._GridRow(layout=self, row=key)


class UInt32Validator(QValidator):
    """Validator for unsigned 32-bit integer input.

    Accepts integers in the range 0 to 4294967295 (0xFFFFFFFF).

    Methods
    -------
    validate(input_str, pos)
        Validate input string as uint32.
    """

    def validate(self, input_str, pos):
        """Validate input string as uint32.

        Parameters
        ----------
        input_str : str
            Input string to validate.
        pos : int
            Current cursor position.

        Returns
        -------
        Tuple of (QValidator::State, input string, position)
        """
        if input_str == "":
            return (QValidator.Intermediate, input_str, pos)
        try:
            value = int(input_str)
            if 0 <= value <= 0xFFFFFFFF:
                return (QValidator.Acceptable, input_str, pos)
            else:
                return (QValidator.Invalid, input_str, pos)
        except ValueError:
            return (QValidator.Invalid, input_str, pos)


def callMe(self):
    """A dummy function which can be hooked up to signals for testing."""
    print(
        f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Hey I just wrote you, and this is crazy\nBut here's my purpose, you should call me, maybe"
    )
