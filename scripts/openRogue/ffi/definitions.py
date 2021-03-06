"""
Doubled im Python C enum which is used for communicating the change of window state
"""
from enum import IntFlag

class EventType(IntFlag):
    UNKNOUN_EVENT = 0
    POINTER_EVENT = 1
    INPUT_EVENT = 2
    CLOSE_EVENT = 4
    RESIZE_EVENT = 8
    REPOS_EVENT = 16,


class WindowSignal(IntFlag):
    WINDOW_SIGNAL_CLEAR = 0
    WINDOW_SIGNAL_CLOSED = 1
    WINDOW_SIGNAL_RESIZED = 2
    WINDOW_SIGNAL_SHOWN = 4
    WINDOW_SIGNAL_HIDDEN = 8
    WINDOW_SIGNAL_EXPOSED = 16
    WINDOW_SIGNAL_MOVED = 32
    WINDOW_SIGNAL_MINIMIZED = 64
    WINDOW_SIGNAL_MAXIMIZED = 128
    WINDOW_SIGNAL_RESTORED = 256
    WINDOW_SIGNAL_MOUSE_ENTERED = 512
    WINDOW_SIGNAL_MOUSE_EXITED = 1024
    WINDOW_SIGNAL_FOCUS_GAINED = 2048
    WINDOW_SIGNAL_FOCUS_LOST = 4096


class KeyMod(IntFlag):
    KEYMOD_NONE = 0
    KEYMOD_LEFT_SHIFT = 1
    KEYMOD_RIGHT_SHIFT = 2
    KEYMOD_LEFT_CTRL = 4
    KEYMOD_RIGHT_CTRL = 8
    KEYMOD_LEFT_ALT = 16
    KEYMOD_RIGHT_ALT = 32


class InputAction(IntFlag):
    INPUT_CLEAR = 0
    INPUT_KEYDOWN = 1
    INPUT_KEYUP = 2
