
# module's __init__
class MyModule(Module):
	name = "myname"
	flavor = Flavor(
		eng="myflavor",
		rus="мойвкус",
	)

impl_signal("get_modules", lambda: MyModule)


# caller's __main__
modules = []

# result of all 'get_modules' will be appended to the list
signal_dispatch("get_modules", lambda x: modules.append(x))

# is used to call without care about results
# signal("get_modules")
