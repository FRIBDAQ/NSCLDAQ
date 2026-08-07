import logging
import sys
import traceback

from PyQt5.QtCore import QObject, QThreadPool, QRunnable, pyqtSignal, pyqtSlot

from pixie_error import PixieError

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
        """Initialize the runner function with passed args, kwargs."""
        try:
            self.signals.running.emit()
            result = self.fn(*self.args, **self.kwargs)
        except Exception:
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
    functions connected to the thread's `running`, `finished`, `results`, and
    `error` signals. The `start_thread()` method will create, configure, and
    run an object encapsulating a worker thread in a QRunnable. Note that
    QThreadPool takes ownership of the runnable and will delete it if it
    returns true; otherwise ownership remains with the caller. Worker failures
    are always reported by `_report_worker_error`; the `errors` list adds
    recovery handlers on top of that reporting.

    Attributes
    ----------
    pool : QThreadPool
        Global thread pool.

    Methods
    -------
    start_thread(fcn, running, finished, results, errors)
        Reserve a thread and run its callback, wiring the signal handlers.
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
        In any event, `running`, `finished`, and `results` handlers are
        expected to take no arguments (a `results` handler may accept the
        returned object). Worker failures are always reported by
        `_report_worker_error`; any handlers in `errors` are connected in
        addition to that reporting, for recovery, not to replace it. If a
        handler requires arguments this can be circumvented via a
        locally-defined lambda function; the same can be done with `fcn`.

        Parameters
        ----------
        fcn : function
            Function to run in the thread.
        running : list of callables
            Functions connected to the `running` signal (fcn started).
        finished : list of callables
            Functions connected to the `finished` signal (fcn finished, on
            both success and failure).
        results : list of callables
            Functions connected to the `result` signal (fcn returned
            successfully); receive the returned object.
        errors : list of callables
            Recovery functions connected to the `error` signal, in addition
            to the always-connected reporter.
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
        # Reporting is always connected; errors= adds recovery handlers,
        # it does not replace reporting:
        worker.signals.error.connect(self._report_worker_error)
        for f in errors or []:
            worker.signals.error.connect(f)
        self.pool.start(worker)

    def get_active_thread_count(self):
        """Return the number of threads from QThreadPool.getActiveThreadCount().

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

    def _report_worker_error(self, err):
        """Report a worker failure. Connected to every worker.

        PixieError was already logged at origin with its reason, so only
        the user-facing line is emitted. Anything else is a bug and gets a
        full traceback in the log.

        Parameters
        ----------
        err : tuple
            (exctype, value, traceback) emitted by the worker.
        """
        exctype, value, tb = err
        if not isinstance(value, PixieError):
            _logger.error(f"Unexpected worker error:\n{tb}")
        print(f"Operation failed: {value}")
