"""pixie_error.py

Provides a dependency-free module containing only the PixieError exception class.
For now just a re-named subclass of built-in RuntimeError, but extensible if needed.

Classes
-------
PixieError : RuntimeError
    QtScope error reporting class for anything handled across the ctypes interface.
"""


class PixieError(RuntimeError):
    """Raised for FRIBDAQ/hardware/API failures. Already logged at origin."""
