"""
#This is comment, all lines that begin with '#' are considered to be comments
#All lines that begin with '@' are module attributes that will be evaluated
@description="Testing and showcasing module showing workflow"
@version=0.0000000001
@authors=["nobody", "everybody"]
"""
import openRogue as rogue

main = rogue.root.get_child("main")

nest = main.init_child("test",
                       rogue.nodes.Container,
                       x=1,
                       y=1,
                       width=16,
                       height=8)

nest2 = main.init_child("test2",
                        rogue.nodes.Container,
                        x=20,
                        y=6,
                        width=24,
                        height=20,
                        style_attrs={
                            "bg_color": rogue.Color(255, 0, 0, 255),
                        })

text = nest.init_child(
    "text",
    rogue.nodes.Label,
    text=
    "AAAAAAAAAAAAAAAAAAAAAAA Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ????"
)

text2 = nest2.init_child(
    "text",
    rogue.nodes.Label,
    text=
    "IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ??? IT IS KINDA SUS ???",
)

rogue.root.init_child("huh!", rogue.nodes.Label, text="Cool!")
