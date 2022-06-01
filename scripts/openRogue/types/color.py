"""
"""

# TODO Access to colors from text names such as Color("black")


class Color:
    """
    Construct hex color from RGBA components
    Alpha is optional and defaulted to 255 (1.0)
    """
    def __new__(_, r, g, b, a=255) -> int:
        output = int()
        output |= a
        output |= b << 8
        output |= g << 16
        output |= r << 24

        return output
