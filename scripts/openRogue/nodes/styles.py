"""
Themes are the main way of implementing visuals of interfaces
They consist of special tags that correlate to colors, tile sizes, visual accents and etc.
"""
from openRogue.types import Color

# TODO Reading default theme from preference files

# If there's no concrete value for element - it should be derived from parent
DEFAULT_STYLE = {
    "common": {
        "tile_width": 24,
        "tile_height": 16,
        "font_size": 24,
    },
    "text": {
        "fg_color": Color(255, 255, 255),
        "font": "resources/fonts/FSEX300.ttf",
    },
    "panel": {
        "bg_color": Color(125, 125, 125),
    },
}


class StyleManager:
    """
    Global theme manager which is used to retrieve and register styles
    """
    __slots__ = ("styles", )

    def __init__(self):
        self.styles = {"default": DEFAULT_STYLE}

    def resolve(self, style: str) -> {}:
        if style == "":
            return self.styles["default"]
        return self.styles.get(style, self.styles["default"])
