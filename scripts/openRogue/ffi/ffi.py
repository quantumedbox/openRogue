"""
Integration with C backend
"""
from ctypes import *
import os

from . window_signals import WindowSignal


DEFAULT_SHARED_PATH = "./openRogue"

# 
_SHARED = None


def _init_shared() -> None:
	"""
	"""
	global _SHARED
	try:
		_SHARED = CDLL(os.path.abspath(DEFAULT_SHARED_PATH))
	except:
		raise


class WindowHandle:
	"""
	"""
	__slots__ = ("w_p", "is_closed")

	def __init__(self, w_p: object):
		self.w_p = w_p
		self.is_closed = False

	def state_dispatch(self, mask: int) -> None:
		if mask & WindowSignal.WINDOW_SIGNAL_CLOSED:
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
