cmake_minimum_required(VERSION 3.16)

project(EndlessAdventure VERSION 0.1.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/Main.cpp
    src/BlockTicks.cpp
    src/ChunkGenerator.cpp
    src/ChunkThread.cpp
    src/DBManager.cpp
    src/GameData.cpp
    srcs/glad.c
    srcs/sqlite3.c
)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(EndlessAdventure ${SOURCES})

if(WIN32)
    target_link_libraries(EndlessAdventure PRIVATE glfw3 gdi32 opengl32)
    set_target_properties(EndlessAdventure PROPERTIES LINK_FLAGS "-Wl,-subsystem,windows")
elseif(UNIX)
    target_link_libraries(EndlessAdventure PRIVATE glfw GL dl X11 pthread Xrandr Xi)
endif()

