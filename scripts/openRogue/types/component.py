"""
"""

# TODO Make sure that deimplementation of components is also possible

class Component:
	"""
	Dummy class for distinguishing component classes
	"""
	pass


def implement_component(obj: object, component: Component) -> object:
	"""
	Constructs new class with SysWindow as additional base and copies all data from obj instance to a new one
	"""
	obj.__class__ = type(type(obj).__name__, (type(obj), component), {})
	component.__init__(obj)
	return obj


def pipe_before(func, pipe):
	"""
	"""
	def wrapper(*args, **kwargs):
		pipe(*args, **kwargs)
		func(*args, **kwargs)

	return wrapper


def pipe_after(func, pipe):
	"""
	"""
	def wrapper(*args, **kwargs):
		func(*args, **kwargs)
		pipe(*args, **kwargs)

	return wrapper
