"""
    Testing and showcasing module showing workflow
"""
import openRogue as rogue

# Get rid of requirement to call it manually, engine loader could get the information from __init__ doc
rogue.modules.register("testPayload", "pre-release")

test = rogue.root.init_child("тест",
                             rogue.nodes.Container,
                             width=400,
                             height=80,
                             style_attrs={"bg_color": rogue.Color(5, 5, 255)})

nest = test.init_child("test",
                       rogue.nodes.Container,
                       x=1,
                       y=1,
                       width=16,
                       height=8)

# nest2 = test.init_child("test2",
#                         rogue.nodes.Container,
#                         x=6,
#                         y=6,
#                         width=2,
#                         height=20,
#                         style_attrs={
#                             "bg_color": rogue.Color(255, 0, 0, 150),
#                         })

# nest.init_child("test",
#                 rogue.nodes.Container,
#                 x=1,
#                 y=1,
#                 width=16,
#                 height=8,
#                 style_attrs={
#                     "tile_width": 12,
#                     "tile_height": 12,
#                     "bg_color": rogue.Color(25, 65, 34, 200),
#                 })

text = nest.init_child(
    "text",
    rogue.nodes.Label,
    text=
    "longfuckingshithuh test me! i want to be tested!!! hm, does it work or not? i'm not quite sure yet",
    width=16,
    height=8)

print(text._lines)