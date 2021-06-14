"""
Corner stone of scene structure
"""
from . import node
from . import ui
from . import window
from ..types import Vector, implement_component
from .. import signal


class Root(node.Node):
    """
    Used as layer of integration of openRogue objects into operating system
    It mimics the desktop in ways that allow children to be aware of their screen position and etc.
    Every children of root that is derived from UI class is considered to be its own system window
    """
    __slots__ = (
        "size",
        "_should_stop",
    )

    def __init__(self):
        node.Node.__init__(self)
        self.name = "root"
        # TODO get screen size (should it be implemented in the Backend? by SDL_ListModes() for example)
        self.size = Vector(0, 0)
        self._should_stop = False

    def attach_child(self, name: str, child: 'Node') -> None:
        """
        Contextual constructor that implements system window component for every attached UI child
        """
        node.Node.attach_child(self, name, child)
        if issubclass(type(child), ui.NodeUI):
            implement_component(child, window.WindowComponent)

    def pre_loop(self) -> None:
        """
        -- FREE TO OVERRIDE --
        This function is called each cycle before everything else
        By default it contains logic that stops the loop when there's no window objects left
        """
        for _, child in self._children.items():
            if issubclass(type(child), window.WindowComponent):
                break
        else:
            self.stop()

    def post_loop(self) -> None:
        """
        -- FREE TO OVERRIDE --
        This function is called each cycle after all updates
        """

    def _loop(self) -> None:
        """
        Init game loop
        Should not be called manually
        """
        while True:
            # Check if any of children implement window component and if not, - exit the loop
            if self._should_stop == True:
                break
            self.pre_loop()
            # Emmit update event each loop for nodes to be processed
            self.emit_event("update", None)
            self.post_loop()

        # Signal on_exit for every module that could need to do some work before exiting the program
        signal.signal("on_exit")

    def stop(self) -> None:
        """
        Function that is used for stopping the loop
        """
        self._should_stop = True
