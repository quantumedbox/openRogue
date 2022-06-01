"""
Module for providing the flow control in your node trees
"""
from openRogue.nodes import node

# TODO
# Idea of filters: for example, you could create a filter that prevents event passing for every object that isn't visible
"""
EXAMPLE:
def my_filter(obj, ptype) -> bool:
	if ptype is not "update":
		return false
	if type(obj) is ui.NodeUI:
		# true means that event should be passed
		return true
	return false
"""


class Context(node.Node):
    """
	Provides flow control for incoming events
	"""
