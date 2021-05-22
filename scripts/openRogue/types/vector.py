"""
"""

# TODO Calculation methods
# TODO Numpy implementation if needed (or even binding to cglm if we need calculations)

class Vector:
	"""
	"""
	__slots__ = ("_x", "_y")

	def __init__(self, x, y):
		self._x = x
		self._y = y

	def set_x(self, x): self._x = x
	def get_x(self): return self._x

	def set_y(self, y): self._y = y
	def get_y(self): return self._y

	# aliases for coords
	x = property(get_x, set_x)
	y = property(get_y, set_y)
	# aliases for sizes
	width = property(get_x, set_x)
	height = property(get_x, set_x)
