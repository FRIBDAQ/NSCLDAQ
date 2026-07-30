import logging

import numpy as np
from scipy.optimize import minimize

np.seterr(all="ignore")


class FitFunction:
    """Base class for fitting functions used by QtScope.

    Attributes
    ----------
    p_init : list
        Initial parameter value guesses.
    form : str
        Function formula string.
    count_data : bool
        True if data represent counts (optional, default=True).
    logger : Logger
        QtScope Logger instance.

    Methods
    -------
    model(x, *p)
        Returns an array of function values evaluated over the range x.
    set_initial_parameters(x, y, params)
        Sets initial parameter values. Derived classes can override this
        function to e.g., make educated guesses at parameters based on data.
    neg_log_likelihood_p(params, x, y)
        Poisson negative log likelihood. Used if count_data == True.
    neg_log_likelihood_g(params, x, y)
        Gaussian negative log likelihood. Used if count_data == False.
    start(x, y, params, axis)
        Implementation of the fitting algorithm.
    get_parameter_errors(result, x)
        Get the parameter errors for the fit.

    """

    def __init__(self, params, form, count_data=True):
        """Class constructor.

        Set initial fit parameters, functional form, etc.

        Parameters
        ----------
        params : list
            Initial parameters to set.
        form : str
            Function formula to display on the fitting panel.
        count_data : bool
            True if the data represents counts.
        """
        self.p_init = params  # Initial guesses
        self.form = form
        self.count_data = count_data
        self.logger = logging.getLogger("qtscope_logger")

    def model(self, x, params):
        """The fit function over a range of x values.

        This function is not implemented in the base class and must be
        implemented by each fitting function.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        params : ndarray
            Parameters used to evaluate the function over x.

        Raises
        ------
        NotImplementedError
            If model from the base class is called. This method must be
            implemented in derived classes.
        """
        raise NotImplementedError()

    def set_initial_parameters(self, x, y, params):
        """Set initial parameter values.

        Default behavior is to set initial parameter guesses using the values
        provided on the fit panel.

        Parameters
        ----------
        x : ndarray
            x data values.
        y : ndarray
            y data values.
        params : array-like
            Array of fit parameters.
        """
        self.p_init = params[0 : len(self.p_init)]

    def neg_log_likelihood_p(self, params, x, y):
        """Poisson negative log-likelihood.

        Fit parameters must be first argument and are initially set by x0 in
        the `minimize` call. x, y are passed as additional args.

        Parameters
        ----------
        params : array-like
            Array of fit parameters.
        x : ndarray
            x data values.
        y : ndarray
            y data values.
        """
        # Make sure the model is implemented:
        try:
            pred = self.model(x, params)
        except NotImplementedError:
            print("ERROR: Model function is not defined!")
            return np.inf
        else:
            # Replace small values with some small number to ensure log(pred)
            # is valid. I chose to modify the likelihood this way over
            # returning, e.g, np.inf if any pred <= 0 because it prevents huge
            # jumps in the objective function value when the parameters are
            # close to values which give pred <= 0. Also the return value is
            # always defined.
            pred = np.maximum(pred, 1e-10)
            return -np.sum(y * np.log(pred) - pred)

    def neg_log_likelihood_g(self, params, x, y):
        """Gaussian negative log-likelihood.

        Fit parameters must be first argument and are initially set by x0 in
        the `minimize` call. x, y are passed as additional args.

        Parameters
        ----------
        params : array-like
            Array of fit parameters.
        x : ndarray
            x data values.
        y : ndarray
            y data values.
        """
        # Make sure the model is implemented:
        try:
            pred = self.model(x, params)
        except NotImplementedError:
            print("ERROR: Model function is not defined!")
            return np.inf
        else:
            # For Gaussian errors, the MLE reduces to OLS:
            res = y - pred
            return np.sum(np.square(res))

    def start(self, x, y, params, axis):
        """Implementation of the fitting algorithm.

        Parameters
        ----------
        x : list
            x data values.
        y : list
            y data values.
        params : list
            Array of fit parameters.
        axis : matplotlib axes
            Axes for the plot.

        Returns
        -------
        result : OptimizeResult
            Contains fit results and other info. See https://docs.scipy.org/doc/scipy/reference/generated/scipy.optimize.OptimizeResult.html.
        """
        y = np.float64(y)  # Make sure y data is floating point
        self.set_initial_parameters(x, y, params)
        # If the data represents counts, use Poisson MLE, otherwise Gaussian:
        if self.count_data:
            result = minimize(
                self.neg_log_likelihood_p,
                x0=self.p_init,
                args=(x, y),
                method="bfgs",
                jac="3-point",
            )
        else:
            result = minimize(
                self.neg_log_likelihood_g,
                x0=self.p_init,
                args=(x, y),
                method="bfgs",
                jac="3-point",
            )
        ## Most often an issue with final precision on error estimates:
        # if not result.success:
        #    print(f"WARNING: fit did not terminate successfully:\n{result}")

        return result

    def get_parameter_errors(self, result, x):
        """Get the parabolic parameter uncertainties from the optimizer.

        The BFGS inverse Hessian approximates the parameter covariance when
        the objective is a negative log-likelihood, which is the case for
        count data. The unweighted least-squares objective is 2*sigma^2
        times a Gaussian NLL, so its covariance is 2*sigma^2 times the
        inverse Hessian, with sigma^2 estimated from the fit residuals.

        Parameters
        ----------
        result : OptimizeResult
            Result returned by start().
        x : ndarray
            x data values the fit was performed over.

        Returns
        -------
        ndarray
            Standard error for each fit parameter.
        """
        scale = 1.0
        if not self.count_data:
            ndf = max(len(x) - len(result.x), 1)
            # Note for Gaussian fits result.fun is RSS:
            scale = 2.0 * result.fun / ndf

        return np.sqrt(scale * np.diag(result.hess_inv))
