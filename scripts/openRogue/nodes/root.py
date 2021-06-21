"""
Corner stone of scene structure
"""
from openRogue.nodes import node, ui, window, container
from openRogue.types import Vector
from openRogue import signal


class Root(node.Node):
    """
    Used as layer of integration of openRogue objects into operating system
    It mimics the desktop in ways that allow children to be aware of their screen position and etc.
    Every children of root that is derived from UI class is considered to be its own system window
    """
    __slots__ = (
        "size",
        "_should_stop",
        "_max_tile_size",
    )

    def __init__(self):
        super().__init__()
        self.name = "root"
        # TODO get screen size (should it be implemented in the Backend? by SDL_ListModes() for example)
        # Screen resolution
        self.size = None
        # Limits the maximum amount of tiles that could be rendered
        self._should_stop = False

    def attach_child(self, name: str, child: 'Node') -> 'Node':
        """
        Contextual constructor that implements system window component for every attached UI child
        """
        child.name = name
        if issubclass(type(child), ui.NodeUI):
            child = window.WindowComponent(child)
        super().attach_child(name, child)
        return child

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
        Usually is called from engine
        """
        while not self._should_stop:
            self.pre_loop()
            # Emmit update event each loop for nodes to be processed
            self.recieve_event("update", None)
            self.post_loop()

            # ??? Should it be here ?
            while super()._freeing_queue:
                to_free = super()._freeing_queue.pop()
                parent = to_free._parent()
                if parent is not None:
                    parent._children.pop(to_free.name)
                to_free.free()

        # Signal on_exit for every module that could need to do some work before exiting the program
        signal.signal("on_exit")

    def stop(self) -> None:
        """
        Function that is used for stopping the loop
        """
        self._should_stop = True
