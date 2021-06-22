"""
Base interface for every scene object
"""
import weakref

from typing import Union, Any
from collections import OrderedDict

# ??? Maybe it's better to return/store proxies and not weakrefs?

# TODO Adding nodes while iterating could crash the app, we should prevent it

# TODO Re-attachment of nodes


class Node:
    """
    Basic node that implements tree hierarchy and event logic
    """
    __slots__ = (
        "name",
        "_parent",
        "_children",
        "__weakref__",
    )

    # Used for queuing nodes to be deleted
    _freeing_queue = []

    def __init__(self):
        self._parent = None
        self.name = "unnamed"
        self._children = OrderedDict()

    def init_child(self, name: str, cls, **kwargs) -> 'Node':
        """
        Preferable way of child instancing by name and class
        Arguments for class instancing are passed through kwargs
        """
        child = cls(**kwargs)
        return self.attach_child(name, child)

    def attach_child(self, name: str, child: 'Node') -> 'Node':
        """
        Attach already instanced node
        Used internally on init_child
        """
        if name in self._children:
            raise NameError(
                f"Child by the name of \"{name}\" is already present in {self.name}"
            )
        child.name = name
        self._children[name] = child
        child._parent = weakref.ref(self)
        return child

    def get_child(self, name: str) -> Union['Node', None]:
        """
        """
        return self._children.get(name)

    def queue_free(self) -> None:
        """
        Queue specified child to be deleted
        """
        Node._freeing_queue.append(self)

    def get_parent(self) -> 'Node':
        """
        """
        if self._parent is not None:
            return self._parent()
        return None

    def recieve_event(self, event_type: str, event_packet: object) -> None:
        """
        """
        if event_type == "update":
            if hasattr(self, "update"):
                self.update(event_packet)
            for _, child in self._children.items():
                child.recieve_event(event_type, event_packet)

    def update(self, event_packet) -> None:
        """
        -- FREE TO OVERRIDE --
        """

    def free(self) -> None:
        """
        -- FREE TO OVERRIDE --
        Used for deallocating data which is not tracked by garbage collector
        Or as callback on deletion
        """

    def __del__(self) -> None:
        self.free()
