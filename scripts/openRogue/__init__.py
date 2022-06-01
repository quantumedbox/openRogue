"""
Base module of openRogue runtime
By importing it you should be able to access all its features
"""
import sys

from openRogue import modules
from openRogue import config_reader

print(sys.version)

config_reader.update_config()

if sys.version_info[0] != 3:
    raise Error("Given openRogue implementation requires Python 3")

print("""Initializing openRogue...
Press Ctrl+C to stop execution if needed""")

module_manager = modules.ModuleManager()

# Provide public access from openRogue import
from openRogue.nodes import root
from openRogue.types import *
from openRogue.utils import *
