"""
Window node implementation

openRogue has a rather unique understanding of windows, -
they are base game objects that do have standard interfaces as every other object in the scene
"""
from openRogue import ffi
from openRogue.nodes import ui, node, style_manager
from openRogue.types import component, Vector

# TODO Random roguelike style window titles on default

# TODO Support all kinds of input events


class WindowComponent(component.Component):
    """
    NodeUI component that manages the behavior of binded window
    It should not interfere with any possible node attribute
    """
    def __init__(self, base):
        # Specific for this window API
        # TODO Ability to change desired API on creation or after
        # POSSIBLE SOLUTION: Setting global API state machine:
        #   ffi.bind_window_creation_api("curses")
        super().__init__(base)

        self._api = ffi.manager.resolve("default")

        self._window = self._api.init_window(self.size.width, self.size.height,
                                             self.name)

        self._max_tile_size = Vector(self._api.get_spec(b"max_tile_size"))

        self._api.set_window_icon_from_file(self._window,
                                            b"resources/images/icon.png")

    def update(self, event_packet):
        self._base.update(event_packet)

        event_queue = self._api.get_window_events(self._window)

        for i in range(event_queue.len):
            event = event_queue.events[i]
            if event.type == ffi.EventType.CLOSE_EVENT:
                self.close_window_behaviour()

            elif event.type == ffi.EventType.RESIZE_EVENT:
                self.resize_window_behaviour(
                    Vector(event.resize_event.width,
                           event.resize_event.height))

            elif event.type == ffi.EventType.REPOS_EVENT:
                self.repos_window_behaviour(
                    Vector(event.repos_event.x, event.repos_event.y))

    def recieve_event(self, event_type, event_packet) -> None:
        self._base.recieve_event(event_type, event_packet)
        if event_type == "update":
            self.update(event_packet)
            self._drawing_cycle()

    def queue_free(self) -> None:
        node.Node._freeing_queue.append(self)

    def free(self):
        self._base.free()
        # Prevent double free after force deletion
        if self._window is not None:
            print(f"{self.name}: closed")
            self._api.close_window(self._window)
            self._window = None

    def get_size(self):
        return self._base.size

    def set_size(self, value: Vector):
        self._base.size = value
        self._api.resize_window(self._window, value.width, value.height)

    size = property(get_size, set_size)

    def get_pos(self):
        return self._base.pos

    def set_pos(self, value: Vector):
        self._base.pos = value
        self._api.repos_window(self._window, value.x, value.y)

    pos = property(get_pos, set_pos)

    def _drawing_cycle(self) -> None:
        """
        """
        init_render_package = {
            "_api": self._api,
            "_max_tile_size": self._max_tile_size,
            "x_origin": -self._base.pos.x,
            "y_origin": -self._base.pos.y,
        }

        self._api.start_drawing(self._window)

        # TODO Styles are rather costly now which isn't ideal

        stack = []

        # Node to which window is attached is a special case:
        style = style_manager.resolve(self._base.style)

        if style.get("common", None) is not None:
            init_render_package.update(style["common"].items())
        if style.get(self._base.style_id, None) is not None:
            init_render_package.update(style[self._base.style_id].items())
        init_render_package.update(self._base.style_attrs.items())

        # Render it as each tile is equal to a single pixel
        unit_render_package = init_render_package.copy()
        unit_render_package.update([("tile_width", 1), ("tile_height", 1)])
        self._base.render(unit_render_package)

        init_render_package["x_origin"] += self._base.pos.x
        init_render_package["y_origin"] += self._base.pos.y

        for _, child in reversed(self._base._children.items()):
            if isinstance(child, ui.NodeUI):
                stack.append((child, init_render_package.copy()))

        # Descend down the tree
        while len(stack) != 0:
            cur_node, render_package = stack.pop()

            style = style_manager.resolve(cur_node.style)

            if style.get("common", None) is not None:
                render_package.update(style["common"].items())
            if style.get(cur_node.style_id, None) is not None:
                render_package.update(style[cur_node.style_id].items())
            render_package.update(cur_node.style_attrs.items())

            cur_node.render(render_package)

            render_package[
                "x_origin"] += cur_node.pos.x * render_package["tile_width"]
            render_package[
                "y_origin"] += cur_node.pos.y * render_package["tile_height"]

            for _, child in reversed(cur_node._children.items()):
                if isinstance(child, ui.NodeUI):
                    stack.append((child, render_package.copy()))

        self._api.finish_drawing()

    # ------------------------------------------------------------ Behaviors --- #
    # Special window component callbacks that are called on specific window events
    # They could overrided if needed

    def close_window_behaviour(self) -> None:
        """
        Called when system window receives close event
        By default it deletes the object from its parent and thus leaves the tree
        """
        # self._parent().free_child(self.name)
        self.queue_free()

    def resize_window_behaviour(self, size: Vector) -> None:
        """
        Called when system window changes its size
        """
        self.size = size

    def repos_window_behaviour(self, pos: Vector) -> None:
        """
        Called when system window changes its position
        """
        self.pos = pos
