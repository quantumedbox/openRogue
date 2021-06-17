"""
"""
# from typing import Callable, Type, Class
# from weakref import ref
from typing import Any


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


# class Component:
#     """
#     Dummy class for distinguishing component classes
#     """
#     def __init__(self, base: reference):
#         self.

#     def __setattr__(self, name, value) -> None:
#         # Should we check is still reference valid ?
#         setattr(self._base(), name, value)

# def implement_component(obj: object, component: Class) -> object:
#     """
#     """
#     t = type(component.__name__, (type(obj), component), component.__dict__)
#     base = ref(obj) if not issubclass(type(obj), Component) else obj._base
#     return t(base)

# def implement_component(obj: object, component: Component) -> object:
#     """
#     Constructs new class with component as additional base and copies all data from obj instance to a new one
#     """
#     obj.__class__ = type("%s, %s" % (type(obj).__name__, component.__name__),
#                          (type(obj), component), {})
#     component.__init__(obj)
#     return obj

# class Pipe:
#     """
#     Callable ordered function queue
#     """
#     __slots__ = ("queue", )

#     # TODO Registration of changes that were made by components

#     def __init__(self, func: Callable):
#         #
#         self.queue = [func]

#     def __call__(self, *args, **kwargs) -> None:
#         for f in self.queue:
#             f(*args, **kwargs)

#     def __len__(self) -> int:
#         return len(self.queue)

#     def push_back(self, func) -> None:
#         self.queue.append(func)

#     def push_front(self, func) -> None:
#         self.queue.insert(0, func)

# def deploy_back(obj: 'subclass of Component',
#                 attr: str,
#                 func: Callable,
#                 propf="setter") -> None:
#     """
#     Helper for deploying Pipe components to object's function attribute
#     'propf' is used to describe which property functions should be piped
#     """
#     a = getattr(obj.__class__, attr)
#     # If attribute is property then we have to set new class attribute
#     # It's fine because component instances have unique constructed classes
#     if isinstance(a, property):

#         if propf == "getter":
#             if not isinstance(a.fget, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(Pipe(a.fget), a.fset, a.fdel))
#             getattr(obj.__class__, attr).fget.push_back(func)

#         elif propf == "setter":
#             if not isinstance(a.fset, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(a.fget, Pipe(a.fset), a.fdel))
#             getattr(obj.__class__, attr).fset.push_back(func)

#         elif propf == "deleter":
#             if not isinstance(a.fdel, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(a.fget, a.fset, Pipe(a.fdel)))
#             getattr(obj.__class__, attr).fdel.push_back(func)

#         else:
#             raise KeyError("%s is not a valid property attribute" % property)

#     # Functions are placed directly into the instance dict to overshadow class methods
#     else:
#         if not isinstance(a, Pipe):
#             # Bind the original method to instance that it could know about 'self'
#             bounded_func = a.__get__(obj, type(obj))
#             setattr(obj, attr, Pipe(bounded_func))
#         getattr(obj, attr).push_back(func)

# def deploy_front(obj: 'subclass of Component',
#                  attr: str,
#                  func: Callable,
#                  propf="setter") -> None:
#     """
#     Helper for deploying Pipe components to object's function attribute
#     'propf' arg is used to describe which property functions should be piped
#     """
#     a = getattr(obj.__class__, attr)
#     # If attribute is property then we have to set new class attribute
#     # It's fine because component instances have unique constructed classes
#     if isinstance(a, property):

#         if propf == "getter":
#             if not isinstance(a.fget, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(Pipe(a.fget), a.fset, a.fdel))
#             getattr(obj.__class__, attr).fget.push_front(func)

#         elif propf == "setter":
#             if not isinstance(a.fset, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(a.fget, Pipe(a.fset), a.fdel))
#             getattr(obj.__class__, attr).fset.push_front(func)

#         elif propf == "deleter":
#             if not isinstance(a.fdel, Pipe):
#                 setattr(obj.__class__, attr,
#                         property(a.fget, a.fset, Pipe(a.fdel)))
#             getattr(obj.__class__, attr).fdel.push_front(func)

#         else:
#             raise KeyError("%s is not a valid property attribute" % property)

#     # Functions are placed directly into the instance dict to overshadow class methods
#     else:
#         if not isinstance(a, Pipe):
#             # Bind the original method to instance that it could know about 'self'
#             bounded_func = a.__get__(obj, type(obj))
#             setattr(obj, attr, Pipe(bounded_func))
#         getattr(obj, attr).push_front(func)
