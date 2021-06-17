"""
Base interface for every scene object
"""
# import uuid
import weakref
from typing import Union, Any
from collections import OrderedDict

# ??? Maybe it's better to return/store proxies and not weakrefs?

# TODO Vertual method "tree_entered" that is called when node is added to an other node that is in /root/ tree
# TODO Re-attachment of nodes
# TODO Concrete name structure
# TODO Delete node from scene without getting the parent ?

# Parent should be the sole owner of all its children


class Node:
    """
    Basic node that implements tree hierarchy and event logic
    """
    __slots__ = (
        "name",
        "_parent",
        "_children",
        "_components",
        "__weakref__",
    )

    # Used for queuing nodes to be deleted
    _freeing_queue = []

    def __init__(self):
        self._parent = None
        self.name = "unnamed"
        self._children = OrderedDict()
        self._components = OrderedDict()

    def update_event(self, event_packet) -> None:
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

    def recieve_event(self, event_name: str, event_packet: object) -> None:
        """
        """
        if event_name == "update_event":
            if hasattr(self, "update_event"):
                self.update_event(event_packet)
            for _, child in self._children.items():
                child.recieve_event(event_name, event_packet)
