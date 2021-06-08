"""
Base class of UI nodes
"""

from . import node
from ..types import Vector
from .primitives import Primitive
from typing import Union, Iterable

# Position and size should be in floating points where whole part is n of tiles relative to upper-left corner
# QUESTION: Where should tile sizes be determined?


class NodeUI(node.Node):

    __slots__ = ("x", "y", "width", "height", "align", "stretch", "is_shown",
                 "_size", "_pos")

    def __init__(self,
                 x=0,
                 y=0,
                 width=120,
                 height=80,
                 align="ul",
                 stretch="h",
                 is_shown=True):
        # Are short args preferable?
        """
        Possible align args: "upperleft", "upperright", "bottomleft", "bottomright", "ul", "ur", "bl", "br"
        Possible stretch args: any combination of "h" and "v" symbols or "" empty string for no stretch
        """
        node.Node.__init__(self)
        self.event_ports["ui"] = self.ui_event
        self.pos = Vector(x, y)
        self.size = Vector(width, height)
        self.align = align
        self.stretch = stretch
        self.is_shown = is_shown

    @property
    def size(self):
        return self._size

    @size.setter
    def size(self, size: Vector):
        # print("({}) new size: {}".format(self.name, size.as_tuple()))
        self._size = size

    @property
    def pos(self):
        return self._pos

    @pos.setter
    def pos(self, pos: Vector):
        # print("({}) new pos: {}".format(self.name, pos.as_tuple()))
        self._pos = pos

    def ui_event(self, event: object) -> None:
        """
        TODO
        """

    def recompose(self) -> None:
        """
        Used for fitting the new configuration of ui node
        """

    def show(self) -> None:
        """
        """
        self.is_shown = True

    def hide(self) -> None:
        """
        """
        self.is_shown = False

    # Render is not a good name in this case
    # TODO Should be cacheable
    def render(self) -> Union[Primitive, Iterable[Primitive], None]:
        """
        Should provide primitives that do describe the desirable graphical structure
        Return value could be a single primitive, iterable of primitives or None
        """
        return None
