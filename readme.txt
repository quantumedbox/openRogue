
DESIGN PRINCIPLES:
```
	Player and Developer Freedom
	----------------------------

	Expandability
	-------------
	Every module that follows the conventions could be added
	alongside the others, in a way similar to "mods"

	Modularity
	----------
	You have multiple stages that could be swapped if needed
	until they implement the standardized interface

	Stages:
		v User modules
		v Engine module
		. Backend API

```


HOW TO RUN:
```
	Build the 'loader' solution in ./src folder if you don't have
	./run.exe already and then just run it -
	it will try to find everything that is required

	Alternative is calling python directly:
	For that you have to specify PYTHONPATH environment variable to ./scripts folder
	and then pass those arguments to python executable: "-m openRogue"

```


DISTRIBUTION OF PROJECTS:
```
	If you want package your game for others to play you need to provide
	everything that is needed so unexperienced in software players could
	easily play your game

	For that place python distribution in ./python folder
	(If it does add a lot of data, - try to package it with pyinstaller or similar)

	Precompile backend and loader for your target system

```
