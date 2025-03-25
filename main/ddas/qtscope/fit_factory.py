import logging

class FitFactory:
    """Factory method for fitting functions.
    
    Attributes
    ----------
    logger : Logger
        QtScope Logger object.
    builders : dict 
        Dictionary of builder methods.
    config : dict 
        Dictionary of configurations.

    Methods
    -------
    register_builder(key, builder, config)
        Register a builder and config by key name.
    create(key)
        Create an instance of a fit function from a key name.
    initialize(item) 
        Initial setup of the fit factory for supported functions.
    """
    
    def __init__(self):
        """FitFactory constructor."""
        self.logger = logging.getLogger("qtscope_logger")
        self.builders = {}
        self.configs = {}

    def register_builder(self, key, builder, config):
        """Register a fitting function builder.

        Parameters
        ----------
        key : str 
            Key name for the fitting function.
        builder 
            Builder method for the fitting function class.
        config : dict 
            Parameter dictionary with initial guesses.
        """        
        self.builders[key] = builder
        self.configs[key] = config
        self.logger.debug(f"\tRegistered: {key}")
        self.logger.debug(f"\t\tConfig: {config}")

    def create(self, key):
        """Create an instance of the fit function from a key value. 

        Uses the configuration dictionary registered with the factory for the 
        provided key value if it exists.

        Parameters
        ----------
        key : str
            Key name for the fitting function.

        Returns
        -------
        An instance of the fitting function class.

        Raises
        ------
        ValueError
            If the key does not correspond to a registered builder.
        """
        builder = self.builders.get(key)
        if not builder:
            raise ValueError(key)        
        return builder(**self.configs.get(key))
        
    def initialize(self, item):
        """Initialize the list of fitting functions.

        Parameters
        ----------
        item : QWidget
            Add known fit methods to a widget. Practically this is a QComboBox
            in the FitPanel.
        """
        self.logger.debug("Initializing fit functions:")     
        for key in self.builders:
            item.addItem(key)
            self.logger.debug(f"\tAdded: {key}")
