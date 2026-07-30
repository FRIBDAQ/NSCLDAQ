import numpy as np

from fit_function import FitFunction


class GaussP2Fit(FitFunction):
    """Gaussian-plus-quadratic-background fitting function class used by QtScope.

    Fits an un-normalized Gaussian plus a quadratic (second-order polynomial)
    background. Implements function-specific model and set_initial_parameters
    methods from the base class. See the documentation for the FitFunction
    base class in fit_function.py for details.

    """

    def model(self, x, params):
        """Evaluate the fit function over x.

        Implement an un-normalized Gaussian plus a quadratic background:
        params[0:3] are the Gaussian amplitude, mean, and stddev; params[3:6]
        are the quadratic background constant, linear, and quadratic terms.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        params : array-like
            Parameters used to evaluate the function over x.

        Returns
        ------
        ndarray
            Array containing the fit values over the range.
        """

        def gauss(x, params):
            return params[0] * np.exp(-((x - params[1]) ** 2) / (2 * params[2] ** 2))

        def pol2(x, params):
            return params[0] + params[1] * x + params[2] * x**2

        return gauss(x, params[0:3]) + pol2(x, params[3:])

    def set_initial_parameters(self, x, y, params):
        """Set initial parameter values.

        Guess at the Gaussian amplitude, mean, and stddev, and at the constant
        and linear terms of the quadratic background, using the defined fit
        range if no parameters are provided on the fit panel. The quadratic
        term is always taken from the fit panel.

        Parameters
        ----------
        x : list
            x data values.
        y : list
            y data values.
        params : list
            Array of fit parameters.
        """
        super().set_initial_parameters(x, y, params)
        if self.p_init[0] == 0.0:
            self.p_init[0] = max(y)
        if self.p_init[1] == 0.0:
            self.p_init[1] = np.mean(x)
        if self.p_init[2] == 0.0:
            self.p_init[2] = np.std(x)
        if self.p_init[3] == 0.0:  # Constant of quadratic background
            self.p_init[3] = min(y[0], y[-1])
        if self.p_init[4] == 0.0:  # Linear term of quadratic background
            self.p_init[4] = (y[-1] - y[0]) / (x[-1] - x[0])
        # p[5] quadratic term always from fit panel.


class GaussP2FitBuilder:
    """Builder method for factory creation."""

    def __init__(self):
        """GaussP2FitBuilder class constructor."""
        self._instance = None

    def __call__(self, params=[], form="", count_data=True):
        """Create the fitting function.

        Create an instance of the fit function if it does not exist and
        return it to the caller. Parameters passed as unpacked **kwargs
        from the fit factory.

        Parameters
        ----------
        params : array-like
            Array of initial parameters. In general not used, but at least
            ensures the class is initialized with valid and/or reasonable
            starting guesses.
        form : str
            Function formula.
        count_data : bool, default=True
            True if the data represent counts (Poisson statistics), False for
            unweighted least-squares. Passed through to the fit function.

        Returns
        -------
        GaussP2Fit
            Instance of the fit class.
        """
        if not self._instance:
            self._instance = GaussP2Fit(params, form, count_data)
        return self._instance
