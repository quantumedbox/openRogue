"""
FFI implemented through standard ctypes
Does not really work on anything but CPython, but do not require external dependencies
"""
# TODO API swapping mechanism
# TODO Needs serious work
# For now idea is to make FFIInterface object that does the communications with shared objects
# That way we could actually have multiple APIs at the same time

# TODO Fill every argtypes and restype params

# TODO Probably isn't up to date, cffi is preffered for now

import os
from ctypes import *

from .definitions import *
from openRogue.config_reader import get_config


class C_PointerEvent(Structure):
    _fields_ = [('mouse_action', c_uint32), ('is_pressed', c_bool),
                ('x', c_int32), ('y', c_int32), ('x_motion', c_int32),
                ('y_motion', c_int32)]


class C_InputEvent(Structure):
    _fields_ = [('action', c_uint32), ('is_key_pressed', c_bool),
                ('is_key_repeat', c_bool), ('keycode', c_uint32),
                ('keymod', c_uint32)]


class C_ResizeEvent(Structure):
    _fields_ = [('width', c_int32), ('height', c_int32)]


class C_ReposEvent(Structure):
    _fields_ = [('x', c_int32), ('y', c_int32)]


class C_EventUnion(Union):
    _fields_ = [('pointer_event', C_PointerEvent),
                ('input_event', C_InputEvent), ('resize_event', C_ResizeEvent),
                ('repos_event', C_ReposEvent)]


class C_Event(Structure):
    _anonymous_ = ("_union", )
    _fields_ = [('type', c_uint32), ('_union', C_EventUnion)]


class C_EventQueue(Structure):
    _fields_ = [('events', POINTER(C_Event)), ('len', c_uint32)]


class FFIManager:
    """
    """
    __slots__ = ("interfaces", "current_api")

    def __init__(self):
        """
        Init new FFI manager with default api from config
        """
        self.interfaces = {}
        backend_api = get_config("backend_api")
        self.register(backend_api, "default")
        self.current_api = "default"

    def resolve(self, name: str) -> object:
        """
        Get API handler that is register by certain name
        """
        if name in self.interfaces:
            return self.interfaces[name]
        raise KeyError("No registered API with the name of {}".format(name))

    def register(self, path: str, name: str):
        """
        Register new API backend interface from 'path' that is in standart "backends" folder
        """
        delimeter = ';' if os.name == 'nt' else ':'
        extension = '.dll' if os.name == 'nt' else '.so'
        folder = os.path.abspath('backends/' + path)

        if not os.path.isdir(folder):
            raise NameError("Cannot find {} backend".format(path))
        path_buff = os.environ["PATH"]
        os.environ["PATH"] = path_buff + delimeter + folder
        cffi = CDLL(folder + '/' + path + extension)
        os.environ["PATH"] = path_buff
        self.interfaces[name] = FFIInterface(cffi)

    def swap_current_api(name: str, path=None) -> str:
        """
        Set new current API that is used for creating windows
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
    __slots__ = (
        "_shared",
        "close_window",
        "resize_window",
        "repos_window",
        "draw_rect",
        "start_drawing",
        "finish_drawing",
        "set_window_icon_from_file",
    )

    def __init__(self, shared):
        self._shared = shared

        shared.init_window.restype = c_uint32

        self.close_window = shared.close_window
        self.close_window.argtypes = c_uint32,

        shared.get_window_events.restype = POINTER(C_EventQueue)
        shared.get_window_events.argtypes = c_uint32,

        self.resize_window = shared.resize_window
        self.resize_window.argtypes = c_uint32, c_int, c_int

        self.repos_window = shared.repos_window
        self.repos_window.argtypes = c_uint32, c_int, c_int

        self.draw_rect = shared.draw_rect

        self.start_drawing = shared.start_drawing
        self.start_drawing.argtypes = c_uint32,

        self.finish_drawing = shared.finish_drawing

        self.set_window_icon_from_file = shared.set_window_icon_from_file

    def init_window(self, width: int, height: int, title: str) -> 'hash':
        win = self._shared.init_window(width, height, title.encode())

        return win

    def get_window_events(self, win: int):
        return self._shared.get_window_events(win).contents

    def draw_text(self,
                  font: int,
                  size: int,
                  x: int,
                  y: int,
                  text: str,
                  color: int = 0xFFFFFFFF) -> None:
        self._shared.draw_text(font, size, x, y,
                               text.encode(encoding='utf-32-le'), len(text),
                               color)
