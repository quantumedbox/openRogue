import os
import sys
import importlib

from . import module_manager

from openRogue import nodes
from openRogue import modules
from openRogue import config_reader

from openRogue.nodes import root
from openRogue.types import *

# Guaranteed base window
root.init_child("main", nodes.Container, width=600, height=400)

# Load script modules
for f in os.listdir(config_reader.get_config("script_path")):
    if not os.path.isdir(config_reader.get_config("script_path") + '/' + f):
        continue
    if f == config_reader.get_config("engine_module"):
        continue
    spec = importlib.util.find_spec(f)
    if spec is not None:
        module = importlib.util.module_from_spec(spec)
        sys.modules[f] = module
        spec.loader.exec_module(module)
        module_manager.register(module)

# Enter game logic loop
root._loop()
