"""
Window node implementation

openRogue has a rather unique understanding of windows, -
they are base game objects that do have standard interfaces as every other object in the scene
"""
from openRogue import ffi
from openRogue.nodes import ui, node, container, style_manager
from openRogue.types import Vector

# TODO Random roguelike style window titles on default

# TODO Support all kinds of input events

# TODO Provide recursion depth in the render_package


class WindowContainer(container.PanelContainer):
    """
    Special NodeUI container that is used as way of integrating system and openRogue node trees 
    """
    __slots__ = ("_api", "_window_id", "_max_tile_size", "_name")

    def __init__(self, name="unnamed", width=320, height=240, *args, **kwargs):
        # Specific for this window API
        # TODO Ability to change desired API on creation or after
        # POSSIBLE SOLUTION: Setting global API state machine:
        #   ffi.bind_window_creation_api("curses")
        super().__init__(*args, width=width, height=height, **kwargs)

        self._api = ffi.manager.resolve("default")

        self._window_id = self._api.init_window(self.size.width,
                                                self.size.height, self.name)

        self._max_tile_size = Vector(self._api.get_spec("max_tile_size"))

        self._api.set_window_icon_from_file(self._window_id,
                                            b"resources/images/icon.png")

        self.name = name

    def update(self, event_packet):
        super().update(event_packet)

        event_queue = self._api.get_window_events(self._window_id)

        for i in range(event_queue.len):
            event = event_queue.events[i]
            if event.type == ffi.EventType.CLOSE_EVENT:
                self.close_window()

            elif event.type == ffi.EventType.RESIZE_EVENT:
                self.resize_window(
                    Vector(event.resize_event.width,
                           event.resize_event.height))

            elif event.type == ffi.EventType.REPOS_EVENT:
                self.repos_window(
                    Vector(event.repos_event.x, event.repos_event.y))

    def recieve_event(self, event_type, event_packet) -> None:
        super().recieve_event(event_type, event_packet)
        if event_type == "update":
            self.update(event_packet)
            self._drawing_cycle()

    # def queue_free(self) -> None:
    #     node.Node._freeing_queue.append(self)

    def free(self):
        super().free()
        # Prevent double free after force deletion
        if self._window_id is not None:
            print(f"{self.name}: closed")
            self._api.close_window(self._window_id)
            self._window_id = None

    def get_size(self):
        return super().size

    def set_size(self, value: Vector):
        container.Container.size = value
        if getattr(self, "_window_id", None) is not None:
            self._api.resize_window(self._window_id, value.width, value.height)

    size = property(get_size, set_size)

    def get_pos(self):
        return super().pos

    def set_pos(self, value: Vector):
        container.Container.pos = value
        if getattr(self, "_window_id", None) is not None:
            self._api.repos_window(self._window_id, value.x, value.y)

    pos = property(get_pos, set_pos)

    def get_name(self):
        return self._name

    def set_name(self, title: str):
        self._name = title
        if getattr(self, "_api", False):
            self._api.set_window_title(self._window_id, title)

    name = property(get_name, set_name)

    def _drawing_cycle(self) -> None:
        """
        """
        init_render_package = {
            "_api": self._api,
            "_max_tile_size": self._max_tile_size,
            "x_origin": -super().pos.x,
            "y_origin": -super().pos.y,
        }

        self._api.start_drawing(self._window_id)

        # TODO Styles are rather costly now which isn't ideal

        stack = []

        # Node to which window is attached is a special case:
        style = style_manager.resolve(super().style)

        if style.get("common", None) is not None:
            init_render_package.update(style["common"].items())
        if style.get(super().style_id, None) is not None:
            init_render_package.update(style[super().style_id].items())
        init_render_package.update(super().style_attrs.items())

        # Render it as each tile is equal to a single pixel
        unit_render_package = init_render_package.copy()
        unit_render_package.update([("tile_width", 1), ("tile_height", 1)])
        super().render(unit_render_package)

        init_render_package["x_origin"] += super().pos.x
        init_render_package["y_origin"] += super().pos.y

        for _, child in reversed(super()._children.items()):
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
    # Special window callbacks that are called on specific window events
    # They could overrided if needed

    def close_window(self) -> None:
        """
        Called when system window receives close event
        By default it deletes the object from its parent and thus leaves the tree
        """
        # self._parent().free_child(self.name)
        self.queue_free()

    def resize_window(self, size: Vector) -> None:
        """
        Called when system window changes its size
        """
        self.size = size

    def repos_window(self, pos: Vector) -> None:
        """
        Called when system window changes its position
        """
        self.pos = pos
