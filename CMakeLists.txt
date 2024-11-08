cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(EndlessAdvanture VERSION 0.1.1 LANGUAGES C CXX)

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

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++17 -Wall")
endif()

add_executable(EndlessAdvanture ${SOURCES})

if(WIN32)
    target_link_libraries(EndlessAdvanture PRIVATE glfw3 gdi32 opengl32)
    set_target_properties(EndlessAdvanture PROPERTIES LINK_FLAGS "-Wl,-subsystem,windows")
elseif(UNIX)
    target_link_libraries(EndlessAdvanture PRIVATE glfw GL)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl -lX11 -lpthread -lXrandr -lXi")
endif()
