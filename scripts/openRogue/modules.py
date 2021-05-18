import inspect
import os

from . import signal

from types import ModuleType

# Module for module resolution, registry and control


class ModuleInfo:
	"""
	Standard way of describing and getting information about modules through signals
	"""
	__slots__ = ('name', 'version', 'description', 'path')

	def __init__(self, name: str, version: str, description: str, path: str):
		self.name = name
		self.version = version
		self.description = description
		self.path = path

	def __call__(self):
		return self


# Maybe make the version as integer ?
def registry_module(name: str, version: str):
	"""
	Module registry functionality
	Doc string is used as description text
	! Should be called from __init__
	"""
	caller = get_caller_module()
	signal.impl_signal("module_info", ModuleInfo(name, version, caller.__doc__, caller.__file__))


def get_modules_info() -> list:
	"""
	Returns ModuleInfo list with info about all registered objects
	"""
	modules = []
	signal.signal_dispatch("module_info", lambda x: modules.append(x))
	return modules


def get_caller_module() -> ModuleType:
	"""
	Returns module from which caller function was called
	! May be a lot of possible problems with that
	"""
	return inspect.getmodule(inspect.stack()[2][0])
