# from . import extern
# from . import cjit
# from . import signal
# from . import modules
# from . import language
# from . import ffi
from . import nodes
from . types import Vector

main = nodes.root.init_child("main", nodes.Container)
alt = nodes.root.init_child("привет?", nodes.Container)

main.size = Vector(600, 400)
main.pos = Vector(120, 120)

# alt = nodes.root.init_child("alt", nodes.Container)

# alt.size = Vector(64, 64)
# alt.pos = Vector(320, 240)

nodes.root.loop()
