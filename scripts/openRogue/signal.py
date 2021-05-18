from typing import Callable

# Signals are designed to be used for initializations and control flow of modular projects
# Main advantage is that caller modules do not need to know about existence of others
# They could just call the signal to get desirable data from many places at the same time

# One thing - signal naming conventions should be strong for module comparabilities

# Use cases:
#	. getting the information about all initialized modules


# Global signal map for lists of callable objects
_signals = {}


def impl_signal(s: str, f: Callable) -> None:
	"""
	Add a callback function to a named signal event
	This preferably should be called in module's __init__

	Example:
	impl_signal("get_hello_world", lambda: "Hello, World!")
	impl_signal("say_hello_world", lambda: print("Hello, World!"))
	"""
	if s in _signals:
		_signals[s].append(f)
	else:
		_signals[s] = [f]


def signal(s: str) -> None:
	"""
	Execute callbacks for s event without the consideration of return values

	Example:
	signal("say_hello_world")
	"""
	if s not in _signals:
		return
	for c in _signals[s]:
		c()


def signal_dispatch(s: str, d: Callable) -> None:
	"""
	Execute callbacks for s event and call d with return of each callback as argument
	WARNING: d should have exactly 1 argument in its signature

	Example:
	hello_list = []
	signal_dispatch("get_hello_world", lambda x: hello_list.append(x))
	print(*hello_list)
	"""
	if s not in _signals:
		return
	for c in _signals[s]:
		d(c())
