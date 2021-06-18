"""
Node that structures children UI nodes by its rules
"""
from openRogue.nodes import ui


class Container(ui.NodeUI):
    """
    Shortcut for creating different container types
    Type variants: vertical and horizontal
    """

    # __slots__ = ()

    def __init__(self, type="vertical", **kwargs):
        ui.NodeUI.__init__(self, **kwargs)
        # if type == "vertical":
        #   raise NotImplemented("TODO vertical container")
        # else:
        #   raise NotImplemented("TODO horizontal container")


class VerticalContainer(ui.NodeUI):
    """
    """
    def __init__(self):
        """
        """
