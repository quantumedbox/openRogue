"""
Special UI node that is designed to fill the space within containers if needed
Main use: to align elements to bottom or right
"""
from openRogue.nodes import ui


class Filler(ui.NodeUI):
    def __init__(self):
        ui.NodeUI.__init__(self)

    def render(self):
        """
        Doesn't have any graphics attached
        """
        return None
