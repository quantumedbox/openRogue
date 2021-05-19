"""
Base module of openRogue runtime
Submodules:
	. cjit 		-- compile c code directly from python and use it immediately
	. extern 	-- resolve pip dependencies from python code
	. language 	-- support for localization objects that are effected by the language of system or player choice
"""
import sys

from . import modules

modules.register_module("openRogue", "0.1a")


print(sys.version)

if sys.version_info[0] != 3:
	raise RuntimeError("Given openRogue implementation requires Python 3")

print("initializing openRogue...")
