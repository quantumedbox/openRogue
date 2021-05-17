# 'loot' module
class Loot:
	impl = []

	def __init__(self):
		Loot.impl.append(self)

openRogue.newType('loot')
# openRogue.typeImpl()

func = openRogue.C.make_func(r"""
	int func() {
		return 0;
	}
	""")
