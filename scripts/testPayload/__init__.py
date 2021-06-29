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

nest2 = test.init_child("test2",
                        rogue.nodes.Container,
                        x=20,
                        y=6,
                        width=24,
                        height=20,
                        style_attrs={
                            "bg_color": rogue.Color(255, 0, 0, 150),
                        })

text = nest.init_child(
    "text",
    rogue.nodes.Label,
    text=
    "Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ????",
    width=15,
    height=8)

text2 = nest2.init_child(
    "text",
    rogue.nodes.Label,
    text=
    "IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ???",
    width=24,
    height=20)

print(text._lines)
