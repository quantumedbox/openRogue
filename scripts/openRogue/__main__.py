from . import signal
from . import nodes
from .types import Vector

signal.impl_signal("on_exit", lambda: print("Goodbye, World!"))

main = nodes.root.init_child("main", nodes.Container)
alt = nodes.root.init_child("привет?", nodes.Container)

# Vectors are kinda clunky?
main.size = Vector(600, 400)
main.pos = Vector(120, 120)

nodes.root.loop()
