"""
Base interface for every scene object
"""

class Node:
	"""
	"""
	__slots__ = ("children", "parent")

	def __init__(self):
		"""
		"""
		self.children = []
		self.parent = None
