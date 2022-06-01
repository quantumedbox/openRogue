from openRogue.nodes import node, ui

# ???   What is better: having two separate classes for vert and hori containers
#       Or contain them within single class ???


class Container(ui.NodeUI):
    """
    Node that structures children UI nodes by its rules
    """
    def __init__(self, **kwargs):
        style_id_test = kwargs.get("style_id")
        if style_id_test is None:
            kwargs["style_id"] = "panel"
        super().__init__(**kwargs)

    def attach_child(self, name: str, child: node.Node) -> node.Node:
        ret = super().attach_child(name, child)
        self.recompose()
        return ret

    def recompose(self) -> None:
        # TODO we need to have some way of controlling and preventing infinite recompose calls
        needed_width = 0
        needed_height = 0
        for child in self._children.values():
            needed_width += child.size.width
            if child.size.height > needed_height:
                needed_height = child.size.height


class PanelContainer(Container):
    """
    Renderable container
    """
    def render(self, render_packet: dict) -> None:
        render_packet["_api"].draw_rect(
            int(render_packet["x_origin"] +
                (self.pos.x * render_packet["tile_width"])),
            int(render_packet["y_origin"] +
                (self.pos.y * render_packet["tile_height"])),
            int(self.size.width * render_packet["tile_width"]),
            int(self.size.height * render_packet["tile_height"]),
            render_packet["bg_color"])
