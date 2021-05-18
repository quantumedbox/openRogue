import ctypes
import tempfile
import subprocess
import uuid
import os
import sys

from typing import Any, Union

# realisation of JIT C compilation by creating proxies that transform the arguments automatically

# TODO compile linux shared objects

# TODO automatic byref()
# TODO ability to pass python functions as arguments
# TODO cache tables and clearing of unutilized dlls
# TODO helper functions for working with arrays

# TODO optional implementation by CFFI instead of ctypes
# with CFFI there's no need for dlls, we could use sources as headers and link with object files

# TODO also: dynamic compilation by libtcc without dll creation


COMPILERS = ["tcc", "gcc", "cl"]

DEFALUT_DLLS_PATH = "./dlls"


# class jitFunc:
# 	"""
# 	Function handler, that transforms python args to C types automatically
# 	"""
# 	def __init__(self, func: str, argtypes: list):
# 		self.func = func
# 		# func.argtypes = argtypes

# 	def __call__(self, *args) -> Any:
# 		return self.func(*args)


def find_compiler() -> Union[str, None]:
	"""
	Helper for retrieving available compiler
	"""
	if "-cc" in sys.argv:
		return sys.argv[sys.argv.index("-cc")+1]

	# CC variable is set by loader if 'c_compiler' option is in config
	cc = os.environ.get("CC")
	if cc is not None:
		return cc

	# Search for any suitable compiler in PATH if nothing wasn't found prior
	s = os.environ.get("PATH")
	if s is not None:
		ls = s.split(";") if os.name == 'nt' else s.split(":")
		for l in ls:
			for f in (f for f in os.scandir(l) if f.is_file()):
				name = f.name.split(".")[0]
				if name in COMPILERS:
					return name
	return None


def get_execution_kwargs(compiler: str) -> {}:
	"""
	"""
	encoding = "utf-8"
	if compiler == "cl":
		encoding = "cp866"

	ex = os.path.basename(sys.executable).split('.')[0]
	if ex == "pypy3":
		if os.name == 'nt':
			# for some reason pipes are not working with pypy under windows
			return {}

	return {
		"stderr": subprocess.STDOUT,
		"stdout": subprocess.PIPE,
		"encoding": encoding,
	}


def get_compiler_specific_args(compiler: str, src: str, target: str) -> list:
	"""
	"""
	compiler = compiler[compiler.rfind('/')+1] if '/' in compiler else compiler

	if compiler == "tcc":
		return ["tcc", "-shared", "-rdynamic", "-b", "-Wall", "-O3", src, "-o", target]
	if compiler == "gcc":
		# TODO for some reason gcc output doesn't link in this case
		return ["gcc", "-x", "c", "-shared", "-Wall", "-Werror", "-O3", src, "-o", target]
	if compiler == "cl":
		# TODO force this shitty compiler to link
		return ["cl", "/Tc", src, "/LDd", "/O2", "/Oi", "/GS", "/Fe", target]

	raise Exception("Unimplemented compiler \"%s\"" % compiler)


def compile(entry: str, src: str): # -> jitFunc:
	"""
	Compiles given C source string to a DLL and then creates a python ffi handler to call given 'name' function
	"""

	# resolve path that should contain compiled dlls
	dll_path = ""
	if "-dllpath" in sys.argv:
		dll_path = sys.argv[sys.argv.index("-dllpath")+1]
	else:
		dll_path = DEFALUT_DLLS_PATH

	if not os.path.exists(os.path.abspath(dll_path)):
		os.makedirs(dll_path)

	# write src string to a temporary file
	t = tempfile.NamedTemporaryFile(mode="w", delete=False, suffix=".c")
	t.write(src)
	t.close()

	d = tempfile.NamedTemporaryFile(
		delete=False, suffix='.dll', prefix=entry+'_', dir=os.path.abspath(dll_path)
	)
	d.close()

	compiler = find_compiler()
	assert compiler is not None, """Cannot find suitable C compiler in config or path.\n
	Set 'c_compiler' option in config, pass the -cc argument with path to compiler or configure your system $PATH to include it"""

	# run compiler with specific arguments
	r = subprocess.run(
		get_compiler_specific_args(compiler, src=t.name, target=d.name),
		**get_execution_kwargs(compiler)
	)

	# clear temporary files
	os.remove(t.name)
	if r.returncode != 0:
		os.remove(d.name)
		# TODO fix inconsistencies
		if r.stdout is None:
			print("Aborted due to a compiler error")
		else:
			print("Aborted due to a compiler error:\n{}".format(r.stdout))
		exit()

	# this assumes that gcc is from mingw and gendef is present
	if compiler == "gcc" and os.name == "nt":
		subprocess.check_call(["gendef", d.name], cwd=os.path.dirname(d.name))

	# try to load compiled dll
	try:
		cdll = ctypes.CDLL(os.path.abspath(d.name))
	except OSError:
		os.remove(d.name)
		print("Cannot load DLL {}".format(os.path.abspath(d.name)))
		raise

	# return jitFunc(func=getattr(cdll, entry), argtypes=[]) #find_argtypes(entry, src))
	return getattr(cdll, entry)


if __name__ == "__main__":
	"""
	Run this file as module to test it
	"""
	func = compile(
		entry="calc_fib",
		src=r"""
		unsigned int calc_fib(unsigned int n) {
			if (n <= 1) return n;

			unsigned int prev = 0;
			unsigned int cur = 1;

			for (n -= 1; n--;) {
				unsigned int sum = prev + cur;
				prev = cur;
				cur = sum;
			}
			return cur;
		}
		"""
	)

	print(func(0))
	print(func(1))
	print(func(10))
	print(func(40))
