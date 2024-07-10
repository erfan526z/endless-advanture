#gcc -O3 -g3 -Wall -c -o glad.o glad.c
#gcc -O3 -g3 -Wall -c -o sqlite3.o sqlite3.c
g++ -O3 -g3 -Wall -c -o BlockTicks.o BlockTicks.cpp
g++ -O3 -g3 -Wall -c -o ChunkGenerator.o ChunkGenerator.cpp
g++ -O3 -g3 -Wall -c -o ChunkThread.o ChunkThread.cpp
g++ -O3 -g3 -Wall -c -o DBManager.o DBManager.cpp
g++ -O3 -g3 -Wall -c -o GameData.o GameData.cpp
g++ -O3 -g3 -Wall -c -o Main.o Main.cpp 
g++ -o Game Main.o BlockTicks.o ChunkGenerator.o ChunkThread.o DBManager.o GameData.o glad.o sqlite3.o -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
