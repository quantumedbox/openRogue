"""
Base class of UI nodes
"""

from . import node
from .. types import Vector

# Position and size should be in floating points where whole part is n of tiles relative to upper-left corner
# QUESTION: Where should tile sizes be determined?

class NodeUI(node.Node):

	def __init__(self, x=0, y=0, width=120, height=80):
		node.Node.__init__(self)
		self.event_ports["ui"] = self.ui_event
		self._pos = Vector(x, y)
		self._size = Vector(width, height)


	@property
	def size(self):
		return self._size

	@size.setter
	def size(self, size: Vector):
		self._size = size

	@property
	def pos(self):
		return self._pos

	@pos.setter
	def pos(self, pos: Vector):
		self._pos = pos


	def ui_event(self, event: object) -> None:
		"""
		TODO
		"""

	# Render is not a good name in this case
	def render(self) -> ...:
		"""
		"""
