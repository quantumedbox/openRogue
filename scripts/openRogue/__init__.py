"""
    Base module of openRogue runtime
    By importing it you could access all features
"""
import os
import sys
import importlib

from openRogue import modules
from openRogue import config_reader

modules.register("openRogue", "0.1a")

print(sys.version)

config_reader.update_config()

if sys.version_info[0] != 3:
    raise RuntimeError("Given openRogue implementation requires Python 3")

print("""initializing openRogue...
press Ctrl+C to stop execution if needed""")

# Public access from openRogue import
from openRogue import nodes
from openRogue.nodes import root

# Base window that is guaranteed
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

# Enter game loop
root._loop()
