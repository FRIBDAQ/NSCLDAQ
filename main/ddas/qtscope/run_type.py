from enum import Enum, unique


@unique
class RunType(Enum):
    """Enum of possible run types plus an inactive run type.

    @unique decoration ensures that each name is aliased to a unique value.

    Note
    ----
    Run types need values [0, ..., N-1] for N run types because the combo box
    is zero-indexed. As long as the numbers are contiguous, order (should not)
    matter other than whatever is index 0 is set by default.
    """

    HISTOGRAM = 0
    BASELINE = 1
    INACTIVE = 2
