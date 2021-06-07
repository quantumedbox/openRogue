"""
Integration with C backend
"""

# TODO Needs serious work
# For now idea is to make FFIInterface object that does the communications with shared objects
# That way we could actually have multiple APIs at the same time

import os
from ctypes import *

from .definitions import *
from openRogue.config_reader import get_config


class FFIManager:
    """
    """
    __slots__ = ("interfaces", )

    def __init__(self):
        """
        Init new FFI manager with default api from config
        """
        self.interfaces = {}
        backend_api = get_config("backend_api")
        self.register(backend_api, "default")

    def resolve(self, name: str) -> object:
        """
        """
        if name in self.interfaces:
            return self.interfaces[name]
        raise KeyError("No registered API with the name of {}".format(name))

    def register(self, path: str, name: str):
        """
        Register new API backend interface from 'path' that is in standart "backends" folder
        """

        # TODO Maybe just use ctypes.util.find_library ?

        shared_object_format = '.dll' if os.name == 'nt' else '.so'

        if not os.path.isfile('./backends/' + path + shared_object_format):
            if os.path.isfile('./backends/lib' + path + shared_object_format):
                path = 'lib' + path
            else:
                raise NameError("Cannot find {} backend".format(path))

        full_path = os.path.abspath('./backends/' + path)
        cffi = CDLL(full_path)
        self.interfaces[name] = FFIInterface(cffi)


class FFIInterface:
    """
    """
    __slots__ = (
        "_shared",
        "close_window",
        "process_window",
        "resize_window",
        "repos_window",
        "draw_text",
        "start_drawing",
        "finish_drawing",
    )

    def __init__(self, shared):
        self._shared = shared

        shared.init_window.restype = c_uint32

        self.close_window = shared.close_window
        self.close_window.argtypes = c_uint32,

        self.process_window = shared.process_window
        self.process_window.restype = POINTER(C_EventQueue)
        self.process_window.argtypes = c_uint32,

        self.resize_window = shared.resize_window
        self.resize_window.argtypes = c_uint32, c_int, c_int

        self.repos_window = shared.repos_window
        self.repos_window.argtypes = c_uint32, c_int, c_int

        self.draw_text = shared.draw_text

        self.start_drawing = shared.start_drawing
        self.start_drawing.argtypes = c_uint32,

        self.finish_drawing = shared.finish_drawing

        # TODO
        # shared.new_buffer_strip.argtypes = (
        #     c_size_t,
        #     c_uint32,
        #     c_int32,
        #     c_int32,
        #     POINTER(c_uint32),
        #     c_uint32,
        # )

    def init_window(self, width: int, height: int, title: str) -> 'hash':
        win = self._shared.init_window(width, height, title.encode())

        return win
