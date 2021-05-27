"""
"""
from typing import Callable, Type

# TODO Make sure that de-implementation of components is also possible


class Component:
    """
	Dummy class for distinguishing component classes
	"""
    pass


def implement_component(obj: object, component: Component) -> object:
    """
	Constructs new class with SysWindow as additional base and copies all data from obj instance to a new one
	"""
    obj.__class__ = type("%s, %s" % (type(obj).__name__, component.__name__),
                         (type(obj), component), {})
    component.__init__(obj)
    return obj


class Pipe:
    """
	Callable ordered function queue
	"""
    __slots__ = ("queue", )

    # TODO Registration of changes that were made by components

    def __init__(self, func: Callable):
        #
        self.queue = [func]

    def __call__(self, *args, **kwargs) -> None:
        for f in self.queue:
            f(*args, **kwargs)

    def __len__(self) -> int:
        return len(self.queue)

    def push_back(self, func) -> None:
        self.queue.append(func)

    def push_front(self, func) -> None:
        self.queue.insert(0, func)


def deploy_back(obj: 'subclass of Component',
                attr: str,
                func: Callable,
                propf="setter") -> None:
    """
	Helper for deploying Pipe components to object's function attribute
	Property arg is used to describe which property functions should be piped
	"""
    a = getattr(obj.__class__, attr)
    # If attribute is property then we have to set new class attribute
    # It's fine because component instances have unique constructed classes
    if isinstance(a, property):

        if propf == "getter":
            if not isinstance(a.fget, Pipe):
                setattr(obj.__class__, attr,
                        property(Pipe(a.fget), a.fset, a.fdel))
            getattr(obj.__class__, attr).fget.push_back(func)

        elif propf == "setter":
            if not isinstance(a.fset, Pipe):
                setattr(obj.__class__, attr,
                        property(a.fget, Pipe(a.fset), a.fdel))
            getattr(obj.__class__, attr).fset.push_back(func)

        elif propf == "deleter":
            if not isinstance(a.fdel, Pipe):
                setattr(obj.__class__, attr,
                        property(a.fget, a.fset, Pipe(a.fdel)))
            getattr(obj.__class__, attr).fdel.push_back(func)

        else:
            raise KeyError("%s is not a valid property attribute" % property)

    # Functions are placed directly into the instance dict to overshadow class methods
    else:
        if not isinstance(a, Pipe):
            # Bind the original method to instance that it could know about 'self'
            bounded_func = a.__get__(obj, type(obj))
            setattr(obj, attr, Pipe(bounded_func))
        getattr(obj, attr).push_back(func)


def deploy_front(obj: 'subclass of Component',
                 attr: str,
                 func: Callable,
                 propf="setter") -> None:
    """
	Helper for deploying Pipe components to object's function attribute
	Property arg is used to describe which property functions should be piped
	"""
    a = getattr(obj.__class__, attr)
    # If attribute is property then we have to set new class attribute
    # It's fine because component instances have unique constructed classes
    if isinstance(a, property):

        if propf == "getter":
            if not isinstance(a.fget, Pipe):
                setattr(obj.__class__, attr,
                        property(Pipe(a.fget), a.fset, a.fdel))
            getattr(obj.__class__, attr).fget.push_front(func)

        elif propf == "setter":
            if not isinstance(a.fset, Pipe):
                setattr(obj.__class__, attr,
                        property(a.fget, Pipe(a.fset), a.fdel))
            getattr(obj.__class__, attr).fset.push_front(func)

        elif propf == "deleter":
            if not isinstance(a.fdel, Pipe):
                setattr(obj.__class__, attr,
                        property(a.fget, a.fset, Pipe(a.fdel)))
            getattr(obj.__class__, attr).fdel.push_front(func)

        else:
            raise KeyError("%s is not a valid property attribute" % property)

    # Functions are placed directly into the instance dict to overshadow class methods
    else:
        if not isinstance(a, Pipe):
            # Bind the original method to instance that it could know about 'self'
            bounded_func = a.__get__(obj, type(obj))
            setattr(obj, attr, Pipe(bounded_func))
        getattr(obj, attr).push_front(func)
