"""
Base module of openRogue runtime
Submodules:
	. cjit 		-- compile c code directly from python and use it immediately
	. extern 	-- resolve pip dependencies from python code
"""

import sys

from . import modules

modules.registry_module("openRogue", "0.1a")


print(sys.version)

if sys.version_info[0] != 3:
	raise RuntimeError("Given openRogue implementation requires Python 3")

print("openRogue initialization...")
