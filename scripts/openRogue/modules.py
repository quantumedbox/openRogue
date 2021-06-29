"""
Centralized module resolution, registry and control
"""
from types import ModuleType


class ModuleManager:
    __slots__ = ("_modules")

    def __init__(self):
        self._modules = {}

    def register(self, module: ModuleType) -> None:
        args = {}
        for line in module.__doc__.splitlines():
            if line.startswith('@'):
                divisor = line.find('=')
                if divisor != -1:
                    args[line[1:divisor]] = eval(line[divisor + 1:])

        mod = ModuleInfo(**args)
        self._modules[module.__name__] = mod


class ModuleInfo:
    """
    Standard way of describing and getting information about modules through signals
    """

    # __slots__ = ('name', 'version', 'description', 'path')

    def __init__(self, **kwargs):
        for name, value in kwargs.items():
            setattr(self, name, value)

    def __call__(self):
        return self
