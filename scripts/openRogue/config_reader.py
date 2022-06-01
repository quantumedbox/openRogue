"""
"""
from typing import Union, Any

# TODO list all valid strings
# TODO defaults

DEFAULT_CONFIG_PATH = "./config"

DEFAULTS = {
    "language": "en_EN",
    # "backend_api": "libopenRogue_SDL",
    # "c_compiler": "./tcc/tcc",
    "script_path": "./scripts",
    "engine_module": "openRogue",
    "debug": True,
}

# Global config dictionary
# Modules should work with public functions instead of this dict directly
_configs = {}


def update_config(config=DEFAULT_CONFIG_PATH):
    global _configs
    with open(config, "r") as f:
        for l in f.readlines():
            if l.startswith("#"):
                continue
            divisor = l.find("=")
            if divisor == -1:
                continue
            _configs[l[:divisor]] = eval(l[divisor + 1:-1])


def get_config(name: str) -> Union[Any, None]:
    if name in _configs:
        return _configs[name]
    elif name in DEFAULTS:
        return DEFAULTS[name]
    else:
        return None


def has_config(name: str) -> bool:
    if name in _configs:
        return True
    return False
