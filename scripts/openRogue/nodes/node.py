"""
Base interface for every scene object
"""
# import uuid
import weakref
from typing import Union
from collections import OrderedDict

# ??? Maybe it's better to return/store proxies and not weakrefs?

# TODO Vertual method "tree_entered" that is called when node is added to an other node that is in /root/ tree
# TODO Re-attachment of nodes
# TODO Concrete name structure
# TODO Delete node from scene without getting the parent ?


class Node:
    """
    Basic node implements tree hierarchy and event ports

    Parent should be the owner of all children nodes that only
    gives weak references to access them
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
        """
        """
        self._children = OrderedDict()
        self._parent = None
        # Names only make sense in context of node trees, parent should set the name
        self.name = "unnamed"

    def update(self, event) -> None:
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
        self.attach_child(name, child)
        return child

    def attach_child(self, name: str, child: 'Node') -> None:
        """
        Attach already instanced node
        Used internally on init_child
        """
        # TODO Check if the name is available
        if name in self._children:
            raise NameError(
                f"Child by the name of \"{name}\" is already present in {self.name}"
            )
        child.name = name
        self._children[name] = child
        child._parent = weakref.ref(self)

    def get_child(self, name: str) -> Union['Node', None]:
        """
        """
        return self._children.get(name)

    def queue_free(self) -> None:
        """
        -- DO NOT OVERRIDE --
        Queue specified child to be deleted
        """
        Node._freeing_queue.append(self)
        # child = self._children.get(name)
        # if child is not None:
        #     self._children.pop(name)
        #     child.free()
        # else:
        #     raise KeyError(
        #         f"No child by the name of {name} to free in node {self.name}")

    def get_parent(self) -> 'Node':
        """
        """
        if self._parent is not None:
            return self._parent()
        return None

    def emit_event(self, ptype: str, event: object) -> None:
        """
        """
        # Current design doesn't allow receiving all possible event types if needed
        for _, child in self._children.items():
            func_port = getattr(child, ptype)
            if func_port is not None:
                func_port(event)

    # def __str__(self) -> str:
    # return "\n".join([
    # "%s (%s)" % (self.name, type(self).__name__),
    # "children: {}".format(", ".join(
    #   "%s (%s)" % (child.name, type(child).__name__) for child in self._children)
    # ),
    # "event_ports: {}".format(self.event_ports),
    # ])
