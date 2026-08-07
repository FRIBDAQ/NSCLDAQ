import logging

_logger = logging.getLogger("qtscope_logger")


class WidgetFactory:
    """Factory for creating GUI widgets.

    Attributes
    ----------
    logger : Logger
        QtScope Logger object.
    builders : dict
        Dictionary of builder methods for concrete classes.

    Methods
    -------
    register_builder(key, builder)
        Register a builder by key name.
    create(key)
        Create a widget using its builder.
    """

    def __init__(self):
        """WidgetFactory class constructor."""

        self.builders = {}

    def register_builder(self, key, builder):
        """Register a builder with a key.

        Parameters
        ----------
        key : str
            Key name for builders dictionary.
        builder
            Builder method for the concrete class.
        """
        self.builders[key] = builder
        _logger.debug(f"\tRegistered: {key}")

    def create(self, key, *args, **kwargs):
        """Create an instance of a widget from its key.

        Additional parameters are passed to the builder method using *args
        and **kwargs.

        Parameters
        ----------
        key : str
            Get the builder method from the dictionary corresponding to this
            key string.
        *args : tuple
            Other positional arguments passed to widget builder (generally
            not used).
        **kwargs : tuple
            Keyword arguments passed to widget builder (e.g. number of
            channels for this module via `nchannels` for channel params or
            number of modules in the system via `nmodules` for module params).

        Returns
        -------
        An instance of the widget class, if it exists.

        Raises
        ------
        ValueError
            If the key doesn't correspond to a registered builder.
        """
        builder = self.builders.get(key)
        if not builder:
            raise ValueError(key)
        return builder(*args, **kwargs)
