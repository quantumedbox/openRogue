"""
Base class of UI nodes
"""
from openRogue.nodes import node
from openRogue.types import Vector

from typing import Union, Iterable

# Position and size should be in floating points where whole part is n of tiles relative to upper-left corner
# QUESTION: Where should tile sizes be determined?


class NodeUI(node.Node):

    __slots__ = ("x", "y", "width", "height", "style", "style_id", "align",
                 "stretch", "is_shown", "_size", "_pos")

    def __init__(self,
                 x=0,
                 y=0,
                 width=120,
                 height=80,
                 style="",
                 style_id="",
                 style_attrs={},
                 align="ul",
                 stretch="h",
                 is_shown=True):
        # Are short args preferable?
        """
        Possible align args: "upperleft", "upperright", "bottomleft", "bottomright", "ul", "ur", "bl", "br"
        Possible stretch args: any combination of "h" and "v" symbols or "" string for no stretch
        """
        super().__init__()

        # Stupid fix for problem of 'recompose' calling when attrs are not yet set
        self._pos = Vector(x, y)
        self._size = Vector(width, height)

        self.pos = Vector(x, y)
        self.size = Vector(width, height)
        self.style = style
        self.style_id = style_id
        self.style_attrs = style_attrs
        self.align = align
        self.stretch = stretch
        self.is_shown = is_shown

    @property
    def size(self):
        return self._size

    @size.setter
    def size(self, size: Vector):
        print(f"{self.name}: new size -> ({size.x}, {size.y})")
        self._size = size
        self.recompose()

    @property
    def pos(self):
        return self._pos

    @pos.setter
    def pos(self, pos: Vector):
        print(f"{self.name}: new pos -> ({pos.x}, {pos.y})")
        self._pos = pos
        self.recompose()

    def render(self, carry) -> None:
        """
        TODO
        """

    def receive_ui_event(self, event: object) -> None:
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
