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
        output |= b << 0x8
        output |= g << 0xF
        output |= r << 0x18

        return output
