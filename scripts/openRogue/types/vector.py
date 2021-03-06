"""
"""

# ??? Maybe just use tuples ???
# TODO Arithmetic methods (+ - * / and such)


class Vector:
    """
    """
    __slots__ = ("_x", "_y")

    def __init__(self, *args):
        if isinstance(args[0], int):
            self._x = args[0]
            self._y = args[1]
        elif hasattr(args[0], "__iter__"):
            self._x, self._y = args[0]
        else:
            raise ValueError(
                f"Cannot construct vector from given args: {args}")

    # def set_x(self, x):
    #     self._x = x

    def get_x(self):
        return self._x

    # def set_y(self, y):
    #     self._y = y

    def get_y(self):
        return self._y

    # aliases for coords
    x = property(get_x)  #, set_x)
    y = property(get_y)  #, set_y)

    # aliases for sizes
    width = property(get_x)  #, set_x)
    height = property(get_y)  #, set_y)

    def as_tuple(self) -> (int, int):
        return (self.x, self.y)

    def __str__(self) -> str:
        return f"({self._x}, {self._y})"
