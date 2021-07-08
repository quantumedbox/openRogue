import math


class Sine:
    __slots__ = ("end", "speed", "_current")

    def __init__(self, end=1.0, speed=1.0) -> None:
        self.end = float(end)
        self.speed = float(speed)
        self._current = 0.0

    def step(self, delta) -> float:
        self._current += delta * self.speed
        if self._current > 1.0:
            self._current -= 1.0
        return self.end * math.sin(math.pi * 2 * self._current)
