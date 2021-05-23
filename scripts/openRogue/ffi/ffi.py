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
		"resize_window",
		"repos_window",

		# 
		"_free_event_queue",
	)

	def __init__(self, shared):
		self.init_window = shared.init_window
		self.init_window.restype = c_void_p

		self.close_window = shared.close_window
		self.close_window.argtypes = (c_void_p,)

		self.process_window = shared.process_window
		self.process_window.restype = POINTER(C_EventQueue)
		self.process_window.argtypes = (c_void_p,)

		self.resize_window = shared.resize_window
		self.resize_window.argtypes = (c_void_p, c_int, c_int)

		self.repos_window = shared.repos_window
		self.repos_window.argtypes = (c_void_p, c_int, c_int)

		self._free_event_queue = shared._free_event_queue
		self._free_event_queue.argtypes = (POINTER(C_EventQueue),)
