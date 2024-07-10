#include "ChunkManager.h"
#include "GameData.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct WorldProperty {
	int world_year;
	int year_day;
	double day_time;

	int spawn_x;
	int spawn_y;

	WorldProperty() {
		world_year = 0;
		year_day = 0;
		day_time = 0.0;
		spawn_x = 0;
		spawn_y = 150;
	}
};

struct WorldPlayerProperty {
	int x; // X position
	int y; // Y position
	int z; // Z position
	float yaw; // View angle yaw
	float pitch; // View angle pitch
	
	float health; // Player health
	float food; // Player food
	float water; // Player water
	float energy; // Player energy

	float air;
	float fall_speed;

	WorldPlayerProperty() {
		x = z = 0;
		yaw = pitch = fall_speed = 0;
		health = 20.0f;
		food = 20.0f;
		water = 20.0f;
		energy = 20.0f;
		air = 20.0f;
		y = 100;
	}
};

class EngineWorld {
	
public:

	EngineWorld(const char* save_folder, const char* world_name = "World 1") {
		this->save_folder = save_folder;
		int len = strlen(world_name) + 1;
		this->world_name = new char[len];
		memcpy(this->world_name, world_name, len);
		last_x = last_y = last_z = 0;
		yaw = 0.0f;
		current_user = 0;
		world_property = WorldProperty();
		player_property = WorldPlayerProperty();
	}
	
	void initialize(int render_distance, int player_id, const char* seed) {
		int mmc = (render_distance * 2 + 1);
		mmc *= mmc * 3;
		player_inventory.init(36);
		terrain_manager.initialize(save_folder, world_name, mmc, render_distance, seed);
		terrain_manager_active = true;
		current_user = player_id;
		loadProperties();
		loadPlayerProperties();
	}

	void destroy() {
		savePlayerProperties();
		saveProperties();
		terrain_manager.destroy();
		delete[] world_name;
	}
	
	void updateCurrentPosition(int x, int y, int z, float yaw = 0.0f) {
		last_x = x;
		last_y = y;
		last_z = z;
		this->yaw = yaw;
		if(terrain_manager_active)
			terrain_manager.updatePlayer(x, y, z, yaw);
	}
	
	void updateTerrain(ChunkTimeStamp now) {
		if(terrain_manager_active)
			terrain_manager.update(now);
	}

	void processBlockTicks(ChunkTimeStamp now) {
		if (terrain_manager_active)
			terrain_manager.processBlockTicks(now);
	}
	
	unsigned short int getBlock(int x, int y, int z) {
		unsigned short int block = 0;
		terrain_manager.getBlock(x, y, z, block);
		return block;
	}
	
	bool getBlock(int x, int y, int z, unsigned short int& block) {
		return terrain_manager.getBlock(x, y, z, block);
	}
	
	bool setBlock(int x, int y, int z, unsigned short int block) {
		return terrain_manager.setBlock(x, y, z, block);
	}

	bool chunkPresent(int chunk_x, int chunk_z) {
		return terrain_manager.chunkExists(chunk_x, chunk_z);
	}
	
	void renderPrepare() {
		if (terrain_manager_active)
			terrain_manager.updateRenderList(last_x, last_y, last_z, yaw);
	}
	
	bool renderNextChunkInfo(unsigned int& vao, int& vbo_length, int& cx, int& cz) {
		return terrain_manager.getRenderInfoFor(vao, vbo_length, cx, cz);
	}

	ItemInventory& playerInventory() {
		return player_inventory;
	}

	WorldProperty& properties() {
		return world_property;
	}

	WorldPlayerProperty& playerProperties() {
		return player_property;
	}
	
private:

	ChunkManager terrain_manager;

	ItemInventory player_inventory;

	WorldProperty world_property;

	WorldPlayerProperty player_property;
	
	const char* save_folder;
	
	char* world_name;
	
	bool terrain_manager_active = false;
	
	int last_x, last_y, last_z;

	int current_user;
	
	float yaw;
	
	void loadProperties() {
		char path[256];
		sprintf(path, "%s%s/p00.bin", save_folder, world_name);
		FILE* fp = nullptr;
		fp = fopen(path, "rb");
		if (fp) {
			fread(&world_property, sizeof(WorldProperty), 1, fp);
			fclose(fp);
		}
	}

	void saveProperties() {
		char path[256];
		sprintf(path, "%s%s/p00.bin", save_folder, world_name);
		FILE* fp = nullptr;
		fp = fopen(path, "wb");
		if (fp) {
			fwrite(&world_property, sizeof(WorldProperty), 1, fp);
			fclose(fp);
		}
	}

	void loadPlayerProperties() {
		char path[256];
		sprintf(path, "%s%s/p0%d.bin", save_folder, world_name, current_user);
		gamedata::loadItemInventory(&player_inventory, path);
		sprintf(path, "%s%s/p1%d.bin", save_folder, world_name, current_user);
		FILE* fp = nullptr;
		fp = fopen(path, "rb");
		if (fp) {
			fread(&player_property, sizeof(WorldPlayerProperty), 1, fp);
			fclose(fp);
		}
	}

	void savePlayerProperties() {
		char path[256];
		sprintf(path, "%s%s/p0%d.bin", save_folder, world_name, current_user);
		gamedata::saveItemInventory(&player_inventory, path);
		sprintf(path, "%s%s/p1%d.bin", save_folder, world_name, current_user);
		FILE* fp = nullptr;
		fp = fopen(path, "wb");
		if (fp) {
			fwrite(&player_property, sizeof(WorldPlayerProperty), 1, fp);
			fclose(fp);
		}
	}

};
