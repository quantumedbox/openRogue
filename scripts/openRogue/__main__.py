# from . import extern
# from . import cjit
# from . import signal
# from . import modules
# from . import language
# from . import ffi
from . import nodes

# Обновления окон должны быть обновлениями, вызываемые из структуры
# Прямой контроль циклов стоит избегать

main = nodes.root.init_child("main", nodes.Container)

nodes.root.loop()
