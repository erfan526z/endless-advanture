# Endless Advanture

## What is this?

This is a simple voxel game inspired by Minecraft which has basic elements like block breaking and placement, crafting, storages, and infinite terrain generation. At top of these features, it also has a weather and season system which makes the game environment more dynamic.

## Before going further

__This project is done just for learning purposes and I didn't have the time for cleaning the code and documenting.__

As I was still learning to work with OpenGL and first C++ project which is more than 5k lines of code, the code is messy and probably contains a lot of bugs and stuff. So, keep that in mind.

Also I don't have plans to go any further in this project so this is the first and probably the last version. Maybe I created a more organized project as a new repo and continue on that with more features and better documentation.

## How to run

The game is compiled and ran successfully in Ubuntu 22.04.3 LTS but it might run on other platforms by some little changes.

### Dependencies

_This section might get updated_

Libraries used:
- glfw and glad: used for handling OpenGL.
- glm: used for maths.
- FastNoiseLight: terrain generation
- Sqlite: user information (I didn't like to use a database like that, it's just for practice :D)
- stb_image: used for loading textures

```bash
sudo apt-get update
```

Installing dependecies:
```bash
sudo apt-get install libglfw3 libglfw3-dev libglm-dev
```

If you don't have GNU GCC compiler installed:
```bash
sudp apt-install gcc g++
```

### Get

Clone this repo or download the zip file manualy.

### Compile

There are two _.sh_ bash files

1. compile.sh: This bash file compiles and links the entire project, you need to use this first time you compile.

2. compile-onlygame.sh: This bash file only complies the game source files and uses _glad.o_ and _sqlite3.o_ which were created once by complie.sh, recuding compile time a little.

```bash
./compile.sh
```
Or
```bash
./compile-onlygame.sh
```

Note: If you get permission denied error when trying to execute bash files you might need to give the executable permission to each of these, this is how:
```
chmod +x compile.sh
chmod +x compile-onlygame.sh
```

If that was successfull, a file called _Game_ should appear.

### Run & Notes

The Game is executable and can be run via command or just double clicking if you use an OS like Ubuntu.

Some notes:

1. If the game opens but it shows a blank page and closes (or remain open), it probably is because assets are missing. Make sure that assets folder is present and asset files are inside it.

2. You need OpenGL 3.3+ to run the game. If you don't have that supported on your GPU, it fails to create the window. I didn't test it on a old PC to see what error it gives, but keep that in mind.

3. The game might run and get _Make sure 'data' folder exists beside the executable_ error. in this case you need to manually creaate a folder with the name 'data' beside the Game executable.

## Playing

_This section might get updated_

### Users
There is a basic user management system. You can create a new user or login with existing ones. Users can have shared worlds, in which the world is the same but the players can open the world individually and play as different characters (Not multiplayer because players can't play at the same time, but that seems a little similar).

### Gameplay
Keybinds:
- Move Forward: W
- Move Backwards: S
- Move Left: A
- Move Right: D
- Jump / Swim up: Space
- Sprint: Left Control
- Select hotbar items: 0 to 9 keys at top of the keyboard
- Open / Close Inventory: E
- Pause / Unpause: Escape

Mouse in game:
- Look around: Mouse movement
- Destroy block: Left-Click hold
- Place block, interact with block, eat or drink: Right-Click

Mouse in inventory:
- Select an item with left-click, then you can move one single item to another inventory by right-click or move the entire stack by left-click

Goals:
The game doesn't have a story, you just need to survive. Gather food from bushes by right-clicking, drink water from the water sources to survive and heal, Build whatever you want, Explore.

## Thanks To

- learnopengl.com
- Every single individual or complany whom contributed to libraries used in the project.
- Notch (Creator of a masterpiece called Minecraft)

## Licence 

_This is a small project, but anyways :D_
_This section will get updated to add github links_

This project is using GNU GPLv3, check LICENSE for more information.

Used libraries are listed on Dependencies section.
These files in the project root folder are from others and not owned by me:
- glad.c
- glad.h
- FastNoiseLite.h
- sqlite3.c
- sqlite3.h
- sqlite3ext.h
- khrplatform.h

