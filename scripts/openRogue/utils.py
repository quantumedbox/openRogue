from typing import Callable


# it's kinda ugly
def override_method(obj: object, attr: str, fn: Callable) -> None:
    """
	Helper for setting new method with descriptor for 'self' binding
	"""
    setattr(obj, attr, fn.__get__(obj, obj.__class__))
