import logging
import sys
import traceback

from PyQt5.QtCore import QObject, QThreadPool, QRunnable, pyqtSignal, pyqtSlot

from pixie_utilities import PixieError

_logger = logging.getLogger("qtscope_logger")


class WorkerSignals(QObject):
    """Defines the signals available from a running worker thread. Encapsulates
    custom signals in a class derived from QObject which can be incorporated
    into classes derived from QRunnable for signals/slots in threads.

    Supported signals are:
        running: No data.
        finished: No data.
        error: Tuple (exctype, value, traceback.format_exc()).
        result: Object data returned from processing, anything.
    """

    running = pyqtSignal()
    finished = pyqtSignal()
    error = pyqtSignal(tuple)
    result = pyqtSignal(object)


class Worker(QRunnable):
    """Worker thread.

    Inherits from QRunnable to handle setup, signals and wrap-up.

    Attributes
    ----------
    fn : function
        Function attached to this worker thread.
    args : tuple
        Arguments to pass to the callback function.
    kwargs : dict
        Keyword arguments to pass to the callback function.
    signals : WorkerSignals
        Signals emitted by the worker.
    logger : Logger
        QtScope Logger instance.

    Methods
    -------
    run()
        Executes the code we wish to run from the passed function fn.
    set_function(function)
        Set function attribute.
    """

    def __init__(self, fn=None, *args, **kwargs):
        """
        Worker thread class constructor.

        Constructs a worker thread and callback function from a function
        object and callback args/kwargs.

        Parameters
        ----------
        fn : QObject
            Function attached to this worker thread.
        args : tuple
            Arguments to pass to the callback function.
        kwargs : dict
            Keyword arguments to pass to the callback function.
        """
        super().__init__()

        # Store constructor arguments (re-used for processing):

        self.fn = fn
        self.args = args
        self.kwargs = kwargs
        self.signals = WorkerSignals()

    @pyqtSlot()
    def run(self):
        """Initialise the runner function with passed args, kwargs."""
        try:
            self.signals.running.emit()
            result = self.fn(*self.args, **self.kwargs)
        except:
            exctype, value = sys.exc_info()[:2]
            self.signals.error.emit((exctype, value, traceback.format_exc()))
        else:
            self.signals.result.emit(result)
        finally:
            self.signals.finished.emit()

    def set_function(self, new_fn):
        """Set the function for this worker thread.

        Parameters
        ----------
        new_fn : function
            Function to set.
        """
        self.fn = new_fn


class ThreadPoolManager:
    """Thread management for QtScope.

    QThreadPool is used to recycle thread objects and reduce creation overhead.
    The blocking feature of Python multithreading is useful here to
    simultaneously block inputs from sections of the GUI which we want to
    disable as well as disable the GUI elements which would respond to those
    inputs. The concept of the "main event loop" in Qt and Python's blocking
    thread behavior imply both of these steps are necessary: events can still
    be queued in the main event loop if the GUI elements connected to those
    signaling events are not disabled.

    The main way to interact with this class is through the `start_thread()`
    method, specifying the function to be executed within the thread and the
    functions connected to the thread's `running` and `finished` signals.
    The `start_thread()` method will create and configure and run an object
    encapsulating a worker thread in a QRunnable. Note that QThreadPool takes
    ownership of the runnable and will delete it if it returns true; otherwise
    ownership remains with the caller. Error handling and message logging is
    the caller/runnable's responsibility.

    Attributes
    ----------
    pool : QThreadPool
        Global thread pool.

    Methods
    -------
    start_thread(QRunnable, list, list)
        Reserve a thread and run it's callback.
    get_active_thread_count()
        Return the number of active threads.
    wait(int)
        Wait for all threads to exit and remove them from the global pool.
    exit()
        Remove all queued runnables which have not yet started.

    """

    def __init__(self, *args, **kwargs):
        """Initialize the thread pool. Get the global thread pool.

        Parameters
        ----------
        args : tuple
            Arguments (passed uninspected to base class).
        kwargs : dict
            Keyword arguments (passed uninspected to base class).
        """
        super().__init__(*args, **kwargs)
        self.pool = QThreadPool.globalInstance()

    def start_thread(
        self,
        fcn=None,
        running=None,
        finished=None,
        results=None,
        errors=None,
        *args,
        **kwargs,
    ):
        """Configure and run a thread given the passed parameters.

        Though you can in principle pass parameters to `fcn` using args and
        kwargs, this feature is not used within QtScope and is untested.
        In any event, all signals are expected to take no arguments. If no
        error-handler is provided, a default handler is used. If they require
        arguments this can be circumvented via some via a locally-defined
        lambda function. The same can be done with `fcn`, for the time being.

        Parameters
        ----------
        fcn : function
            Function to run in the thread.
        running : list of QObjects
            List of functions called when fcn runs.
        finished : list of QObjects
            List of functions called when fcn finishes.
        results : list of QObjects
            List of functions called when fcn produces results.
        errors : list of QObjects
            List of functions called when fcn produces errors.
        args : tuple
            Arguments to pass to the callback function.
        kwargs : dict
            Keyword arguments to pass to the callback function.
        """
        worker = Worker(fcn, *args, **kwargs)
        for f in running or []:
            worker.signals.running.connect(f)
        for f in finished or []:
            worker.signals.finished.connect(f)
        for f in results or []:
            worker.signals.result.connect(f)
        for f in errors or [self._default_error_handler]:
            worker.signals.error.connect(f)
        self.pool.start(worker)

    def get_active_thread_count(self):
        """Return the number of threads from
        QThreadPool.getActiveThreadCount().

        Returns
        -------
        int
            Number of active threads from QThreadPool.activeThreadCount().
        """
        return self.pool.activeThreadCount()

    def wait(self, time=10000):
        """Wrapper for QThreadPool `waitForDone()`. Waits for all threads to
        exit and removes them from the pool.

        Parameters
        ----------
        time : int
            Wait time in milliseconds (optional, default=10 s).
        """
        self.pool.waitForDone(time)

    def exit(self):
        """Clears all runnables from the queue and waits for all threads
        to exit.
        """
        self.pool.clear()
        self.wait()

    ##
    # Private functions
    #

    def _default_error_handler(self, err):
        """Default error handler for worker threads. Logs the error to the QtScope logger."""
        exctype, value, tb = err
        _logger = logging.getLogger("qtscope_logger")
        if isinstance(value, PixieError):
            _logger.error(f"Unhandled worker error: {value}")
        else:
            _logger.error(f"Unhandled worker error:\n{tb}")
