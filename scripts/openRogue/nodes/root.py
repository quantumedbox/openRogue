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
	Used as layer of integration of openRogue objects into system
	It mimics the desktop in ways that allow children to be aware of their screen position and etc.
	Every children of root that is derived from UI class is considered to be its own system window
	"""
    __slots__ = (
        "size",
        "exit_without_window_children",
    )

    def __init__(self):
        node.Node.__init__(self)
        self.name = "root"
        # TODO get screen size (should it be implemented in the Backend? by SDL_ListModes() for example)
        self.size = Vector(0, 0)
        self.exit_without_window_children = True

    def attach_child(self, name: str, child: 'Node'):
        """
		Contextual constructor that implements system window component for every attached UI child
		"""
        child.name = name
        if issubclass(type(child), ui.NodeUI):
            implement_component(child, window.WindowComponent)
        node.Node.attach_child(self, name, child)

    def loop(self):
        """
		Init game loop
		"""
        while True:
            # Check if any of children implement window component and if not, - exit the loop
            if self.exit_without_window_children:
                for c in self._children:
                    if issubclass(type(c), window.WindowComponent):
                        break
                else:
                    break
            # Emmit update event each loop for nodes to be processed
            self.emit_event("update", None)

        # Signal on_exit for every module that could need to do some work before exiting the program
        signal.signal("on_exit")
