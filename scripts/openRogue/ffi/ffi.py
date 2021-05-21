"""
Integration with C backend
"""

# TODO Needs serious work
# For now idea is to make FFIInterface object that does the communications with shared objects
# That way we could actually have multiple APIs at the same time

import os
from ctypes import *

from . definitions import *
from .. config_reader import get_config

# 
_SHARED = None
_FFI = None


class FFIManager:
	"""
	"""
	__slots__ = ("interfaces",)

	def __init__(self):
		"""
		Init new FFI manager with default api from config
		"""
		self.interfaces = {}
		backend_api = get_config("backend_api")
		self.register(backend_api, "default")

	def resolve(self, name: str) -> object:
		"""
		"""
		if name in self.interfaces:
			return self.interfaces[name]
		raise KeyError("No registered API with the name of {}".format(name))

	def register(self, path: str, name: str):
		"""
		"""
		full_path = os.path.abspath(path)
		cffi = CDLL(full_path)
		self.interfaces[name] = FFIInterface(cffi)


class FFIInterface:
	"""
	"""
	__slots__ = (
		"init_window",
		"close_window",
		"process_window",
		"dispatch_window_events",

		# 
		"_free_event_queue",
	)

	def __init__(self, shared):
		self.init_window = shared.init_window
		self.init_window.restype = c_void_p

		self.close_window = shared.close_window
		self.close_window.argtypes = (c_void_p,)

		# self.update_window = shared.update_window
		# self.update_window.argtypes = (c_void_p,)

		self.process_window = shared.process_window
		self.process_window.restype = c_void_p
		self.process_window.argtypes = (c_void_p,)

		self.dispatch_window_events = shared.dispatch_window_events
		self.dispatch_window_events.restype = POINTER(C_EventQueue)
		self.dispatch_window_events.argtypes = (c_void_p,)

		self._free_event_queue = shared._free_event_queue
		self._free_event_queue.argtypes = (POINTER(C_EventQueue),)


def _init_shared() -> None:
	"""
	"""
	global _SHARED, _FFI

	try:
		_SHARED = CDLL(os.path.abspath(DEFAULT_SHARED_PATH))
	except:
		raise

	_FFI = FFIInterface(_SHARED)


# TODO Move window stuff in separate file

class WindowHandle:
	"""
	"""
	__slots__ = ("w_p", "is_closed")

	def __init__(self, w_p: object):
		self.w_p = w_p
		self.is_closed = False

	def state_dispatch(self, mask: int) -> None:
		if mask & WindowSignal.WINDOW_SIGNAL_CLOSED:
			_SHARED.close_window(c_void_p(self.w_p))
			self.is_closed = True


def init_window(width: int, height: int, title: str) -> WindowHandle:
	"""
	"""
	w_p = _SHARED.init_window(c_int(width), c_int(height), c_char_p(title))
	return WindowHandle(w_p)


def process_window(w: WindowHandle) -> None:
	"""
	"""
	signal_mask = _SHARED.process_window(c_void_p(w.w_p))
	if signal_mask != 0:
		print(signal_mask)
		w.state_dispatch(signal_mask)


class EventQueueLifetime:
	"""
	Used as deallocator call of memory on C side even queue is no longer needed
	"""
	__slots__ = ('queue',)

	def __init__(self, queue: C_EventQueue):
		self.queue = queue

	def __del__(self):
		_SHARED._free_event_queue(self.queue)


def poll_window_events(w: WindowHandle):
	event_queue = _FFI.dispatch_window_events(w.w_p)
	l = EventQueueLifetime(event_queue)

	for i in range(event_queue.contents.len):
		event = event_queue.contents.events[i]

		if event.type == EventType.INPUT_EVENT:
			yield {
				"key": event.input_event.keycode,
				"is_pressed": event.input_event.is_key_pressed,
				"is_repeat": event.input_event.is_key_repeat,
				"action": event.input_event.action,
				"mod": event.input_event.keymod,
			}

		elif event.type == EventType.POINTER_EVENT:
			yield {
				"action": event.pointer_event.mouse_action,
				"x": event.pointer_event.x,
				"y": event.pointer_event.y,
				"x_motion": event.pointer_event.x_motion,
				"y_motion": event.pointer_event.y_motion,
			}
