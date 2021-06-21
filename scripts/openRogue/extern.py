import os
import sys
import importlib
import subprocess

from openRogue.config_reader import get_config


def dependencies(*args) -> None:
    """
    Resolves pip dependencies on runtime
    """
    try:
        import pip
    except ImportError:
        # pip is ensured to be in pypy3, but you have to call ensurepip for it to be detectable
        if os.path.basename(sys.executable).split('.')[0] == "pypy3":
            subprocess.check_call([sys.executable, "-m", "ensurepip"])
        else:
            print("pip isn't present, trying to get it from get-pip.py")
            subprocess.check_call(
                [sys.executable,
                 get_config("scriptPath") + "/get-pip.py"])

    for module in args:
        try:
            importlib.import_module(module)
            # print("module {} in present".format(module))
        except ImportError:
            print("cannot find {}, installing it from pip...".format(module))
            subprocess.check_call(
                [sys.executable, "-m", "pip", "install", module])
