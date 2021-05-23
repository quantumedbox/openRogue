"""
Base interface for every scene object
"""
# import uuid
import weakref
from typing import Union

# ??? Maybe it's better to return/store proxies and not weakrefs?

# TODO Vertual method "tree_entered" that is called when node is added to an other node that is in /root/ tree
# TODO Re-attachment of nodes
# TODO Concrete name structure


class Node:
	"""
	Basic node implements tree hierarchy and event ports

	Parent should be the owner of all children nodes that only
	gives weak references to access them
	"""
	__slots__ = (
		"name",
		"event_ports",
		"_parent",
		"_children",

		"__weakref__",
	)

	# TODO _children should be implemented as OrderedDict

	def __init__(self):
		"""
		"""
		self._children = []
		self._parent = None
		self.event_ports = {"update": "update"}
		# Names only make sense in context of node trees, parent should set the name
		self.name = ""


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
		print(self, "was freed")


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
		child.name = name
		self._children.append(child)
		child._parent = weakref.ref(self)


	def get_child(self, name: str) -> Union['Node', None]:
		"""
		"""
		for child in self._children:
			if child.name == name:
				return child

		return None


	def free_child(self, name: str) -> None:
		"""
		"""
		for child in self._children:
			if child.name == name:
				self._children.remove(child)
				child.free()
				return

		raise KeyError("No child by the name of {} to free in node {}".format(name, self.name))


	def get_parent(self) -> 'Node':
		"""
		"""
		if self._parent is not None:
			return self._parent()
		return None


	def emit_event(self, ptype: str, event: object) -> None:
		"""
		"""
		for child in self._children:
			if ptype in child.event_ports:
				getattr(child, child.event_ports[ptype])(event)


	def __str__(self) -> str:
		return "\n".join(
			[
				"%s (%s)" % (self.name, type(self).__name__),
				# "children: {}".format(", ".join(
				# 	"%s (%s)" % (child.name, type(child).__name__) for child in self._children)
				# ),
				# "event_ports: {}".format(self.event_ports),
			]
		)
