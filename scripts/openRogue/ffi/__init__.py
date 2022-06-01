import os
import sys

from .definitions import *

# Maybe call --version to get the actual information ?
# if os.path.basename(sys.executable).split('.')[0] == "pypy3":
from .implementations.cffi import *
# else:
#     from .implementations.ctypes import *

# Global manager that resolves access to shared objects
manager = FFIManager()
