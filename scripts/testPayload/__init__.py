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
                       rogue.nodes.PanelContainer,
                       x=1,
                       y=1,
                       width=16,
                       height=8)

nest2 = main.init_child("test2",
                        rogue.nodes.PanelContainer,
                        x=20,
                        y=6,
                        width=24,
                        height=20,
                        style_attrs={
                            "bg_color": rogue.Color(255, 0, 0, 255),
                        })

text = nest.init_child("text", rogue.nodes.Label)

text.text = """
AAAAAAAAAAAAAAAAAAAAAAA Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ???? Что насчёт этого? Huh. ????
"""

text2 = nest2.init_child("text", rogue.nodes.Label)

text2.text = """
Lorem ipsum dolor sit amet, consectetur adipiscing elit. In aliquet, leo a tincidunt rhoncus,
это текст-"рыба", часто используемый в печати и вэб-дизайне. Lorem Ipsum является стандартной "рыбой" для текстов на латинице с начала XVI века.
"""

sine = rogue.types.Sine(1.0, 1.0)


def sine_func(self, delta) -> None:
    self.pos = rogue.types.Vector(self.pos.x, sine.step(delta))


rogue.override_method(nest2, 'update', sine_func)
