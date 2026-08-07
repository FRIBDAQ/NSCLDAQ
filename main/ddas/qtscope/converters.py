"""converters.py

Utilities for converting between data types and bitarray utilities
needed by QtScope which may not be present in the bitarray module
provided in the OS repos.

Methods
-------
str2char(pystr)
    Convert a Python string to C-type char*.
zeros(length, endian="big")
    Create a bitarray of given length and endianness filled with all zeroes.
ba2int(a, signed=False)
    Convert a bitarray to an integer.
int2ba(i, length=None, endian="big", signed=False)
    Convert an integer to a bitarray.
"""

from ctypes import create_string_buffer
import sys

from bitarray import bitarray, bits2bytes

_is_py2 = bool(sys.version_info[0] == 2)


def str2char(pystr):
    """Convert Python string to char*.

    Parameters
    ----------
    pystr : str
        Python string.

    Returns
    -------
    char*
        Mutable memory string buffer generated from input Python string.
    """
    bstr = pystr.encode("utf-8")
    return create_string_buffer(bstr)


##
# bitarray utils
#

# We have to replicate some utilities taken from 1.6.3 (Debian 11 Bullseye
# repo version) in order for this to run on e.g. Buster which has a bitarray
# version prior to the development of bitarray.utils.


def zeros(length, endian="big"):
    """Create a bitarray of zeros.

    Create a bitarray of length, with all values 0, and optional
    endianness, which may be 'big', 'little'.

    Older bitarray module versions which require us to define this function
    do not necessarily support a gettable/settable  default endianness, so
    here we assume a default value of "big" (as is done in 1.6.3) and instead
    pass it as a default parameter value to the function.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied from tag 1.6.3, which is the version of the bitarray module
    available in the Debian 11 repo.

    Parameters
    ----------
    length : int
        Length of the bitarray.
    endian : str, optional, default='big'
        bitarray endianness.

    Returns
    -------
    bitarray
        bitarray of zeroes of the given length and endianness.

    Raises
    ------
    TypeError
        If the length is not an integer.
    """
    if not isinstance(length, (int, long) if _is_py2 else int):
        raise TypeError("Integer expected")

    a = bitarray(length, endian)
    a.setall(0)
    return a


def ba2int(a, signed=False):
    """Convert the given bitarray to an integer.

    The bit-endianness of the bitarray is respected. `signed` indicates
    whether two's complement is used to represent the integer. Added for
    compatibility with older versions of bitarray found in the Debian repos
    that do not install with the utilities module.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied from tag 1.6.3, which is the version of the bitarray module
    available in the Debian 11 repo.

    Parameters
    ----------
    a : bitarray
        bitarray to convert.
    signed : bool, default=False
        Two's complement.

    Returns
    -------
    int
        Integer representation of the bitarray.

    Raises
    ------
    TypeError
        If the argument to convert is not a bitarray.
    ValueError
        If the bitarray is empty (length 0).
    """
    if not isinstance(a, bitarray):
        raise TypeError("bitarray expected")
    length = len(a)
    if length == 0:
        raise ValueError("non-empty bitarray expected")

    big_endian = bool(a.endian() == "big")
    # for big endian pad leading zeros - for little endian we don't need to
    # pad trailing zeros, as .tobytes() will treat them as zero
    if big_endian and length % 8:
        a = zeros(8 - length % 8, "big") + a
    b = a.tobytes()

    if _is_py2:
        c = bytearray(b)
        res = 0
        j = len(c) - 1 if big_endian else 0
        for x in c:
            res |= x << 8 * j
            j += -1 if big_endian else 1
    else:  # py3
        res = int.from_bytes(b, byteorder=a.endian())

    if signed and res >= 1 << (length - 1):
        res -= 1 << length
    return res


def int2ba(i, length=None, endian="big", signed=False):
    """Convert the given integer to a bitarray.

    Convert integer to bitarray with given endianness, and no leading
    (big-endian) / trailing (little-endian) zeros, unless the `length`
    of the bitarray is provided.  An `OverflowError` is raised if the
    integer is not representable with the given number of bits. `signed`
    determines  whether two's complement is used to represent the integer,
    and requires `length` to be provided. If signed is False and a negative
    integer is given, an OverflowError is raised.

    Older bitarray module versions which require us to define this function
    do not necessarily support a gettable/settable  default endianness, so
    here we assume a default value of "big" (as is done in 1.6.3) and instead
    pass it as a default parameter value to the function.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied largely from tag 1.6.3, which is the version of the bitarray module
    available in the Debian 11 repo.

    Parameters
    ----------
    i : int
        Integer to convert to bitarray.
    length : int, default=None
        Length of the bitarray.
    endian : str, {'big', 'little'}
        Endianness of the bitarray.
    signed : bool, default=False
        Two's complement signed.

    Returns
    -------
    bitarray
        bitarray representation of the integer.

    Raises
    ------
    TypeError
        If the argument to convert is not an integer.
        If no output bitarray length is provided.
        If output is a signed integer and length is zero.
    ValueError
        The output bitarray length is zero.
    OverflowError
        If the integer value is too large to be represented by the number of
        provided bits.
    """
    if not isinstance(i, (int, long) if _is_py2 else int):
        raise TypeError("integer expected")
    if length is not None:
        if not isinstance(length, int):
            raise TypeError("integer expected for length")
        if length <= 0:
            raise ValueError("integer larger than 0 expected for length")
    if signed and length is None:
        raise TypeError("signed requires length")

    if i == 0:
        # there are special cases for 0 which we'd rather not deal with below
        za = zeros(length or 1, endian)
        return za

    if signed:
        if i >= 1 << (length - 1) or i < -(1 << (length - 1)):
            raise OverflowError("signed integer out of range")
        if i < 0:
            i += 1 << length
    elif i < 0 or (length and i >= 1 << length):
        raise OverflowError("unsigned integer out of range")

    a = bitarray(0, endian)
    big_endian = bool(a.endian() == "big")
    if _is_py2:
        c = bytearray()
        while i:
            i, r = divmod(i, 256)
            c.append(r)
        if big_endian:
            c.reverse()
        b = bytes(c)
    else:  # py3
        b = i.to_bytes(bits2bytes(i.bit_length()), byteorder=a.endian())

    a.frombytes(b)
    if length is None:
        return _strip(a, "left" if big_endian else "right")

    la = len(a)
    if la > length:
        a = a[-length:] if big_endian else a[:length]
    if la < length:
        pad = zeros(length - la, endian)
        a = pad + a if big_endian else a + pad
    assert len(a) == length
    return a


##
# Private methods
#


def _strip(a, mode="right"):
    """Strip zeros from left, right or both ends.

    Allowed values for mode are the strings: `left`, `right`, `both`.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied from tag 1.6.3, which is the version of the bitarray module
    available in the Debian 11 repo.

    Parameters
    ----------
    a : bitarray
        A bitarray object.
    mode : str, optional, default='right'
        End of the bitarray to strip zeroes from.

    Returns
    -------
    bitarray
        bitarray with zeroes stripped of the end(s).

    Raises
    ------
    TypeError
        If the argument to strip is not a bitarray.
        If mode argument is not a string.
    ValueError
        If the mode string is not "left," "right," or "both."
    """
    if not isinstance(a, bitarray):
        raise TypeError("bitarray expected")
    if not isinstance(mode, str):
        raise TypeError("String expected for mode")
    if mode not in ("left", "right", "both"):
        raise ValueError("Allowed values 'left', 'right', 'both', got: %r" % mode)

    first = 0
    if mode in ("left", "both"):
        try:
            first = a.index(1)
        except ValueError:
            return bitarray(0, a.endian())

    last = len(a) - 1
    if mode in ("right", "both"):
        try:
            last = _rindex(a)
        except ValueError:
            return bitarray(0, a.endian())

    return a[first : last + 1]


def _rindex(a, value=True):
    """Return the rightmost index of `value` in a bitarray.

    Replicates bitarray.util.rindex for hosts whose bitarray version
    predates the util module. Upstream implements this in C; this is a
    pure Python equivalent which scans the bitarray in reverse and returns
    on the first match.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Required by strip(), which relies on the ValueError to detect a
    bitarray containing no set bits.

    Parameters
    ----------
    a : bitarray
        A bitarray object.
    value : bool, optional, default=True
        Bit value to search for.

    Returns
    -------
    int
        Index of the rightmost occurrence of `value` in the bitarray.

    Raises
    ------
    TypeError
        If the argument to rindex is not a bitarray.
    ValueError
        If `value` does not occur in the bitarray.
    """
    if not isinstance(a, bitarray):
        raise TypeError("bitarray expected")

    # Backwards scan: range(start, stop, step)
    for i in range(len(a) - 1, -1, -1):
        if a[i] == value:
            return i

    raise ValueError("rindex(x): x not in bitarray")
