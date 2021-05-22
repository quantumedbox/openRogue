"""
Base class of UI nodes
"""

from . import node
from .. types import Vector

# Position and size should be in floating points where whole part is n of tiles relative to upper-left corner
# QUESTION: Where should tile sizes be determined?

class NodeUI(node.Node):

	def __init__(self, x=0, y=0, width=64, height=64):
		node.Node.__init__(self)
		self.event_ports["ui"] = self.ui_event
		self.position = Vector(x, y)
		self.size = Vector(width, height)


	def ui_event(self, event: object) -> None:
		"""
		TODO
		"""

	# Render is not a good name in this case
	def render(self) -> ...:
		"""
		"""
