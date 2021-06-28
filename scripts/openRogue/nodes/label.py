from openRogue.nodes import ui


class Label(ui.NodeUI):
    """
    Text rendering and fitting node
    """
    def __init__(self, text="", **kwargs):
        style_id_test = kwargs.get("style_id")
        if style_id_test is None:
            kwargs["style_id"] = "text"
        self._text = text
        super().__init__(**kwargs)

    @property
    def text(self) -> str:
        return self._text

    @text.setter
    def text(self, text: str) -> None:
        self._text = text
        self.recompose()

    def recompose(self) -> None:
        """
        Restructures text to fit within node boundaries
        """
        width, height = self.size.as_tuple()

        cur_line = ""

        cur_x = 0
        cur_y = 0

        self._lines = []

        # TODO We should consider how to split the words to wrap them more naturally
        # TODO Division of very long words
        for word in self.text.split():
            word_len = len(word)
            if cur_x > word_len + width and not cur_x == 0:
                self._lines.append(cur_line)
                cur_line = ""
                cur_y += 1
                if cur_y > height:
                    break
                cur_x = 0
            cur_line += word + ' '
            cur_x += len(word)
        self._lines.append(cur_line)

    def render(self, render_packet: dict) -> None:
        for n, cur_line in enumerate(self._lines):
            render_packet["_api"].draw_text(
                render_packet["_api"].resolve_font(
                    render_packet["font"].encode()),
                render_packet["tile_height"], render_packet["x_origin"] +
                (self.pos.x * render_packet["tile_width"]),
                render_packet["y_origin"] +
                ((self.pos.y + n) * render_packet["tile_height"]), cur_line,
                render_packet["fg_color"])
