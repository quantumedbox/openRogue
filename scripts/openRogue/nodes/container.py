"""
Node that structures children UI nodes by its rules
"""
from openRogue.nodes import ui

# ???   What is better: having two separate classes for vert and hori containers
#       Or contain them within single class ???


class Container(ui.NodeUI):
    """
    """

    # __slots__ = ()

    def __init__(self, type="vertical", **kwargs):
        style_id_test = kwargs.get("style_id")
        if style_id_test is None:
            kwargs["style_id"] = "panel"
        super().__init__(**kwargs)

    def render(self, render_packet: dict) -> None:
        render_packet["_api"].draw_rect(
            render_packet["x_origin"] +
            (self.pos.x * render_packet["tile_width"]),
            render_packet["y_origin"] +
            (self.pos.y * render_packet["tile_height"]),
            self.size.width * render_packet["tile_width"],
            self.size.height * render_packet["tile_height"],
            render_packet["bg_color"])
