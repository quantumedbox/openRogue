"""
Window node implementation

openRogue has a rather unique understanding of windows, -
they are base game objects that do have standard interfaces as every other object in the scene
"""
from .. import node
from ... import ffi

class Window(node.Node):
	"""
	"""
	__slots__ = (
		"api",
		"window_pointer",
		"is_closed",
		)

	def __init__(self, width=600, height=400, title="openRogue", api="default"):
		"""
		kwargs:
		. width 	-- window width, default is 600
		. height 	-- width height, default is 400
		. title		-- window title, default is "openRogue"
		. api 		-- which api should be used for this particular window.
					by default it is determined by "backend_api" config setting
		"""
		node.Node.__init__(self)
		self.api = ffi.manager.resolve(api)
		self.window_pointer = self.api.init_window(width, height, str.encode(title))
		self.is_closed = False

	def __del__(self):
		"""
		"""
		print("Window is closed: ", self)
		self.api.close_window(self.window_pointer)


	def events(self):
		"""
		"""
		event_queue = self.api.dispatch_window_events(self.window_pointer)
		l = EventQueueLifetime(event_queue, self.api)

		for i in range(event_queue.contents.len):
			event = event_queue.contents.events[i]

			if event.type == ffi.EventType.INPUT_EVENT:
				yield {
					"key": event.input_event.keycode,
					"is_pressed": event.input_event.is_key_pressed,
					"is_repeat": event.input_event.is_key_repeat,
					"action": event.input_event.action,
					"mod": event.input_event.keymod,
				}

			elif event.type == ffi.EventType.POINTER_EVENT:
				yield {
					"action": event.pointer_event.mouse_action,
					"x": event.pointer_event.x,
					"y": event.pointer_event.y,
					"x_motion": event.pointer_event.x_motion,
					"y_motion": event.pointer_event.y_motion,
				}


class EventQueueLifetime:
	"""
	Used as deallocator call of memory on C side even queue is no longer needed
	"""
	__slots__ = ('queue', "api")

	def __init__(self, queue, api):
		self.queue = queue
		self.api = api

	def __del__(self):
		self.api._free_event_queue(self.queue)
