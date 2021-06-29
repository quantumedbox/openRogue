"""
FFI implemented through external cffi module
Works as alternative to standard ctypes that works more independently throughout python implementations (especially PyPy)
"""
from openRogue.extern import dependencies
dependencies("cffi")

import os
import cffi

# from .definitions import *
from openRogue.config_reader import get_config
from openRogue.types import Vector

from typing import Any

# Should not be here
ffi = cffi.FFI()


class FFIManager:
    """
    """
    __slots__ = ("interfaces", "current_api")

    def __init__(self):
        """
        Init new FFI manager with default api from config
        """
        self.interfaces = {}
        self.register(get_config("backend_api"), "default")
        self.current_api = "default"

    def resolve(self, name: str) -> object:
        """
        Get API handler that is register by certain name
        """
        if name in self.interfaces:
            return self.interfaces[name]
        raise KeyError(f"No registered API with the name of {name}")

    def register(self, path: str, name: str):
        """
        Register new API backend interface from 'path' that is in standart "backends" folder
        """
        delimeter = ';' if os.name == 'nt' else ':'
        extension = '.dll' if os.name == 'nt' else '.so'
        folder = os.path.abspath('backends/' + path)

        if os.path.isdir(folder):
            with open(folder + '/cdef.h', "r") as f:
                ffi.cdef(f.read())
            path_buff = os.environ["PATH"]
            os.environ["PATH"] = path_buff + delimeter + folder
            lib = ffi.dlopen(folder + '/' + path + extension)
            os.environ["PATH"] = path_buff
            self.interfaces[name] = FFIInterface(lib)
        else:
            raise NameError(f"Cannot find {path} backend")

    def swap_current_api(name: str, path=None) -> str:
        """
        Set new current API that is used for window context creation
        This function returns previously active api name to restore previous api if needed
        """
        # TODO
        if name not in self.interfaces:
            if path is not None:
                self.register(path, name)
            else:
                return self.current_api

        # ...


class FFIInterface:
    """
    """
    __slots__ = ("_shared", "close_window", "get_window_events",
                 "resize_window", "repos_window", "draw_rect", "start_drawing",
                 "finish_drawing", "set_window_icon_from_file", "resolve_font")

    def __init__(self, shared):
        self._shared = shared

        self.close_window = shared.close_window
        self.get_window_events = shared.get_window_events
        self.resize_window = shared.resize_window
        self.repos_window = shared.repos_window
        self.resolve_font = shared.resolve_font
        self.draw_rect = shared.draw_rect
        self.start_drawing = shared.start_drawing
        self.finish_drawing = shared.finish_drawing
        self.set_window_icon_from_file = shared.set_window_icon_from_file

    def init_window(self, width: int, height: int, title: str) -> int:
        return self._shared.init_window(width, height, title.encode())

    def draw_text(self,
                  font: int,
                  size: int,
                  width: int,
                  x: int,
                  y: int,
                  text: str,
                  color: int = 0xFFFFFFFF) -> None:
        self._shared.draw_text(font, size, width, x, y, text, len(text), color)

    def get_spec(self, spec: str) -> Any:
        data = self._shared.get_spec(spec.encode(encoding='ascii'))
        return eval(ffi.string(data))
