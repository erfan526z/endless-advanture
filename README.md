# Endless Advanture

## Overview

Endless Advanture is a simple voxel game inspired by Minecraft, featuring block breaking and placement, crafting, storages, infinite terrain generation, and a dynamic weather and season system. Created for learning purposes. This project is not actively developed but will receive occasional updates.

## Features
- Infinite terrain generation
- Block breaking and placement
- Crafting system
- Storages
- Weather and season system
- Dynamic blocks based on season and temperature
- User management

## Getting Started

### Requirements
The game is tested on _Ubuntu 22.04.3 LTS_ and _Microsoft Windows 10_, but it should work with other Linux distros like Ubuntu and other versions of Windows, but keep these requirements in mind:
- OpenGL 3.3+
- 64-bit operating system

### Dependencies
Source files of libraries are embeded to the project, the only thing that is needed to be installed is GLFW libraries
- **Ubuntu**: Run these commands to get the libraries
```bash
sudo apt-get update
sudo apt-get install libglfw3 libglfw3-dev
```
- **Windows** (Using MinGW): Head to _https://www.glfw.org/download_ and in the "Windows pre-compiled binaries" section, grab the 64-bit binaries (the version used in project is currently 3.4). Then extract the files and add files inside _lib-mingw-w64_ folder to your MinGW libraries.
 
### Libraries used
- [GLFW](https://www.glfw.org/) and GLAD (Handling OpenGL) 
- [GLM](https://github.com/g-truc/glm) (Maths)
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) (Terrain Generation)
- [SQLite](https://www.sqlite.org/) (User Management)
- [stb_image](https://github.com/nothings/stb) (Loading Textures)

### Compiling On Ubuntu

When you have downloaded the files, open the terminal in the folder the files are and then follow these steps (Needs gcc, g++, and CMake installed):
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
After that, you should have the executable. Put the _assets_ folder and executable in the same folder and it's done.

### Compiling On Windows

When you have downloaded the files, open the command prompt in the folder with files, then follow these steps (Needs MinGW and CMake installed):
```
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```
And then, you should see the .exe file. Put the _assets_ folder and executable in the same folder and it's done.
_(Personally I used TDM-GCC as compiler, you can get it here if you want: https://jmeubank.github.io/tdm-gcc/download/ )_

### Notes

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
- Notch (Creator of Minecraft)

## Licence 

_This is a small project, but anyways :D_

This project is using GNU GPLv3, check LICENSE for more information.
