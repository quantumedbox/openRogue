"""
Localization submodule

Usage example:

    entity_desc = lang.Bit(
        eng="my entity description",
        rus="описание сущности моей",
    )

    print(entity_desc)
    lang.set_language("run")
    print(entity_desc)

"""
import sys
import locale
import ctypes

# TODO .mo objects
# import gettext

# Current binded language locale
_LINGUA = None

# Locale options to pick from
_OPTIONS = []

# Aliases for language description
# TODO
_ALIASES = {
    "eng": "en_EN",
    "english": "en_EN",
    "rus": "ru_RU",
    "russian": "ru_RU",
}


class Bit:
    """
    String mimic
    Receives **kwargs of languages and their data in Unicode string
    """
    __slots__ = ("_data", )

    def __init__(self, **kwargs):
        self._data = {}
        for arg in kwargs:
            if arg in _ALIASES:
                self._data[_ALIASES[arg]] = kwargs[arg]
            else:
                self._data[arg] = kwargs[arg]

    def __str__(self):
        global _OPTIONS
        for opt in _OPTIONS:
            if opt in self._data:
                return self._data[opt]

        return "*no translation*"


def _init_language() -> None:
    """
    Initialize language by the system locale
    """
    set_language(get_system_locale())


def set_language(lang: str) -> None:
    """
    """
    if lang in _ALIASES:
        lang = _ALIASES[lang]

    global _LINGUA, _OPTIONS
    _LINGUA = lang
    _OPTIONS = [_LINGUA, lang]


def get_locale() -> str:
    """
    Get current binded locale
    """
    if _LINGUA is None:
        raise Exception("Language locale wasn't set")
    return _LINGUA


def get_system_locale() -> str:
    """
    """
    if sys.platform == "win32":
        windll = ctypes.windll.kernel32
        local_code = windll.GetUserDefaultUILanguage()
        return locale.windows_locale[local_code]

    else:
        return locale.getdefaultlocale()[0]
