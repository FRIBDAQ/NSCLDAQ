from PyQt5.QtWidgets import QWidget, QGroupBox, QVBoxLayout


class ModDSPLayout(QWidget):
    """Layout of module DSP parameters.

    Uses the mod_dsp_factory to configure a set of widgets. Each of these
    widgets is contained in a group box of similar settings.

    Attributes
    ----------
    param_widgets : list
        List of widgets contained in the layout.
    box_dict : dict
        Dictionary of DSP parameter widgets and their factory creation methods
        to be included in each group box.
    """

    def __init__(self, factory, nmodules, channel_map, *args, **kwargs):
        """
        ModDSPLayout class constructor.

        Parameters
        ----------
        factory : WidgetFactory
            Factory for implemented module DSP parameters.
        nmodules : int
            Number of modules from ModDSPManager.
        channel_map : list
            List of channels per module.
        """
        super().__init__(*args, **kwargs)

        # List of DSP parameter widgets. Unlike the channel DSP tabbed widget,
        # there is not "easy" method to access the constituent class widgets
        # for the module parameters using PyQt methods. And I don't really want
        # to parse lists of child widgets.

        self.param_widgets = []

        # Dictionary of DSP parameter widgets included in each group box. Key
        # is the group box name, value is a list of class widgets to include in
        # that box (by factory create method name).

        self.box_dict = {
            "Crate settings": ["CrateID"],
            "Crate configuration (CSRB) options": ["CSRB"],
            "Trigger configuration options": ["TrigConfig0", "TrigConfigExtra"],
        }

        # Define layout:

        layout = QVBoxLayout()
        for box_name, widget_names in self.box_dict.items():
            box = QGroupBox(box_name)
            box_layout = QVBoxLayout()
            # Create the widget and add it to the list and group layout:
            for name in widget_names:
                w = factory.create(name, nmodules=nmodules, channel_map=channel_map)
                if w:
                    self.param_widgets.append(w)
                    box_layout.addWidget(w)

            box.setLayout(box_layout)
            layout.addWidget(box)

        self.setLayout(layout)
