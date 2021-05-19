from . import extern
from . import cjit
from . import signal
from . import modules
from . import language
from . import ffi

w = ffi.init_window(500, 500, b"test")
g = ffi.init_window(500, 500, b"setset")

while not w.is_closed or not g.is_closed:
	if not w.is_closed:
		ffi.process_window(w)
	if not g.is_closed:
		ffi.process_window(g)


# for i in modules.get_modules_info():
# 	print(i.path)
# 	print(i.description)

# extern.depend("numpy")

# test_f = cjit.compile(
# 	entry="func",
# 	src=r"""
# 	int func(int i) {
# 		return i * i;
# 	}
# 	"""
# )

# print(test_f(2))
