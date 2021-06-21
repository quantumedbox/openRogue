"""
	Testing and showcasing module showing workflow
"""
import openRogue as rogue

rogue.modules.register("testPayload", "pre-release")

rogue.root.init_child("тест", rogue.nodes.Container, width=400, height=80)
