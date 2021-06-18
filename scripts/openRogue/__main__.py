from openRogue import signal
from openRogue import nodes
from openRogue.types import Vector

signal.impl_signal("on_exit", lambda: print("Goodbye, World!"))

main = nodes.root.init_child("main", nodes.Container, width=600, height=400)
alt = nodes.root.init_child("привет?", nodes.Container, width=200, height=200)

# add_to = alt
# for _ in range(1000):
#     add_to = add_to.init_child("привет?", nodes.Container)

# Vectors are kinda clunky?
main.size = Vector(600, 400)
# main.pos = Vector(120, 120)
main.pos.x = 480
print(main.pos)
nodes.root._loop()
