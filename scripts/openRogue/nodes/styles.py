"""
Themes are the main way of implementing visuals of interfaces
They consist of special tags that correlate to colors, tile sizes, visual accents and etc.
"""

# TODO Reading default theme from preference files

# If there's no concrete value for element - it should be derived from parent
DEFAULT_THEME = {
    "common": {
        "tile_width": 24,
        "tile_height": 24,
        "font_size": 24,
    },
    "text": {
        "fg_color": (255, 255, 255)
    },
    "panel": {
        "bg_color": (125, 125, 125)
    },
}


class StyleManager:
    """
    Global theme manager which is used to retrieve and register styles
    """
    __slots__ = ("styles", )

    def __init__(self):
        self.styles = {"default": DEFAULT_THEME}

    def resolve(self, style: str) -> {}:
        return self.styles.get(style, self.styles["default"])
