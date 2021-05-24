"""
Doubled im Python C enum which is used for communicating the change of window state
"""
from enum import IntFlag
from ctypes import *


class C_PointerEvent(Structure):
	_fields_ = [('mouse_action', c_uint32),
				('is_pressed', c_bool),
				('x', c_int32),
				('y', c_int32),
				('x_motion', c_int32),
				('y_motion', c_int32)]


# class C_CloseEvent(Structure):
# 	_fields_ = []


class C_InputEvent(Structure):
	_fields_ = [('action', c_uint32),
				('is_key_pressed', c_bool),
				('is_key_repeat', c_bool),
				('keycode', c_uint32),
				('keymod', c_uint32)]


class C_ResizeEvent(Structure):
	_fields_ = [('width', c_int32),
				('height', c_int32)]


class C_ReposEvent(Structure):
	_fields_ = [('x', c_int32),
				('y', c_int32)]


class C_EventUnion(Union):
	_fields_ = [('pointer_event', C_PointerEvent),
				('input_event', C_InputEvent),
				# ('close_event', C_CloseEvent),
				('resize_event', C_ResizeEvent),
				('repos_event', C_ReposEvent)]


class C_Event(Structure):
	_anonymous_ = ("_union",)
	_fields_ = [('type', c_uint32),
				('timestamp', c_uint32),
				('_union', C_EventUnion)]


class C_EventQueue(Structure):
	_fields_ = [('events', POINTER(C_Event)),
				('len', c_uint32)]
				# ('window_signals', c_uint32)]


class EventType(IntFlag):
	UNKNOUN_EVENT	= 0
	POINTER_EVENT 	= 1
	INPUT_EVENT 	= 2
	CLOSE_EVENT		= 4
	RESIZE_EVENT	= 8
	REPOS_EVENT		= 16,


class WindowSignal(IntFlag):
	WINDOW_SIGNAL_CLEAR			= 0
	WINDOW_SIGNAL_CLOSED 		= 1
	WINDOW_SIGNAL_RESIZED		= 2
	WINDOW_SIGNAL_SHOWN 		= 4
	WINDOW_SIGNAL_HIDDEN		= 8
	WINDOW_SIGNAL_EXPOSED 		= 16
	WINDOW_SIGNAL_MOVED 		= 32
	WINDOW_SIGNAL_MINIMIZED 	= 64
	WINDOW_SIGNAL_MAXIMIZED 	= 128
	WINDOW_SIGNAL_RESTORED 		= 256
	WINDOW_SIGNAL_MOUSE_ENTERED = 512
	WINDOW_SIGNAL_MOUSE_EXITED 	= 1024
	WINDOW_SIGNAL_FOCUS_GAINED 	= 2048
	WINDOW_SIGNAL_FOCUS_LOST 	= 4096


class KeyMod(IntFlag):
	KEYMOD_NONE			= 0
	KEYMOD_LEFT_SHIFT	= 1
	KEYMOD_RIGHT_SHIFT	= 2
	KEYMOD_LEFT_CTRL	= 4
	KEYMOD_RIGHT_CTRL	= 8
	KEYMOD_LEFT_ALT		= 16
	KEYMOD_RIGHT_ALT	= 32


class InputAction(IntFlag):
	INPUT_CLEAR		= 0
	INPUT_KEYDOWN	= 1
	INPUT_KEYUP		= 2
