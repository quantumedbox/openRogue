from . import lang
from .. config_reader import get_config, has_config

if has_config("language"):
	language.set_language(get_config("language"))
else:
	language._init_language()
