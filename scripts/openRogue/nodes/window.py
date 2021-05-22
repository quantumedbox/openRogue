"""
Window node implementation

openRogue has a rather unique understanding of windows, -
they are base game objects that do have standard interfaces as every other object in the scene
"""
# from . import node
from . import ui
from .. import ffi

from .. types import component

# TODO Random roguelike style window titles on default

# TODO Customizable closing behavior


class WindowComponent(component.Component):
	"""
	NodeUI component that manages the behavior of binded window
	It should not interfere with any possible node attribute
	"""
	def __init__(self):
		self._api = ffi.manager.resolve("default")
		self._window = self._api.init_window(self.size.width, self.size.height, str.encode(self.name))
		self.free = component.pipe_before(self.free, self._free_window)
		self.update = component.pipe_after(self.update, self._update_window)


	def _update_window(self, _):
		"""
		"""
		event_queue = self._api.process_window(self._window)

		# for i in range(event_queue.contents.len):
		# 	print(event_queue.contents.events[i])

		self._api._free_event_queue(event_queue)


	def _free_window(self):
		# Prevent double free after force deletion
		if self._window is not None:
			print("Window is closed:", self.name)
			self._api.close_window(self._window)
			self._window = None
