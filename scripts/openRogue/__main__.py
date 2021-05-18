from . import extern
from . import cjit
from . import signal
from . import modules

for i in modules.get_modules_info():
	print(i.path)
	print(i.description)

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
