"""
Window node implementation

openRogue has a rather unique understanding of windows, -
they are base game objects that do have standard interfaces as every other object in the scene
"""
# from . import node
from . import ui
from .. import ffi

from ..types import component, Vector

# TODO Random roguelike style window titles on default

# TODO Customizable closing behavior (and other things)


class WindowComponent(component.Component):
    """
    NodeUI component that manages the behavior of binded window
    It should not interfere with any possible node attribute
    """
    def __init__(self):
        # Specific for this window API
        # TODO Ability to change desired API on creation or after
        # POSSIBLE SOLUTION: Setting global API state machine:
        #   ffi.bind_window_creation_api("curses")
        self._api = ffi.manager.resolve("default")

        self._window = self._api.init_window(self.size.width, self.size.height,
                                             self.name)
        #
        component.deploy_front(self, "free", self._free_window)
        #
        component.deploy_back(self, "update", self._update_window)
        #
        component.deploy_back(self,
                              "size",
                              self._resize_window,
                              propf="setter")
        #
        component.deploy_back(self, "pos", self._repos_window, propf="setter")

    def _update_window(self, *args):
        """
        """
        event_queue = self._api.process_window(self._window)

        print("len", event_queue.contents.len)

        for i in range(event_queue.contents.len):
            event = event_queue.contents.events[i]
            if event.type == ffi.EventType.CLOSE_EVENT:
                self.close_window_behaviour()

            elif event.type == ffi.EventType.RESIZE_EVENT:
                self.resize_window_behaviour(
                    Vector(event.resize_event.width,
                           event.resize_event.height))

            elif event.type == ffi.EventType.REPOS_EVENT:
                self.repos_window_behaviour(
                    Vector(event.repos_event.x, event.repos_event.y))

        self._api.free_event_queue(event_queue)

    def _free_window(self, *args):
        # Prevent double free after force deletion
        if self._window is not None:
            print("Window is closed:", self.name)
            self._api.close_window(self._window)
            self._window = None

    def _resize_window(self, *args):
        self._api.resize_window(self._window, self.size.width,
                                self.size.height)

    def _repos_window(self, *args):
        self._api.repos_window(self._window, self.pos.x, self.pos.y)

    # ------------------------------------------------------------ Behaviors --- #
    # Special window component callbacks that are called on specific window events
    # They could overrided if needed

    def close_window_behaviour(self) -> None:
        """
        Called when system window receives close event
        By default it deletes the object from its parent and thus leaves the tree
        """
        self._parent().free_child(self.name)

    def resize_window_behaviour(self, size: Vector) -> None:
        """
        Called when system window changes its size
        """
        self.size = size

    def repos_window_behaviour(self, pos: Vector) -> None:
        """
        Called when system window changes its position
        """
        self.pos = pos
