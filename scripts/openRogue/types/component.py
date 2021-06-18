"""
"""
from typing import Any

# TODO Deletion of components
# TODO Ways of customizable component chaining

# TODO redirect __call__ to base ???


class Component:
    """
    Remarks:
    . BE CAREFUL! Use only the reference from component constructor, not the original base !
    . When setting attributes they're set to the original base and not components
    . You have to be careful and not call/get attributes from the base directly
    """
    def __init__(self, base) -> None:
        # To not trigger the __setattr__
        self.__dict__["_base"] = base

    # TODO
    # def deimpl(self, component: 'Component') -> None:

    # TODO
    # def get_component(self, 'Component') -> Union['instance of Component', None]

    @property
    def components(self) -> list:
        """Returns list of attached components
        You can also check if there's any components by: hasattr(obj, "components")
        Or just getattr(obj, "components") which will return None or []
        """
        components = [self]
        cur_base = self.__dict__.get("_base")
        while isinstance(cur_base, Component):
            components.append(cur_base)
            cur_base = cur_base.__dict__.get("_base")
        return components

    def __getattr__(self, attr: str) -> Any:
        check_attr = self.__dict__.get(attr)
        if check_attr is not None:
            return check_attr
        check_attr = type(self).__dict__.get(attr)
        if check_attr is not None:
            return check_attr
        for base in type(self).__bases__:
            check_attr = base.__dict__.get(attr)
            if check_attr is not None:
                return check_attr
        return getattr(self.__dict__.get("_base"), attr)

    def __setattr__(self, attr: str, value: Any) -> None:
        cur_base = self
        while isinstance(cur_base, Component):
            check_attr = cur_base.__dict__.get(attr)
            if check_attr is not None:
                if type(check_attr) is property:
                    check_attr.fset(cur_base, value)
                    return
            else:
                check_attr = type(cur_base).__dict__.get(attr)
                if check_attr is not None:
                    if type(check_attr) is property:
                        check_attr.fset(cur_base, value)
                        return

                for base in type(cur_base).__bases__:
                    check_attr = base.__dict__.get(attr)
                    if check_attr is not None:
                        if type(check_attr) is property:
                            check_attr.fset(cur_base, value)
                            return
            cur_base = cur_base.__dict__["_base"]
        if callable(value) and hasattr(value, "__get__"):
            value = value.__get__(cur_base, type(cur_base))
        setattr(cur_base, attr, value)

    def __delattr__(self, attr: str) -> None:
        base = self.__dict__.get("_base")
        while isinstance(base, Component):
            base = base.__dict__["_base"]
        delattr(base, attr)
