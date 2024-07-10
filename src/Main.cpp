#include <iostream>
#include <filesystem>
#include "SettingsFile.h"
#include "EngineWorld.h"
#include "EngineGraphics.h"
#include "Player.h"
#include "Physics.h"
#include "BlockRaycast.h"
#include "ChunkGenerator.h"
#include "DBManager.h"

#define ASSET_DIR_PATH "assets/"
#define DATA_DIR_PATH "data/"

#define TEMP_BUFFER_SIZE 256

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

glm::mat4 getChunkTransform(int cx, int cz, int rel_x = 0, int rel_z = 0);

void updateCamera(PhysicalPlayer& p, Camera& c);

void generatePhysicalProperties(PhysicalPlayer p, PlayerPhysicalProperties& ppp);

void createWorldDataPath(char* target, int target_size, const char* world, const char* data_thing);

void createWorldDataPath(char* target, int target_size, int world_id, const char* data_thing);

void createBlockInventory(unsigned int block_id, int x, int y, int z, unsigned int world_id);

ItemInventory* loadBlockInventory(int x, int y, int z, unsigned int world_id);

void saveBlockInventory(int x, int y, int z, unsigned int world_id, ItemInventory* ie);

void deleteBlockInventory(unsigned int bl, int x, int y, int z, unsigned int world_id);

EngineWorld* current_world = nullptr;

bool game_running;

unsigned short int func_getBlock(int x, int y, int z) { return current_world->getBlock(x, y, z); }

bool func_isBlockLiquid(unsigned short int block) { return gamedata::blocks.indexer[block]->getID() == gamedata::blocks.water.getID(); }

void game_core();

void game_play(WorldRecord& world_record, SettingsRecord& settings_record, StatisticsRecord& statistics_record, int user_id, Camera& game_camera, Renderer& game_renderer, bool new_wolrd);

int main()
{
	game_core();

	return 0;
}

bool func_blockHasHitbox(unsigned short int blockid) {
	return gamedata::blocks.indexer[blockid]->hasCollision();
}

bool func_blockIsTouchable(unsigned short int blockid) {
	return gamedata::blocks.indexer[blockid]->isTouchable();
}

void game_play(WorldRecord& world_record, SettingsRecord& settings_record, StatisticsRecord& statistics_record, int user_id, Camera& game_camera, Renderer& game_renderer, bool new_world) {

	_aspect_ratio_updated = true; // Temporary

	char temp_buffer[TEMP_BUFFER_SIZE];
	char temp_buffer2[TEMP_BUFFER_SIZE];

	Texture land_texture(32, 32);
	land_texture.loadTexture(ASSET_DIR_PATH"land.png");

	Texture texture_player_stat(8, 8);
	texture_player_stat.loadTexture(ASSET_DIR_PATH"playerstat.png");

	Texture texture_player_inventory(2, 2);
	texture_player_inventory.loadTexture(ASSET_DIR_PATH"inventory.png");

	Texture font_texture(16, 16);
	font_texture.loadTexture(ASSET_DIR_PATH"5x7_font.png");

	Texture font_large_texture(16, 16);
	font_large_texture.loadTexture(ASSET_DIR_PATH"charmap.png");

	Texture gui_component_texture(1, 8);
	gui_component_texture.loadTexture(ASSET_DIR_PATH"button.png");

	Texture crosshair_texture(1, 1);
	crosshair_texture.loadTexture(ASSET_DIR_PATH"crosshair.png");

	Texture target_texture(1, 1);
	target_texture.loadTexture(ASSET_DIR_PATH"target.png");

	Texture texture_selected(1, 1);
	texture_selected.loadTexture(ASSET_DIR_PATH"selected.png");

	Texture texture_rain(8, 512);
	texture_rain.loadTexture(ASSET_DIR_PATH"rainfall.png");

	RawModel model = RawModel();
	model.initFromFile(ASSET_DIR_PATH"box.obj");

	RawModel model_rain = RawModel();
	model_rain.initFromFile(ASSET_DIR_PATH"rain.obj");
	
	sprintf(temp_buffer, "%d", world_record.world_id);
	EngineWorld world = EngineWorld(DATA_DIR_PATH, temp_buffer);
	world.initialize(settings_record.render_dist, user_id, world_record.world_seed);
	current_world = &world;

	// Player Setup
	bool player_update = true;
	int selected_index = 0;

	PhysicalPlayer player;
	Physics physics;
	PlayerPhysicalProperties playerphys;
	BlockRaycast blockRaycast;

	player.setPositionOn(world.playerProperties().x, world.playerProperties().y, world.playerProperties().z);
	player.setRotation(world.playerProperties().pitch, world.playerProperties().yaw);
	
	if (new_world) { // If it is new world, spawn player
		world.properties().day_time = 600.0;
		world.properties().year_day = 3;

		float t1, t2; // Just used to fill arguments
		int alt = 0;
		int iter = 0;
		while (alt < 113) {
			iter++;
			alt = generateSingleBlock(iter, 0, 0, t1, t2);
		}
		player.setPositionOn(iter, alt + 1, 0);

		world.properties().spawn_x = iter;
		world.properties().spawn_y = alt + 1;
	}

	updateCamera(player, game_camera);

	// GUI setup

	GUIScene gui_scene_debug_text;
	GUIScene gui_scene_hud;
	GUIScene gui_scene_hud_inv;
	GUIScene gui_scene_hud_storage;
	GUIScene gui_scene_hud_crafting;
	GUIScene gui_scene_pause;
	GUIScene gui_scene_death;

	GUIText gui_pause_label = GUIText(&font_large_texture, "Paused", 0, 40, 16, 0, 0, 1);
	GUIText gui_pause_continue_txt = GUIText(&font_large_texture, "Continue", 0, 0, 16, 0, 0, 1);
	GUIImage gui_pause_continue = GUIImage(&gui_component_texture, 0, 0, 256, 32, 0, 0, 1, 1);
	GUIText gui_pause_quit_txt = GUIText(&font_large_texture, "Main Menu", 0, -40, 16, 0, 0, 1);
	GUIImage gui_pause_quit = GUIImage(&gui_component_texture, 0, -40, 256, 32, 0, 0, 1, 1);

	gui_scene_pause.add(gui_pause_label);
	gui_scene_pause.add(gui_pause_continue_txt);
	gui_scene_pause.add(gui_pause_continue);
	gui_scene_pause.add(gui_pause_quit_txt);
	gui_scene_pause.add(gui_pause_quit);

	GUIText gui_death_label = GUIText(&font_large_texture, "You died!", 0, 40, 16, 0, 0, 1);
	GUIText gui_death_respawn_txt = GUIText(&font_large_texture, "Respawn", 0, 0, 16, 0, 0, 1);
	GUIImage gui_death_respawn = GUIImage(&gui_component_texture, 0, 0, 256, 32, 0, 0, 1, 1);
	GUIText gui_death_quit_txt = GUIText(&font_large_texture, "Main Menu", 0, -40, 16, 0, 0, 1);
	GUIImage gui_death_quit = GUIImage(&gui_component_texture, 0, -40, 256, 32, 0, 0, 1, 1);

	gui_scene_death.add(gui_death_label);
	gui_scene_death.add(gui_death_respawn_txt);
	gui_scene_death.add(gui_death_respawn);
	gui_scene_death.add(gui_death_quit_txt);
	gui_scene_death.add(gui_death_quit);

	sprintf(temp_buffer, "x:%d y:%d z:%d", player.getGlobalX(), player.getGlobalY(), player.getGlobalZ());
	GUIText txt_position = GUIText(&font_texture, temp_buffer, 2, 2, 8, -1, 1, 1);
	gui_scene_debug_text.add(txt_position);

	sprintf(temp_buffer, "Temperature: %.2f, Rainfall: %.0f", 0.0f, 0.0f);
	GUIText txt_climate_info = GUIText(&font_texture, temp_buffer, 2, 12, 8, -1, 1, 1);
	gui_scene_debug_text.add(txt_climate_info);

	sprintf(temp_buffer, "Day %d of %s, Time:%02d:%02d", 0, "Spring", 0, 0);
	GUIText txt_time_info = GUIText(&font_texture, temp_buffer, 2, 22, 8, -1, 1, 1);
	gui_scene_debug_text.add(txt_time_info);

	sprintf(temp_buffer, "Local: temperature: %.2f, rainfall: %.0f", 0.0f, 0.0f);
	GUIText txt_local_info = GUIText(&font_texture, temp_buffer, 2, 32, 8, -1, 1, 1);
	gui_scene_debug_text.add(txt_local_info);

	sprintf(temp_buffer, "FPS: %.1f", 0.0f);
	GUIText txt_fps_info = GUIText(&font_texture, temp_buffer, 2, 42, 8, -1, 1, 1);
	gui_scene_debug_text.add(txt_fps_info);

	GUIImage gui_cross = GUIImage(&crosshair_texture, 0, 0, 16, 16, 0, 0, 1, 0);
	gui_scene_hud.add(gui_cross);

	GUIImage gui_sitem = GUIImage(&texture_selected, 0, 0, 32, 32, 0, -1, 1, 0);

	GUIImage gui_hotbar[9];

	GUIText txt_hotbar[9];

	GUIImage gui_inventory_items[27];

	GUIText txt_inventory_items[27];

	GUIImage gui_storage_items[27];

	GUIText txt_storage_items[27];

	GUIImage gui_crafting_items[18];

	GUIText txt_crafting_items[18];

	GUIImage gui_health[10];

	GUIImage gui_food[10];

	GUIImage gui_water[10];

	GUIImage gui_energy[10];

	GUIImage gui_inventory = GUIImage(&texture_player_inventory, 0, 0, 256, 256, 0, -1, 1, 2);
	gui_scene_hud.add(gui_inventory);

	for (int i = 0; i < 10; i++) {
		gui_health[i] = GUIImage(&texture_player_stat, 8 * i - 84, 30, 8, 8, 0, -1, 1, 0);
		gui_food[i] = GUIImage(&texture_player_stat, 8 * i + 12, 30, 8, 8, 0, -1, 1, 16);
		gui_water[i] = GUIImage(&texture_player_stat, 8 * i + 12, 40, 8, 8, 0, -1, 1, 32);
		gui_energy[i] = GUIImage(&texture_player_stat, 8 * i - 84, 40, 8, 8, 0, -1, 1, 48);
		gui_scene_hud.add(gui_health[i]);
		gui_scene_hud.add(gui_food[i]);
		gui_scene_hud.add(gui_water[i]);
		gui_scene_hud.add(gui_energy[i]);
	}

	for (int i = 0; i < 9; i++) {
		unsigned int cur_texture = gamedata::TEXTURE_NULL;
		if (world.playerInventory().itemStackAt(i).getItem() != nullptr)
			cur_texture = world.playerInventory().itemStackAt(i).getItem()->getTexture();
		gui_hotbar[i] = GUIImage(&land_texture, 20 * (i - 4), 10, 16, 0, -1, 1, cur_texture);
		int item_cnt = world.playerInventory().itemStackAt(i).getCount();
		if(item_cnt)
			sprintf(temp_buffer, "%d", item_cnt);
		else
			sprintf(temp_buffer, "   ");
		txt_hotbar[i] = GUIText(&font_texture, temp_buffer, 20 * (i - 4), 10, 8, 0, -1, 1);

		gui_scene_hud.add(gui_hotbar[i]);
		gui_scene_hud.add(txt_hotbar[i]);
	}

	for (int i = 0; i < 27; i++) {
		int row = i % 9;
		int col = i / 9;
		unsigned int cur_texture = gamedata::TEXTURE_NULL;
		if (world.playerInventory().itemStackAt(i + 9).getItem() != nullptr)
			cur_texture = world.playerInventory().itemStackAt(i + 9).getItem()->getTexture();
		gui_inventory_items[i] = GUIImage(&land_texture, 20 * (row - 4), 100 - col * 20, 16, 0, -1, 1, cur_texture);
		int item_cnt = world.playerInventory().itemStackAt(i + 9).getCount();
		if (item_cnt)
			sprintf(temp_buffer, "%d", item_cnt);
		else
			sprintf(temp_buffer, "   ");
		txt_inventory_items[i] = GUIText(&font_texture, temp_buffer, 20 * (row - 4), 100 - col * 20, 8, 0, -1, 1);

		gui_scene_hud_inv.add(gui_inventory_items[i]);
		gui_scene_hud_inv.add(txt_inventory_items[i]);
	}

	sprintf(temp_buffer, "   ");
	for (int i = 0; i < 27; i++) {
		int row = i % 9;
		int col = i / 9;
		unsigned int cur_texture = gamedata::TEXTURE_NULL;
		gui_storage_items[i] = GUIImage(&land_texture, 20 * (row - 4), 168 - col * 20, 16, 0, -1, 1, cur_texture);
		int item_cnt = world.playerInventory().itemStackAt(i + 9).getCount();
		txt_storage_items[i] = GUIText(&font_texture, temp_buffer, 20 * (row - 4), 168 - col * 20, 8, 0, -1, 1);
		gui_scene_hud_storage.add(gui_storage_items[i]);
		gui_scene_hud_storage.add(txt_storage_items[i]);
	}

	for (int i = 0; i < 2; i++) {
		int xoff = -50 + i * 100;
		int i9 = i * 9;
		for (int j = 0; j < 9; j++) {
			int row = j % 3;
			int col = j / 3;
			gui_crafting_items[i9 + j] = GUIImage(&land_texture, 20 * (row - 1) + xoff, 168 - col * 20, 16, 0, -1, 1, gamedata::TEXTURE_NULL);
			txt_crafting_items[i9 + j] = GUIText(&font_texture, temp_buffer, 20 * (row - 1) + xoff, 168 - col * 20, 8, 0, -1, 1);
			gui_scene_hud_crafting.add(gui_crafting_items[i9 + j]);
			gui_scene_hud_crafting.add(txt_crafting_items[i9 + j]);
		}
	}

	gui_scene_hud.add(gui_sitem);

	gui_scene_hud.setGUIScaleForAll(settings_record.gui_scale);
	gui_scene_hud_inv.setGUIScaleForAll(settings_record.gui_scale);
	gui_scene_hud_storage.setGUIScaleForAll(settings_record.gui_scale);
	gui_scene_hud_crafting.setGUIScaleForAll(settings_record.gui_scale);
	gui_scene_debug_text.setGUIScaleForAll(settings_record.gui_scale);
	
	BasicEntity entity_selected_block = BasicEntity(&target_texture, &model);
	entity_selected_block.setScale(4.1f, 4.1f, 4.1f);
	entity_selected_block.setPosition(0.0f, -10.0f, 0.0f);

	BasicEntity entity_rain = BasicEntity(&texture_rain, &model_rain);
	entity_rain.setPosition(0.0f, 50.0, 0.0f);

	/////// LOOP stuff & itself

	int tick = 0;
	double session_play_time = 0.0;
	double start = glfwGetTime();
	double frame_start = glfwGetTime();
	float last_frame = 0.0f;
	bool holding_m = false;
	bool holding_r = false;
	int inventory_open = 0;
	int storage_open = 0;
	int crafting_open = 0;
	int autosave_10sec_interval = 6;
	int autosave_iterator = 0;

	float mouse_hold_time = 0.0f;

	int inventory_selected_index_src = -1; // 0 ~ 35: Main inventory, 36 ~ 62: Storage inventory, 62 ~ 80: Crafting items
	int inventory_selected_index_dst = -1; // 0 ~ 35: Main inventory, 36 ~ 62: Storage inventory, 62 ~ 80: Crafting items

	ItemInventory* temp_storage = nullptr;
	int temp_storage_x, temp_storage_y, temp_storage_z;

	ItemStack* crafting_items = new ItemStack[18];
	bool crafting_items_modified = false;
	bool pulled_out_of_crafting = false;
	int last_index = 0;

	bool game_paused = false;
	bool exit_request = false;
	bool esc_held = false;

	bool death_screen = false;

	ControlSwitchs last_switchs;
	last_switchs.backward = last_switchs.forward = last_switchs.left = last_switchs.right = last_switchs.jump = false;

	float sun_light = 1.0f;
	float local_temp = 1.0f;
	float local_rain = 1.0f;

	bool debug_text_on = false;
	bool key_down_f3 = false;

	while (game_running && !exit_request) {

		sun_light = calculateSunlight(world.properties().day_time);
		game_renderer.setLightLevel(sun_light);
		float pos_temp, pos_rain;
		generateSingleBlock(player.getGlobalX(), player.getGlobalY(), player.getGlobalZ(), pos_temp, pos_rain);
		double randd = getNoiseResult(15, player.getGlobalX(), player.getGlobalZ(), world.properties().day_time);
		local_temp = calculateLocalTemperature(world.properties().day_time, randd, world.properties().year_day, pos_temp, pos_rain);
		local_rain = calculateLocalRain(world.properties().day_time + world.properties().year_day * 1440.0 + world.properties().world_year * 40320.0, 
			(double)player.getGlobalX(), (double)player.getGlobalZ(), pos_rain, getNoiseGenerator(14));
		game_renderer.setWeatherLevel((local_rain + 1.0f) / 2.0f);

		// Close Request Check

		if (glfwWindowShouldClose(game_renderer.getWindow()))
			exit_request = true;

		//// Cheats
		//if (isKeyPressedOrHeld(GLFW_KEY_R)) {
		//	world.properties().day_time += 60.0 * last_frame;
		//}
		//if (isKeyPressedOrHeld(GLFW_KEY_T)) {
		//	world.properties().day_time += 2000.0 * last_frame;
		//}

		// Pause & Unpause + Pause Menu Rendering

		if (isKeyPressedOrHeld(GLFW_KEY_ESCAPE)) {
			if (!esc_held) {
				esc_held = true;
				game_paused = !game_paused;
				game_renderer.setCursorMode(game_paused ? 0 : 1);
			}
		}
		else {
			esc_held = false;
		}

		if (game_paused) {
			// Menu Logic
			float mx, my;
			game_renderer.getMouseCoordinate(mx, my);

			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (gui_pause_continue.isMouseInside(mx, my)) {
					game_paused = false;
					game_renderer.setCursorMode(1);
				}
				if (gui_pause_quit.isMouseInside(mx, my)) {
					exit_request = true;
				}
			}
			gui_pause_continue.setAtlasIndex((gui_pause_continue.isMouseInside(mx, my)) ? 0 : 1);
			gui_pause_quit.setAtlasIndex((gui_pause_quit.isMouseInside(mx, my)) ? 0 : 1);

			// Menu Render
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(gui_scene_pause);
			game_renderer.updateDisplay();

			// Frame time calc
			last_frame = (float)(glfwGetTime() - frame_start);
			frame_start = glfwGetTime();

			continue;
		}

		if (death_screen) {
			// Menu Logic
			float mx, my;
			game_renderer.getMouseCoordinate(mx, my);

			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (gui_death_respawn.isMouseInside(mx, my)) {
					// Reset Player TODO
					world.playerProperties().health = 20.0f;
					world.playerProperties().food = 20.0f;
					world.playerProperties().energy = 20.0f;
					world.playerProperties().water = 20.0f;
					death_screen = false;
					game_renderer.setCursorMode(1);
				}
				if (gui_death_quit.isMouseInside(mx, my)) {
					exit_request = true;
				}
			}
			gui_death_respawn.setAtlasIndex((gui_death_respawn.isMouseInside(mx, my)) ? 0 : 1);
			gui_death_quit.setAtlasIndex((gui_death_quit.isMouseInside(mx, my)) ? 0 : 1);

			// Menu Render
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(gui_scene_death);
			game_renderer.updateDisplay();

			// Frame time calc
			last_frame = (float)(glfwGetTime() - frame_start);
			frame_start = glfwGetTime();

			continue;
		}

		// Show/Hide Debug Text Menu

		if (isKeyPressedOrHeld(GLFW_KEY_F3)) {
			if (!key_down_f3) {
				key_down_f3 = true;
				if (debug_text_on)
					debug_text_on = false;
				else
					debug_text_on = true;
			}
		}
		else {
			key_down_f3 = false;
		}

		// Menus GUI Scale

		if (_aspect_ratio_updated) {
			int scale = 1;
			if (_cheight >= 800 && _cwidth >= 1060)
				scale = 2;
			gui_scene_pause.setGUIScaleForAll(scale);
			gui_scene_death.setGUIScaleForAll(scale);
		}

		// Player Status Check
		if (world.playerProperties().health <= 0.0f) {
			death_screen = true;
			world.playerInventory().clear();
			world.playerProperties().x = world.properties().spawn_x;
			world.playerProperties().y = world.properties().spawn_y;
			world.playerProperties().z = 0;
			player.setPositionOn(world.properties().spawn_x, world.properties().spawn_y, 0);
			game_renderer.setCursorMode(0);
			continue;
		}

		if (last_switchs.playerWantsToSprint()) {
			world.playerProperties().energy -= last_frame * 0.3f;
		}

		if (last_switchs.playerWantsToJump() || last_switchs.playerWantsToWalk() || last_switchs.playerWantsToSprint()) {
			world.playerProperties().food -= last_frame * 0.05f;
			world.playerProperties().water -= last_frame * 0.05f;
		}

		if (world.playerProperties().food <= 0.0f)
			world.playerProperties().food = 0.0f;

		if (world.playerProperties().water <= 0.0f)
			world.playerProperties().water = 0.0f;

		if (world.playerProperties().energy < -0.01f)
			world.playerProperties().energy = -0.01f;

		if (world.playerProperties().water > 10.0f && world.playerProperties().food > 10.0f) { // Player healing & Energy restoring
			if (world.playerProperties().energy < 20.0f) {
				world.playerProperties().energy += last_frame * 0.2f;
				world.playerProperties().water -= last_frame * 0.05f;
				world.playerProperties().food -= last_frame * 0.05f;
			}
			if (world.playerProperties().health < 20.0f) {
				world.playerProperties().health += last_frame;
				world.playerProperties().water -= last_frame * 0.5f;
				world.playerProperties().food -= last_frame * 0.8f;
			}
		}
	
		if (world.playerProperties().food <= 0.0f || world.playerProperties().water <= 0.0f) { // Starvation
			world.playerProperties().health -= last_frame;
			world.playerProperties().energy -= last_frame * 2.0f;
		}
		

		// Daylight & Rainfall Calculations

		world.properties().day_time += last_frame;
		if (world.properties().day_time > 1440.0) { // 1,440
			world.properties().day_time -= 1440.0;
			world.properties().year_day++;
			if (world.properties().year_day == 28) { // Four weeks, each 7 days. Spring week: 0 to 6, Summer week: 7 to 13, Fall week: 14 to 20, Winter week: 21 to 27
				world.properties().year_day = 0;
				world.properties().world_year++;
			}
		}

		// 
		if (inventory_open) {
			float mx, my;
			game_renderer.getMouseCoordinate(mx, my);
		}

		// Block pick
		if (isKeyPressedOrHeld(GLFW_KEY_1)) selected_index = 0;
		if (isKeyPressedOrHeld(GLFW_KEY_2)) selected_index = 1;
		if (isKeyPressedOrHeld(GLFW_KEY_3)) selected_index = 2;
		if (isKeyPressedOrHeld(GLFW_KEY_4)) selected_index = 3;
		if (isKeyPressedOrHeld(GLFW_KEY_5)) selected_index = 4;
		if (isKeyPressedOrHeld(GLFW_KEY_6)) selected_index = 5;
		if (isKeyPressedOrHeld(GLFW_KEY_7)) selected_index = 6;
		if (isKeyPressedOrHeld(GLFW_KEY_8)) selected_index = 7;
		if (isKeyPressedOrHeld(GLFW_KEY_9)) selected_index = 8;

		// Inventory
		pulled_out_of_crafting = false;

		if (inventory_open && isMouseLeftClickPressed()) {
			clearMouseLeftClickPressed();
			float mouse_x, mouse_y;
			game_renderer.getMouseCoordinate(mouse_x, mouse_y);
			if (storage_open) for (int i = 0; i < 27; i++) if (gui_storage_items[i].isMouseInside(mouse_x, mouse_y) && temp_storage)
				if (inventory_selected_index_src >= 0)
					inventory_selected_index_dst = i + 36;
				else if (temp_storage && temp_storage->itemStackAt(i).getCount())
					inventory_selected_index_src = i + 36;
			if (crafting_open) for (int i = 0; i < 18; i++) if (gui_crafting_items[i].isMouseInside(mouse_x, mouse_y))
				if (inventory_selected_index_src >= 0)
					inventory_selected_index_dst = i + 63;
				else if (crafting_items[i].getCount())
					inventory_selected_index_src = i + 63;
			for (int i = 0; i < 27; i++) if (gui_inventory_items[i].isMouseInside(mouse_x, mouse_y))
				if (inventory_selected_index_src >= 0)
					inventory_selected_index_dst = i + 9;
				else if (world.playerInventory().itemStackAt(i + 9).getCount())
					inventory_selected_index_src = i + 9;
			for (int i = 0; i < 9; i++) if (gui_hotbar[i].isMouseInside(mouse_x, mouse_y))
				if (inventory_selected_index_src >= 0)
					inventory_selected_index_dst = i;
				else if (world.playerInventory().itemStackAt(i).getCount())
					inventory_selected_index_src = i;

			if (inventory_selected_index_dst >= 72)
				inventory_selected_index_dst = -1;

			if (inventory_selected_index_src >= 0 && inventory_selected_index_dst >= 0) {
				int& iss = inventory_selected_index_src;
				int& isd = inventory_selected_index_dst;

				ItemStack* src = (iss < 36) ? &world.playerInventory().itemStackAt(iss) : (iss < 63) ? &temp_storage->itemStackAt(iss - 36) : &crafting_items[iss - 63];
				ItemStack* dst = (isd < 36) ? &world.playerInventory().itemStackAt(isd) : (isd < 63) ? &temp_storage->itemStackAt(isd - 36) : &crafting_items[isd - 63];

				if (dst->getCount()) {
					const Item* tmp_i = dst->getItem();
					int tmp_c = dst->getCount();
					if (iss >= 72) pulled_out_of_crafting = true;
					if ((iss >= 63 && iss < 72) || (isd >= 63 && isd < 72)) crafting_items_modified = true;
					dst->set(src->getItem(), src->getCount());
					src->set(tmp_i, tmp_c);
					if (pulled_out_of_crafting && !src->isEmpty()) {
						world.playerInventory().addItem(src->getItem(), src->getCount());
						src->clear();
					}
				}
				else {
					dst->set(src->getItem(), src->getCount());
					if (iss >= 72) pulled_out_of_crafting = true;
					if ((iss >= 63 && iss < 72) || (isd >= 63 && isd < 72)) crafting_items_modified = true;
					src->clear();
				}
				inventory_selected_index_dst = inventory_selected_index_src = -1;
			}
		}

		if (inventory_open && isMouseRightClickPressed() && inventory_selected_index_src >= 0) {
			clearMouseRightClickPressed();
			float mouse_x, mouse_y;
			game_renderer.getMouseCoordinate(mouse_x, mouse_y);
			if (storage_open) for (int i = 0; i < 27; i++)
				if (gui_storage_items[i].isMouseInside(mouse_x, mouse_y) && temp_storage)
					inventory_selected_index_dst = i + 36;
			if (crafting_open) for (int i = 0; i < 18; i++)
				if (gui_crafting_items[i].isMouseInside(mouse_x, mouse_y))
					inventory_selected_index_dst = i + 63;
			for (int i = 0; i < 27; i++)
				if (gui_inventory_items[i].isMouseInside(mouse_x, mouse_y))
					inventory_selected_index_dst = i + 9;
			for (int i = 0; i < 9; i++)
				if (gui_hotbar[i].isMouseInside(mouse_x, mouse_y))
					inventory_selected_index_dst = i;

			if (inventory_selected_index_dst >= 72)
				inventory_selected_index_dst = -1;

			if (inventory_selected_index_src >= 0 && inventory_selected_index_dst >= 0) {
				int& iss = inventory_selected_index_src;
				int& isd = inventory_selected_index_dst;

				ItemStack* src = (iss < 36) ? &world.playerInventory().itemStackAt(iss) : (iss < 63) ? &temp_storage->itemStackAt(iss - 36) : &crafting_items[iss - 63];
				ItemStack* dst = (isd < 36) ? &world.playerInventory().itemStackAt(isd) : (isd < 63) ? &temp_storage->itemStackAt(isd - 36) : &crafting_items[isd - 63];

				if (dst->getCount() && dst->getItem()->getID() == src->getItem()->getID() && dst->getCount() < dst->getItem()->getStackSize()) {
					dst->addCount();
					src->addCount(-1, true);
					if (iss >= 72) pulled_out_of_crafting = true;
					if ((iss >= 63 && iss < 72) || (isd >= 63 && isd < 72)) crafting_items_modified = true;
					if (pulled_out_of_crafting && !src->isEmpty()) {
						world.playerInventory().addItem(src->getItem(), src->getCount());
						src->clear();
					}
					if (src->isEmpty()) inventory_selected_index_src = -1;
				}
				else if (dst->isEmpty()) {
					dst->set(src->getItem(), 1);
					src->addCount(-1, true);
					if (iss >= 72) pulled_out_of_crafting = true;
					if ((iss >= 63 && iss < 72) || (isd >= 63 && isd < 72)) crafting_items_modified = true;
					if (src->isEmpty()) inventory_selected_index_src = -1;
				}
				inventory_selected_index_dst = -1;
			}
		}

		// Recipes
		if (crafting_open) {
			bool output_clear = true;
			if (crafting_items_modified) for (int i = 9; i < 18; i++)
				crafting_items[i].clear();
			for (int i = 9; i < 18; i++)
				if (crafting_items[i].getCount())
					output_clear = false;
			if (pulled_out_of_crafting) {
				gamedata::recipes.list[last_index].subInput(crafting_items);
				crafting_items_modified = true;
			}
			if (output_clear) {
				for (int i = 0; i < gamedata::recipes.len; i++) {
					if (gamedata::recipes.list[i].inputMatches(crafting_items)) {
						gamedata::recipes.list[i].fillOutput(&crafting_items[9]);
						last_index = i;
					}
				}
			}
		}

		// Looking Block
		int bx, by, bz;
		if (!inventory_open && blockRaycast.cast(player.getLocalEyePosition(), game_camera.getLookingVector(), bx, by, bz, player.getOffsetX(), player.getOffsetZ(), 5.0f, 0.01f, func_getBlock, func_blockIsTouchable)) {
			int bnx = bx - player.getOffsetX();
			int bnz = bz - player.getOffsetZ();
			entity_selected_block.setPosition(bnx + 0.5f, by + 0.5f, bnz + 0.5f);
		}
		else {
			entity_selected_block.setPosition(0.0f, -100.0f, 0.0f);
		}

		// Mining Block / Placing Block / Interacting With Block
		if (isMouseLeftClickPressed() && !inventory_open) {
			int bx, by, bz;
			if (blockRaycast.cast(player.getLocalEyePosition(), game_camera.getLookingVector(), bx, by, bz, player.getOffsetX(), player.getOffsetZ(), 5.0f, 0.01f, func_getBlock, func_blockIsTouchable)) {
				unsigned short int bl;
				bool a = world.getBlock(bx, by, bz, bl);
				float break_time = gamedata::blocks.indexer[bl]->getBreakingTime();
				if (world.playerInventory().itemStackAt(selected_index).getItem() &&
					gamedata::blocks.indexer[bl]->getMaterial() == world.playerInventory().itemStackAt(selected_index).getItem()->getToolMaterial()) break_time *= 0.3f;
				if (mouse_hold_time >= break_time) {
					if (bl != gamedata::blocks.air.getID()) {
						deleteBlockInventory(bl, bx, by, bz, world_record.world_id);
						if (world.setBlock(bx, by, bz, gamedata::blocks.air.getID()) && gamedata::blocks.indexer[bl]->getDropItem() != nullptr) {
							world.playerInventory().addItem(gamedata::blocks.indexer[bl]->getDropItem(), gamedata::blocks.indexer[bl]->getDropCount());
							statistics_record.blocks_mined += 1;
							a = world.getBlock(bx-1, by, bz, bl);
							if(bl == gamedata::blocks.water.getID()) {
								world.setBlock(bx, by, bz, gamedata::blocks.water.getID());
							}
							a = world.getBlock(bx, by, bz-1, bl);
							if(bl == gamedata::blocks.water.getID()) {
								world.setBlock(bx, by, bz, gamedata::blocks.water.getID());
							}
							a = world.getBlock(bx+1, by, bz, bl);
							if(bl == gamedata::blocks.water.getID()) {
								world.setBlock(bx, by, bz, gamedata::blocks.water.getID());
							}
							a = world.getBlock(bx, by, bz+1, bl);
							if(bl == gamedata::blocks.water.getID()) {
								world.setBlock(bx, by, bz, gamedata::blocks.water.getID());
							}
							a = world.getBlock(bx, by+1, bz, bl);
							if(bl == gamedata::blocks.water.getID()) {
								world.setBlock(bx, by, bz, gamedata::blocks.water.getID());
							}
						}
					}
					mouse_hold_time = 0.0f;
				}
				else {
					mouse_hold_time += last_frame;
				}
			}
		}
		else if (isMouseRightClickPressed() && !inventory_open) {
			clearMouseRightClickPressed();
			int bx, by, bz;
			int res = blockRaycast.cast(player.getLocalEyePosition(), game_camera.getLookingVector(), bx, by, bz, player.getOffsetX(), player.getOffsetZ(), 5.0f, 0.01f, func_getBlock, func_blockIsTouchable);
			unsigned short int bl;
			bl = world.getBlock(bx, by, bz);
			if (gamedata::blocks.indexer[bl]->isStorage()) { // Open Storage
				temp_storage = loadBlockInventory(bx, by, bz, world_record.world_id);
				temp_storage_x = bx;
				temp_storage_y = by;
				temp_storage_z = bz;
				inventory_open = true;
				storage_open = true;
				game_renderer.setCursorMode(0);
				gui_inventory.setAtlasIndex(3);
			}
			else if (gamedata::blocks.indexer[bl]->isHarvestable()) { // Harvest Block
				if (gamedata::blocks.indexer[bl]->getID() == gamedata::blocks.strawberry_bush_fruit.getID()) {
					if (world.setBlock(bx, by, bz, gamedata::blocks.strawberry_bush.getID())) {
						world.playerInventory().addItem(&gamedata::items.strawberry, 3);
					}
				}
				if (gamedata::blocks.indexer[bl]->getID() == gamedata::blocks.dwarf_blueberry_bush_fruit.getID()) {
					if (world.setBlock(bx, by, bz, gamedata::blocks.dwarf_blueberry_bush.getID())) {
						world.playerInventory().addItem(&gamedata::items.blueberry, 3);
					}
				}
				if (gamedata::blocks.indexer[bl]->getID() == gamedata::blocks.bearberry_bush_fruit.getID()) {
					if (world.setBlock(bx, by, bz, gamedata::blocks.bearberry_bush.getID())) {
						world.playerInventory().addItem(&gamedata::items.bearberry, 3);
					}
				}
			}
			else if (world.playerProperties().food < 22.0f && world.playerInventory().itemStackAt(selected_index).getItem() && world.playerInventory().itemStackAt(selected_index).getItem()->getEatValue()) {
				const Item* mitem = world.playerInventory().itemStackAt(selected_index).getItem();
				world.playerInventory().itemStackAt(selected_index).addCount(-1, true);
				world.playerProperties().food += mitem->getEatValue();
			}
			else if (world.playerProperties().water < 22.0f && world.playerInventory().itemStackAt(selected_index).getItem() == nullptr) {
				if (gamedata::blocks.indexer[bl]->getID() == gamedata::blocks.water.getID())
					world.playerProperties().water += 2.0f;
			}
			else { // Place Block
				bx = (res == 1) ? bx - 1 : (res == 2) ? bx + 1 : bx;
				by = (res == 3) ? by - 1 : (res == 4) ? by + 1 : by;
				bz = (res == 5) ? bz - 1 : (res == 6) ? bz + 1 : bz;
				bool a = world.getBlock(bx, by, bz, bl);
				const Item* mitem = world.playerInventory().itemStackAt(selected_index).getItem();
				bool placable_dst = bl == gamedata::blocks.air.getID() || gamedata::blocks.water.getID();
				if (a && placable_dst && !world.playerInventory().itemStackAt(selected_index).isEmpty() && mitem && mitem->blockIDNotNull()) {
					if (world.setBlock(bx, by, bz, world.playerInventory().itemStackAt(selected_index).getItem()->getIdOfBlock())) {
						world.playerInventory().itemStackAt(selected_index).addCount(-1, true);
						if (world.getBlock(bx, by, bz, bl)) { // Create inventory for item if can
							createBlockInventory(bl, bx, by, bz, world_record.world_id);
						}
						statistics_record.blocks_placed += 1;
					}
				}
			}
		}

		if (!isMouseLeftClickPressed() || inventory_open) mouse_hold_time = 0.0f;

		// Player Movement
		if (world.chunkPresent(player.getChunkX(), player.getChunkZ())) {
			last_switchs.set(
				isKeyPressedOrHeld(GLFW_KEY_W), isKeyPressedOrHeld(GLFW_KEY_S), // Forward, Backward
				isKeyPressedOrHeld(GLFW_KEY_A), isKeyPressedOrHeld(GLFW_KEY_D), // Left, Right
				isKeyPressedOrHeld(GLFW_KEY_SPACE), isKeyPressedOrHeld(GLFW_KEY_LEFT_CONTROL), isKeyPressedOrHeld(GLFW_KEY_LEFT_SHIFT)); // Jump, Sprint, Crouch
			if (world.playerProperties().energy <= 0.0f) last_switchs.sprint = false;
			generatePhysicalProperties(player, playerphys);
			glm::vec3 movement;
			physics.player_move(last_switchs, playerphys, last_frame, movement, func_getBlock, func_blockHasHitbox, func_isBlockLiquid);
			if (glm::length(movement) > 0.01f)
				player_update = true;
			player.move(movement.x, movement.y, movement.z);
			if (physics.ground_touch_flag) {
				physics.ground_touch_flag = false;
				float threshold = -0.145f;
				float step = 0.015f;
				float dmg = (int)((threshold - physics.ground_touch_value) / step);
				if (dmg < 0.0f) dmg = 0.0f;
				world.playerProperties().health -= dmg;
			}
		}

		updateCamera(player, game_camera);

		// Open/Close inventory
		if (isKeyPressedOrHeld(GLFW_KEY_E)) {
			if (!holding_m) {
				holding_m = true;
				if (inventory_open) {
					game_renderer.setCursorMode(1);
					gui_inventory.setAtlasIndex(2);
					inventory_open = 0;
					storage_open = 0;
					crafting_open = 0;
					if (temp_storage) {
						saveBlockInventory(temp_storage_x, temp_storage_y, temp_storage_z, world_record.world_id, temp_storage);
						delete temp_storage;
						temp_storage = nullptr;
					}
				}
				else {
					game_renderer.setCursorMode(0);
					gui_inventory.setAtlasIndex(0);
					crafting_open = 1;
					inventory_open = 1;
				}
			}
		}
		else {
			holding_m = false;
		}

		// World Update
		if (player_update) {
			player_update = false;
			world.updateCurrentPosition(player.getGlobalX(), player.getGlobalY(), player.getGlobalZ());

			sprintf(temp_buffer, "x:%d y:%d z:%d", player.getGlobalX(), player.getGlobalY(), player.getGlobalZ());
			txt_position.setText(temp_buffer);

			float t_, r_;
			generateSingleBlock(player.getGlobalX(), player.getGlobalY(), player.getGlobalZ(), t_, r_);
			sprintf(temp_buffer, "Temperature: %.2f, Rainfall: %.0f", t_, r_);
			txt_climate_info.setText(temp_buffer);
		}

		// Gui Prepration

		for (int i = 0; i < 9; i++) {
			unsigned int cur_texture = gamedata::TEXTURE_NULL;
			if (world.playerInventory().itemStackAt(i).getItem() != nullptr)
				cur_texture = world.playerInventory().itemStackAt(i).getItem()->getTexture();
			gui_hotbar[i].setAtlasIndex(cur_texture);
			int item_cnt = world.playerInventory().itemStackAt(i).getCount();
			if (item_cnt)
				sprintf(temp_buffer, "%d", item_cnt);
			else
				sprintf(temp_buffer, "   ");
			txt_hotbar[i].setText(temp_buffer);
		}

		if (inventory_open) for (int i = 0; i < 27; i++) {
			int row = i % 9;
			int col = i / 9;
			unsigned int cur_texture = gamedata::TEXTURE_NULL;
			if (world.playerInventory().itemStackAt(i + 9).getItem() != nullptr)
				cur_texture = world.playerInventory().itemStackAt(i + 9).getItem()->getTexture();
			gui_inventory_items[i].setAtlasIndex(cur_texture);
			int item_cnt = world.playerInventory().itemStackAt(i + 9).getCount();
			if (item_cnt)
				sprintf(temp_buffer, "%d", item_cnt);
			else
				sprintf(temp_buffer, "   ");
			txt_inventory_items[i].setText(temp_buffer);
		}

		if (storage_open && temp_storage) for (int i = 0; i < 27; i++) {
			unsigned int cur_texture = gamedata::TEXTURE_NULL;
			if (temp_storage->itemStackAt(i).getItem() != nullptr)
				cur_texture = temp_storage->itemStackAt(i).getItem()->getTexture();
			gui_storage_items[i].setAtlasIndex(cur_texture);
			int item_cnt = temp_storage->itemStackAt(i).getCount();
			if (item_cnt)
				sprintf(temp_buffer, "%d", item_cnt);
			else
				sprintf(temp_buffer, "   ");
			txt_storage_items[i].setText(temp_buffer);
		}

		if (crafting_open) for (int i = 0; i < 18; i++) {
			unsigned int cur_texture = gamedata::TEXTURE_NULL;
			int cnt;
			if (cnt = crafting_items[i].getCount())
				cur_texture = crafting_items[i].getItem()->getTexture();
			gui_crafting_items[i].setAtlasIndex(cur_texture);
			if (cnt)
				sprintf(temp_buffer, "%d", cnt);
			else
				sprintf(temp_buffer, "   ");
			txt_crafting_items[i].setText(temp_buffer);
		}

		for (int i = 0; i < 10; i++) {
			float pstat = world.playerProperties().food;
			if(pstat >= (float) (i * 2 + 2))
				gui_food[i].setAtlasIndex(16);
			else if(pstat >= (float) (i * 2 + 1))
				gui_food[i].setAtlasIndex(17);
			else
				gui_food[i].setAtlasIndex(18);

			pstat = world.playerProperties().health;
			if (pstat >= (float)(i * 2 + 2))
				gui_health[i].setAtlasIndex(0);
			else if (pstat >= (float)(i * 2 + 1))
				gui_health[i].setAtlasIndex(1);
			else
				gui_health[i].setAtlasIndex(2);

			pstat = world.playerProperties().water;
			if (pstat >= (float)(i * 2 + 2))
				gui_water[i].setAtlasIndex(32);
			else if (pstat >= (float)(i * 2 + 1))
				gui_water[i].setAtlasIndex(33);
			else
				gui_water[i].setAtlasIndex(34);

			pstat = world.playerProperties().energy;
			if (pstat >= (float)(i * 2 + 2))
				gui_energy[i].setAtlasIndex(48);
			else if (pstat >= (float)(i * 2 + 1))
				gui_energy[i].setAtlasIndex(49);
			else
				gui_energy[i].setAtlasIndex(50);
		}

		sprintf(temp_buffer, "Day %d of %s, Time:%02d:%02d", world.properties().year_day % 7 + 1,
			(world.properties().year_day < 7) ? "Spring" : (world.properties().year_day < 14) ? "Summer" : (world.properties().year_day < 21) ? "Fall" : "Winter",
			((int)world.properties().day_time) / 60, ((int)world.properties().day_time) % 60);
		txt_time_info.setText(temp_buffer);

		sprintf(temp_buffer, "Local: temperature: %.2f, rainfall: %s", local_temp, getDisplayTitleForRain(local_rain));
		txt_local_info.setText(temp_buffer);

		if (tick == 30) {
			tick = 0;
			sprintf(temp_buffer, "FPS: %.1f", (float)(30.0 / (glfwGetTime() - start)) );
			txt_fps_info.setText(temp_buffer);
			start = glfwGetTime();
		}

		if (inventory_open) {
			int tempx = inventory_selected_index_src % 9;
			int tempy = inventory_selected_index_src / 9;
			if (inventory_selected_index_src < 0)
				gui_sitem.setPosition(20 * (tempx - 4), -50);
			else if (tempy == 0)
				gui_sitem.setPosition(20 * (tempx - 4), 2);
			else if (tempy < 4)
				gui_sitem.setPosition(20 * (tempx - 4), 92 - (tempy - 1) * 20);
			else if (tempy < 7)
				gui_sitem.setPosition(20 * (tempx - 4), 160 - (tempy - 4) * 20);
			else {
				int cx = tempx % 3;
				int cy = tempx / 3;
				int xo = (tempy == 7) ? 0 : 100;
				gui_sitem.setPosition(20 * cx - 70 + xo, 160 - cy * 20);
			}
			
		}
		else if (selected_index < 9) {
			gui_sitem.setPosition(20 * (selected_index - 4), 2);
		}
		
		// ...
		game_renderer.prepare();

		game_renderer.prepareBackground();

		game_renderer.renderBackground();

		game_renderer.prepare3D();

		// Process Block Ticks And Environment Stuff
		if (!(tick % 3)) {
			ChunkTimeStamp now;
			now.day = world.properties().year_day;
			now.year = world.properties().world_year;
			now.time = (float) world.properties().day_time;
			world.updateTerrain(now);
			world.processBlockTicks(now);
		}

		// Rainfall
		if (local_rain > 0.0f) {
			if (local_temp > 1.0f) { // Rain
				//if (local_rain > )
				int offset = (local_rain > 0.6f) ? 3 : (local_rain > 0.4f) ? 2 : (local_rain > 0.2f) ? 1 : 0;
				int step = 64;
				int c = entity_rain.getAtlasIndex() - entity_rain.getAtlasIndex() % step;
				c = (c > 4096 - step + offset) ? offset : c + step + offset;
				entity_rain.setAtlasIndex(c);
			}
			else if (tick % 2 == 0) { // Snow
				int offset = (local_rain > 0.6f) ? 7 : (local_rain > 0.4f) ? 6 : (local_rain > 0.2f) ? 5 : 4;
				int step = 8;
				int c = entity_rain.getAtlasIndex() - entity_rain.getAtlasIndex() % step;
				c = (c > 4096 - step + offset) ? offset : c + step + offset;
				entity_rain.setAtlasIndex(c);
			}
			int pyp = player.getGlobalY();
			pyp = (pyp + 20) / 40; //		position of player:	-20~20, 20~60, 60~100, ...
			pyp *= 40; //					position of rain:	     0,    40,     80, ...
			entity_rain.setPosition(player.getLocalPosition().x, pyp, player.getLocalPosition().z);
		}
		else {
			entity_rain.setAtlasIndex(0);
		}

		// Render 3D

		world.renderPrepare();

		int cx, cz, vbo_length, first = 1;
		unsigned int vao;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		while (!world.renderNextChunkInfo(vao, vbo_length, cx, cz)) {
			game_renderer.renderChunk(&land_texture, getChunkTransform(cx, cz, player.getChunkX(), player.getChunkZ()), vbo_length, vao, first);
			first = 0;
		}
		glDisable(GL_BLEND);

		game_renderer.renderBasicEntity(entity_selected_block);
		if (local_rain > 0.0f) game_renderer.renderBasicEntity(entity_rain);

		// Render 2D
		game_renderer.prepare2D();

		game_renderer.renderGUIScene(gui_scene_hud);
		if (inventory_open) {
			if (storage_open) game_renderer.renderGUIScene(gui_scene_hud_storage);
			if (crafting_open) game_renderer.renderGUIScene(gui_scene_hud_crafting);
			game_renderer.renderGUIScene(gui_scene_hud_inv);
		}
		if (debug_text_on) {
			game_renderer.renderGUIScene(gui_scene_debug_text);
		}

		// Finialize
		game_renderer.updateDisplay();

		last_frame = (float)(glfwGetTime() - frame_start);
		frame_start = glfwGetTime();
		tick++;
		session_play_time += last_frame;
	}

	delete[] crafting_items;

	world.playerProperties().x = player.getGlobalX();
	world.playerProperties().y = player.getGlobalY();
	world.playerProperties().z = player.getGlobalZ();
	world.playerProperties().pitch = player.getPitch();
	world.playerProperties().yaw = player.getYaw();

	statistics_record.play_time += (int)session_play_time;

	world.destroy();

	land_texture.unloadTexture();
	font_texture.unloadTexture();
	crosshair_texture.unloadTexture();
	target_texture.unloadTexture();
	model.destroyModel();

	gui_scene_hud.clear();
	gui_scene_debug_text.clear();

	game_renderer.setLightLevel(1.0f);
}

glm::mat4 getChunkTransform(int cx, int cz, int rel_x, int rel_z)
{
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3((cx - rel_x) * CHUNK_SIZE, 0.0f, (cz - rel_z) * CHUNK_SIZE));
	return transform;
}

void updateCamera(PhysicalPlayer& p, Camera& c)
{
	glm::vec3 p_ = p.getLocalEyePosition();
	c.setPosition(p_.x, p_.y, p_.z);
	p.setRotation(c.getRotation().x, c.getRotation().y);
}

void generatePhysicalProperties(PhysicalPlayer p, PlayerPhysicalProperties& ppp)
{
	ppp.body_height = p.getBodyHeight();
	ppp.body_width = p.getBodyWidth();
	ppp.normal_speed = 1.1f;
	ppp.crouch_speed = 0.6f;
	ppp.sprint_speed = 2.2f;
	ppp.position = p.getLocalPosition();
	ppp.xoffset = p.getOffsetX();
	ppp.zoffset = p.getOffsetZ();
	ppp.yaw = p.getYaw();
}

void createWorldDataPath(char* target, int target_size, const char* world, const char* data_thing)
{
	sprintf(target, "%s%s/%s", DATA_DIR_PATH, world, data_thing);
}

void createWorldDataPath(char* target, int target_size, int world_id, const char* data_thing)
{
	sprintf(target, "%s%d/%s", DATA_DIR_PATH, world_id, data_thing);
}

void createBlockInventory(unsigned int block_id, int x, int y, int z, unsigned int world_id)
{
	if (gamedata::blocks.indexer[block_id]->isStorage()) {
		char path[256];
		char datathing[128];
		int cx = (x < 0) ? ((x + 1) / 16 - 1) : (x / 16);
		int cz = (z < 0) ? ((z + 1) / 16 - 1) : (z / 16);
		int ox = x - cx * 16;
		int oz = z - cz * 16;
		sprintf(datathing, "%08x%08x%01x%01x%04x.bin", cx, cz, ox, oz, y);
		createWorldDataPath(path, 256, world_id, datathing);
		ItemInventory ie = ItemInventory(27);
		gamedata::saveItemInventory(&ie, path);
	}
}

ItemInventory* loadBlockInventory(int x, int y, int z, unsigned int world_id)
{
	ItemInventory* ie = new ItemInventory(27);
	char path[256];
	char datathing[128];
	int cx = (x < 0) ? ((x + 1) / 16 - 1) : (x / 16);
	int cz = (z < 0) ? ((z + 1) / 16 - 1) : (z / 16);
	int ox = x - cx * 16;
	int oz = z - cz * 16;
	sprintf(datathing, "%08x%08x%01x%01x%04x.bin", cx, cz, ox, oz, y);
	createWorldDataPath(path, 256, world_id, datathing);
	gamedata::loadItemInventory(ie, path);
	return ie;
}

void saveBlockInventory(int x, int y, int z, unsigned int world_id, ItemInventory* ie)
{
	char path[256];
	char datathing[128];
	int cx = (x < 0) ? ((x + 1) / 16 - 1) : (x / 16);
	int cz = (z < 0) ? ((z + 1) / 16 - 1) : (z / 16);
	int ox = x - cx * 16;
	int oz = z - cz * 16;
	sprintf(datathing, "%08x%08x%01x%01x%04x.bin", cx, cz, ox, oz, y);
	createWorldDataPath(path, 256, world_id, datathing);
	gamedata::saveItemInventory(ie, path);
}

void deleteBlockInventory(unsigned int bl, int x, int y, int z, unsigned int world_id)
{
	if (gamedata::blocks.indexer[bl]->isStorage()) {
		char path[256];
		char datathing[128];
		int cx = (x < 0) ? ((x + 1) / 16 - 1) : (x / 16);
		int cz = (z < 0) ? ((z + 1) / 16 - 1) : (z / 16);
		int ox = x - cx * 16;
		int oz = z - cz * 16;
		sprintf(datathing, "%08x%08x%01x%01x%04x.bin", cx, cz, ox, oz, y);
		createWorldDataPath(path, 256, world_id, datathing);
		if (std::remove(path)); // else, the file isn't there so it's fine
	}
}

void game_core()
{
	SettingsFile settings = SettingsFile(DATA_DIR_PATH);

	// Renderer setup
	Renderer game_renderer;
	game_renderer.initializeWindow(_cwidth, _cheight, "Endless Advanture - Snapshot 0.1.1");
	game_renderer.initialize3DShader(ASSET_DIR_PATH"vshader3.glsl", ASSET_DIR_PATH"fshader3.glsl");
	game_renderer.initialize2DShader(ASSET_DIR_PATH"vshader2.glsl", ASSET_DIR_PATH"fshader2.glsl");
	game_renderer.initialize3DBackgroundShader(ASSET_DIR_PATH"vshaderbg.glsl", ASSET_DIR_PATH"fshaderbg.glsl");
	game_renderer.setCursorMode(0);

	Camera game_camera = Camera();
	game_renderer.setCurrentCamera(&game_camera);

	Texture menu_button = Texture(1, 8);
	menu_button.loadTexture(ASSET_DIR_PATH"button.png");

	Texture large_font = Texture(16, 16);
	large_font.loadTexture(ASSET_DIR_PATH"charmap.png");

	/* Menus
	0	: Nothing
	1	: Welcome
	2	: Login
	3	: New User
	4	: Main Menu
	5	: Worlds Menu
	6	: Settings Menu
	7	: Statistics Menu
	8	: New World Menu
	9	: 
	10	: Game Itself
	11	: Delete World Confirmation
	12	: Error Menu
	13	: Change Password Menu
	14	: Delete Confirm Menu
	*/
	constexpr int MENU_NULL = 0;
	constexpr int MENU_WELCOME = 1;
	constexpr int MENU_LOGIN = 2;
	constexpr int MENU_NEW_USER = 3;
	constexpr int MENU_MAIN = 4;
	constexpr int MENU_WORLDS = 5;
	constexpr int MENU_SETTINGS = 6;
	constexpr int MENU_STATISTICS = 7;
	constexpr int MENU_NEW_WORLD = 8;
	constexpr int MENU_GAME = 10;
	constexpr int MENU_DELETE_CONFIRM = 11;
	constexpr int MENU_ERROR = 12;
	constexpr int MENU_CHANGE_PASSWORD = 13;
	constexpr int MENU_DELETE_ACCOUNT = 14;

	// 
	float mouse_x, mouse_y;
	int selected_menu = MENU_NULL;
	cleanKeys();
	game_running = true;

	// Database Manager Initialize
	DataBaseManager dbmo = DataBaseManager(false);
	bool dbopen = dbmo.open(DATA_DIR_PATH"users.db");
	if (dbopen) dbmo.checkTables();

	// Background Image
	Texture background_image = Texture();
	background_image.loadTexture(ASSET_DIR_PATH"background.png");
	GUIScene backgroundScene = GUIScene();
	GUIImage backgroundImage = GUIImage(&background_image, 0, 0, 1920, 979, 0, 0, 1, 0);
	backgroundScene.add(backgroundImage);

	// Welcome Menu (MENU_WELCOME)
	GUIScene wScene = GUIScene();
	GUIText wText = GUIText(&large_font, "Welcome back!", 0, 72, 16, 0, 1, 1);
	GUIImage wLoginButton = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 1);
	GUIText wLoginButtonText = GUIText(&large_font, "Login User", 0, 158, 16, 0, 1, 1);
	GUIImage wNewButton = GUIImage(&menu_button, 0, 190, 256, 32, 0, 1, 1, 1);
	GUIText wNewButtonText = GUIText(&large_font, "New User", 0, 198, 16, 0, 1, 1);
	GUIImage wExit = GUIImage(&menu_button, 0, 230, 256, 32, 0, 1, 1, 1);
	GUIText wExitText = GUIText(&large_font, "Exit", 0, 238, 16, 0, 1, 1);
	wScene.add(wText);
	wScene.add(wLoginButton);
	wScene.add(wLoginButtonText);
	wScene.add(wNewButton);
	wScene.add(wNewButtonText);
	wScene.add(wExit);
	wScene.add(wExitText);

	// User Login Menu (MENU_LOGIN)
	char l_keyinput = 0;
	char l_username[32] = "";
	int l_user_iterrator = 0;
	char l_password[32] = "";
	int l_pass_iterrator = 0;
	int l_selected_text_field = 0;
	GUIScene lScene = GUIScene();
	GUIText lText = GUIText(&large_font, "Enter your username and password:", 0, 72, 16, 0, 1, 1);
	GUIImage lUsernameField = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 3);
	GUIText lUsernameFieldText = GUIText(&large_font, l_username, 0, 158, 16, 0, 1, 1);
	GUIImage lPasswordField = GUIImage(&menu_button, 0, 190, 256, 32, 0, 1, 1, 3);
	GUIText lPasswordFieldText = GUIText(&large_font, l_password, 0, 198, 16, 0, 1, 1);
	GUIImage lLoginButton = GUIImage(&menu_button, 0, 250, 256, 32, 0, 1, 1, 1);
	GUIText lLoginButtonText = GUIText(&large_font, "Login", 0, 258, 16, 0, 1, 1);
	GUIImage lBackButton = GUIImage(&menu_button, 0, 290, 256, 32, 0, 1, 1, 1);
	GUIText lBackButtonText = GUIText(&large_font, "Back", 0, 298, 16, 0, 1, 1);
	GUIText lMessage = GUIText(&large_font, "", 0, 330, 16, 0, 1, 1);
	lScene.add(lText);
	lScene.add(lUsernameField);
	lScene.add(lUsernameFieldText);
	lScene.add(lPasswordField);
	lScene.add(lPasswordFieldText);
	lScene.add(lLoginButton);
	lScene.add(lLoginButtonText);
	lScene.add(lBackButton);
	lScene.add(lBackButtonText);
	lScene.add(lMessage);

	// New User Menu (MENU_NEW_USER)
	char n_keyinput = 0;
	char n_username[32] = "";
	int n_user_iterrator = 0;
	char n_password[32] = "";
	int n_pass_iterrator = 0;
	int n_selected_text_field = 0;
	GUIScene nScene = GUIScene();
	GUIText nText = GUIText(&large_font, "Choose a username and password:", 0, 72, 16, 0, 1, 1);
	GUIImage nUsernameField = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 3);
	GUIText nUsernameFieldText = GUIText(&large_font, n_username, 0, 158, 16, 0, 1, 1);
	GUIImage nPasswordField = GUIImage(&menu_button, 0, 190, 256, 32, 0, 1, 1, 3);
	GUIText nPasswordFieldText = GUIText(&large_font, n_password, 0, 198, 16, 0, 1, 1);
	GUIImage nCreateButton = GUIImage(&menu_button, 0, 250, 256, 32, 0, 1, 1, 1);
	GUIText nLoginButtonText = GUIText(&large_font, "Create", 0, 258, 16, 0, 1, 1);
	GUIImage nBackButton = GUIImage(&menu_button, 0, 290, 256, 32, 0, 1, 1, 1);
	GUIText nBackButtonText = GUIText(&large_font, "Back", 0, 298, 16, 0, 1, 1);
	GUIText nMessage = GUIText(&large_font, "", 0, 330, 16, 0, 1, 1);
	nScene.add(nText);
	nScene.add(nUsernameField);
	nScene.add(nUsernameFieldText);
	nScene.add(nPasswordField);
	nScene.add(nPasswordFieldText);
	nScene.add(nCreateButton);
	nScene.add(nLoginButtonText);
	nScene.add(nBackButton);
	nScene.add(nBackButtonText);
	nScene.add(nMessage);

	// Error Menu - Database read failure (MENU_ERROR)
	GUIScene errScene = GUIScene();
	GUIText errText1 = GUIText(&large_font, "Could not open database.", 0, 72, 16, 0, 1, 1);
	GUIText errText2 = GUIText(&large_font, "Make sure \'data\' folder exists beside the executable", 0, 104, 16, 0, 1, 1);
	GUIImage errExit = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 1);
	GUIText errExitText = GUIText(&large_font, "Exit", 0, 158, 16, 0, 1, 1);
	errScene.add(errText1);
	errScene.add(errText2);
	errScene.add(errExit);
	errScene.add(errExitText);

	// Main Menu (MENU_MAIN)
	bool user_logged_in = false;
	char username[32]; // This is the username of player who currently is playing. (Logged in)
	int current_user_id = 0;
	std::vector<WorldRecord> user_worlds; // Vector of world list playable by current player.
	StatisticsRecord user_statistcs; // Current player's statistics
	SettingsRecord user_settings; // Current player's settings

	GUIScene mScene = GUIScene();
	GUIText mLabel = GUIText(&large_font, "Endless Advanture", 0, 72, 16, 0, 1, 1);
	GUIImage mWorldsButton = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 1);
	GUIText mWorldsButtonText = GUIText(&large_font, "Worlds", 0, 158, 16, 0, 1, 1);
	GUIImage mSettingsButton = GUIImage(&menu_button, 0, 190, 256, 32, 0, 1, 1, 1);
	GUIText mSettingsButtonText = GUIText(&large_font, "Settings", 0, 198, 16, 0, 1, 1);
	GUIImage mStatisticsButton = GUIImage(&menu_button, 0, 230, 256, 32, 0, 1, 1, 1);
	GUIText mStatisticsButtonText = GUIText(&large_font, "Statistics", 0, 238, 16, 0, 1, 1);
	GUIImage mLogoutButton = GUIImage(&menu_button, 0, 270, 256, 32, 0, 1, 1, 1);
	GUIText mLogoutButtonText = GUIText(&large_font, "Logout", 0, 278, 16, 0, 1, 1);
	mScene.add(mLabel);
	mScene.add(mWorldsButton);
	mScene.add(mWorldsButtonText);
	mScene.add(mSettingsButton);
	mScene.add(mSettingsButtonText);
	mScene.add(mStatisticsButton);
	mScene.add(mStatisticsButtonText);
	mScene.add(mLogoutButton);
	mScene.add(mLogoutButtonText);

	// Player Worlds Menu (MENU_WORLDS)
	bool called = false;
	int world_count;
	int world_select_iter = 0;
	WorldRecord displayWorld = WorldRecord();
	GUIScene wrdScene = GUIScene();
	GUIText wrdLabel = GUIText(&large_font, "Endless Advanture", 0, 72, 16, 0, 1, 1);
	GUIImage wrdBackground = GUIImage(&menu_button, 0, 102, 320, 114, 0, 1, 1, 4);
	GUIText wrdWorldNameText = GUIText(&large_font, "-", 0, 116, 16, 0, 1, 1);
	GUIText wrdWorldOwnerText = GUIText(&large_font, "-", 0, 148, 16, 0, 1, 1);
	GUIText wrdWorldSharedText = GUIText(&large_font, "-", 0, 180, 16, 0, 1, 1);
	GUIImage wrdThisWorldButton = GUIImage(&menu_button, 0, 230, 256, 32, 0, 1, 1, 5);
	GUIText wrdThisWorldText = GUIText(&large_font, "Play", 0, 238, 16, 0, 1, 1);
	GUIImage wrdLeftButton = GUIImage(&menu_button, -192, 150, 32, 32, 0, 1, 1, 5);
	GUIText wrdLeftText = GUIText(&large_font, "<", -192, 158, 16, 0, 1, 1);
	GUIImage wrdRightButton = GUIImage(&menu_button, 192, 150, 32, 32, 0, 1, 1, 5);
	GUIText wrdRightText = GUIText(&large_font, ">", 192, 158, 16, 0, 1, 1);
	GUIImage wrdNewWorldButton = GUIImage(&menu_button, 0, 270, 256, 32, 0, 1, 1, 1);
	GUIText wrdNewWorldText = GUIText(&large_font, "New world", 0, 278, 16, 0, 1, 1);
	GUIImage wrdDeleteButton = GUIImage(&menu_button, 0, 310, 256, 32, 0, 1, 1, 5);
	GUIText wrdDeleteText = GUIText(&large_font, "Delete world", 0, 318, 16, 0, 1, 1);
	GUIImage wrdBackButton = GUIImage(&menu_button, 0, 350, 256, 32, 0, 1, 1, 1);
	GUIText wrdBackText = GUIText(&large_font, "Main Menu", 0, 358, 16, 0, 1, 1);
	wrdScene.add(wrdLabel);
	wrdScene.add(wrdBackground);
	wrdScene.add(wrdWorldNameText);
	wrdScene.add(wrdWorldOwnerText);
	wrdScene.add(wrdWorldSharedText);
	wrdScene.add(wrdThisWorldButton);
	wrdScene.add(wrdThisWorldText);
	wrdScene.add(wrdLeftButton);
	wrdScene.add(wrdLeftText);
	wrdScene.add(wrdRightButton);
	wrdScene.add(wrdRightText);
	wrdScene.add(wrdNewWorldButton);
	wrdScene.add(wrdNewWorldText);
	wrdScene.add(wrdDeleteButton);
	wrdScene.add(wrdDeleteText);
	wrdScene.add(wrdBackButton);
	wrdScene.add(wrdBackText);

	// Create New World Menu (MENU_NEW_WORLD)
	char new_world_name[32] = "My World";
	char new_world_seed[32] = "";
	int new_world_name_iter = 0;
	int new_world_seed_iter = 0;
	bool is_new_world_shared = false;
	int nwrd_selected_field = 0;
	char nwrd_key_input;
	GUIScene nwrdScene = GUIScene();
	GUIText nwrdNameLabel = GUIText(&large_font, "World Name", 0, 72, 16, 0, 1, 1);
	GUIImage nwrdNameField = GUIImage(&menu_button, 0, 92, 256, 32, 0, 1, 1, 3);
	GUIText nwrdNameFieldText = GUIText(&large_font, new_world_name, 0, 100, 16, 0, 1, 1);
	GUIText nwrdSeedLabel = GUIText(&large_font, "Seed", 0, 142, 16, 0, 1, 1);
	GUIImage nwrdSeedField = GUIImage(&menu_button, 0, 162, 256, 32, 0, 1, 1, 3);
	GUIText nwrdSeedFieldText = GUIText(&large_font, new_world_seed, 0, 170, 16, 0, 1, 1);
	GUIText nwrdSharedLabel = GUIText(&large_font, "Is world shared?", 0, 212, 16, 0, 1, 1);
	GUIImage nwrdSharedButton = GUIImage(&menu_button, 0, 232, 64, 32, 0, 1, 1, 1);
	GUIText nwrdSharedText = GUIText(&large_font, "No", 0, 240, 16, 0, 1, 1);
	GUIImage nwrdCreateButton = GUIImage(&menu_button, 0, 282, 256, 32, 0, 1, 1, 1);
	GUIText nwrdCreateText = GUIText(&large_font, "Create", 0, 290, 16, 0, 1, 1);
	GUIImage nwrdBackButton = GUIImage(&menu_button, 0, 322, 256, 32, 0, 1, 1, 1);
	GUIText nwrdBackText = GUIText(&large_font, "Back", 0, 330, 16, 0, 1, 1);
	nwrdScene.add(nwrdNameLabel);
	nwrdScene.add(nwrdNameField);
	nwrdScene.add(nwrdNameFieldText);
	nwrdScene.add(nwrdSeedLabel);
	nwrdScene.add(nwrdSeedField);
	nwrdScene.add(nwrdSeedFieldText);
	nwrdScene.add(nwrdSharedLabel);
	nwrdScene.add(nwrdSharedButton);
	nwrdScene.add(nwrdSharedText);
	nwrdScene.add(nwrdCreateButton);
	nwrdScene.add(nwrdCreateText);
	nwrdScene.add(nwrdBackButton);
	nwrdScene.add(nwrdBackText);

	// Delete World Conirmation (MENU_DELETE_CONFIRM)
	GUIScene delScene = GUIScene();
	GUIText delConfirmLabel1 = GUIText(&large_font, "You are about to delete this world!", 0, 72, 16, 0, 1, 1);
	GUIText delConfirmLabel2 = GUIText(&large_font, "Are you sure?", 0, 92, 16, 0, 1, 1);
	GUIImage delYesButton = GUIImage(&menu_button, 0, 150, 256, 32, 0, 1, 1, 1);
	GUIText delYesText = GUIText(&large_font, "Delete", 0, 158, 16, 0, 1, 1);
	GUIImage delNoButton = GUIImage(&menu_button, 0, 190, 256, 32, 0, 1, 1, 1);
	GUIText delNoText = GUIText(&large_font, "Go back", 0, 198, 16, 0, 1, 1);
	delScene.add(delConfirmLabel1);
	delScene.add(delConfirmLabel2);
	delScene.add(delYesButton);
	delScene.add(delYesText);
	delScene.add(delNoButton);
	delScene.add(delNoText);

	// Settings Menu (MENU_SETTINGS)
	int temp_gui_scale = 0;
	int temp_render_distance = 0;
	char temp_buffer[32];
	GUIScene sScene = GUIScene();
	GUIText sSettingsLabel = GUIText(&large_font, "Settings", 0, 72, 16, 0, 1, 1);
	GUIImage sCancel = GUIImage(&menu_button, -144, 290, 256, 32, 0, 1, 1, 1);
	GUIText sCancelText = GUIText(&large_font, "Cancel", -144, 298, 16, 0, 1, 1);
	GUIImage sApply = GUIImage(&menu_button, 144, 290, 256, 32, 0, 1, 1, 1);
	GUIText sApplyText = GUIText(&large_font, "Apply", 144, 298, 16, 0, 1, 1);
	GUIText sRenderDistance = GUIText(&large_font, "Render Distance: 0", 0, 104, 16, 0, 1, 1);
	GUIImage sRenderPlus = GUIImage(&menu_button, 200, 96, 32, 32, 0, 1, 1, 1);
	GUIText sRenderPlusText = GUIText(&large_font, "+", 200, 104, 16, 0, 1, 1);
	GUIImage sRenderMinus = GUIImage(&menu_button, -200, 96, 32, 32, 0, 1, 1, 1);
	GUIText sRenderMinusText = GUIText(&large_font, "-", -200, 104, 16, 0, 1, 1);
	GUIText sGUIScale = GUIText(&large_font, "HUD Scale: 0", 0, 144, 16, 0, 1, 1);
	GUIImage sGUIScalePlus = GUIImage(&menu_button, 200, 136, 32, 32, 0, 1, 1, 1);
	GUIText sGUIScalePlusText = GUIText(&large_font, "+", 200, 144, 16, 0, 1, 1);
	GUIImage sGUIScaleMinus = GUIImage(&menu_button, -200, 136, 32, 32, 0, 1, 1, 1);
	GUIText sGUIScaleMinusText = GUIText(&large_font, "-", -200, 144, 16, 0, 1, 1);
	GUIText sChangePasswordText = GUIText(&large_font, "Change password", 0, 184, 16, 0, 1, 1);
	GUIImage sChangePassword = GUIImage(&menu_button, 0, 176, 256, 32, 0, 1, 1, 1);
	GUIText sDeleteAccountText = GUIText(&large_font, "Delete account", 0, 224, 16, 0, 1, 1);
	GUIImage sDeleteAccount = GUIImage(&menu_button, 0, 216, 256, 32, 0, 1, 1, 1);
	sScene.add(sSettingsLabel);
	sScene.add(sCancel);
	sScene.add(sCancelText);
	sScene.add(sApply);
	sScene.add(sApplyText);
	sScene.add(sRenderDistance);
	sScene.add(sRenderPlus);
	sScene.add(sRenderPlusText);
	sScene.add(sRenderMinus);
	sScene.add(sRenderMinusText);
	sScene.add(sGUIScale);
	sScene.add(sGUIScalePlus);
	sScene.add(sGUIScalePlusText);
	sScene.add(sGUIScaleMinus);
	sScene.add(sGUIScaleMinusText);
	sScene.add(sChangePassword);
	sScene.add(sChangePasswordText);
	sScene.add(sDeleteAccount);
	sScene.add(sDeleteAccountText);

	// Change Password Menu (MENU_CHANGE_PASSWORD)
	char cp_old_pass[32] = "";
	char cp_new_pass[32] = "";
	int cp_old_pass_iter = 0;
	int cp_new_pass_iter = 0;
	int cp_selected_field = 0;
	char cp_key_input;
	GUIScene cpScene = GUIScene();
	GUIText cpChangePWLabel = GUIText(&large_font, "Change Password", 0, 72, 16, 0, 1, 1);
	GUIImage cpCancel = GUIImage(&menu_button, -144, 290, 256, 32, 0, 1, 1, 1);
	GUIText cpCancelText = GUIText(&large_font, "Cancel", -144, 298, 16, 0, 1, 1);
	GUIImage cpApply = GUIImage(&menu_button, 144, 290, 256, 32, 0, 1, 1, 1);
	GUIText cpApplyText = GUIText(&large_font, "Confirm", 144, 298, 16, 0, 1, 1);
	GUIText cpPreviousLabel = GUIText(&large_font, "Enter your previous password:", 0, 102, 16, 0, 1, 1);
	GUIImage cpPreviousBox = GUIImage(&menu_button, 0, 132, 256, 32, 0, 1, 1, 3);
	GUIText cpPreviousField = GUIText(&large_font, cp_old_pass, 0, 140, 16, 0, 1, 1);
	GUIText cpNewPassword = GUIText(&large_font, "Enter your new password:", 0, 172, 16, 0, 1, 1);
	GUIImage cpNewPasswordBox = GUIImage(&menu_button, 0, 202, 256, 32, 0, 1, 1, 3);
	GUIText cpNewPasswordField = GUIText(&large_font, cp_new_pass, 0, 210, 16, 0, 1, 1);
	GUIText cpMessage = GUIText(&large_font, "", 0, 250, 16, 0, 1, 1);
	cpScene.add(cpChangePWLabel);
	cpScene.add(cpCancel);
	cpScene.add(cpCancelText);
	cpScene.add(cpApply);
	cpScene.add(cpApplyText);
	cpScene.add(cpPreviousLabel);
	cpScene.add(cpPreviousBox);
	cpScene.add(cpPreviousField);
	cpScene.add(cpNewPassword);
	cpScene.add(cpNewPasswordBox);
	cpScene.add(cpNewPasswordField);
	cpScene.add(cpMessage);

	// Statistics Menu (MENU_STATISTICS)
	GUIScene statScene = GUIScene();
	GUIText statTitle = GUIText(&large_font, "Statistics", 0, 72, 16, 0, 1, 1);
	GUIText statLogins = GUIText(&large_font, "", 0, 112, 16, 0, 1, 1);
	GUIText statTime = GUIText(&large_font, "", 0, 152, 16, 0, 1, 1);
	GUIText statBlockPlaced = GUIText(&large_font, "", 0, 192, 16, 0, 1, 1);
	GUIText statBlockMined = GUIText(&large_font, "", 0, 232, 16, 0, 1, 1);
	GUIImage statBack = GUIImage(&menu_button, 0, 290, 256, 32, 0, 1, 1, 1);
	GUIText statBackText = GUIText(&large_font, "Back", 0, 298, 16, 0, 1, 1);
	statScene.add(statTitle);
	statScene.add(statLogins);
	statScene.add(statTime);
	statScene.add(statBlockPlaced);
	statScene.add(statBlockMined);
	statScene.add(statBack);
	statScene.add(statBackText);

	// Delete Account Confirmation Page (MENU_DELETE_ACCOUNT)
	bool da_called = false;
	bool da_owned_shared = false;
	GUIScene daScene = GUIScene();
	GUIText daDeleteLabel = GUIText(&large_font, "Are you sure?", 0, 72, 16, 0, 1, 1);
	GUIText daYouCant = GUIText(&large_font, "", 0, 102, 16, 0, 1, 1); // You own shared worlds, delete those first
	GUIImage daCancel = GUIImage(&menu_button, -144, 290, 256, 32, 0, 1, 1, 1);
	GUIText daCancelText = GUIText(&large_font, "Cancel", -144, 298, 16, 0, 1, 1);
	GUIImage daApply = GUIImage(&menu_button, 144, 290, 256, 32, 0, 1, 1, 1);
	GUIText daApplyText = GUIText(&large_font, "Confirm", 144, 298, 16, 0, 1, 1);
	daScene.add(daDeleteLabel);
	daScene.add(daYouCant);
	daScene.add(daCancel);
	daScene.add(daCancelText);
	daScene.add(daApply);
	daScene.add(daApplyText);

	// Game Itself (MENU_GAME)
	bool new_world; // If false, loading from database, if true, saving into database
	GUIScene gmScene = GUIScene();
	GUIText gmLoadingLabel = GUIText(&large_font, "Loading...", 0, 72, 16, 0, 1, 1);
	gmScene.add(gmLoadingLabel);

	// Set Default Menu
	if (!dbopen)
		selected_menu = MENU_ERROR;
	else
		selected_menu = MENU_WELCOME;


	int last_width = 0;
	int last_height = 0;

	while (game_running && !game_renderer.isCloseRequested()) {
		
		game_renderer.getMouseCoordinate(mouse_x, mouse_y);

		if (_cwidth != last_width || _cheight != last_height) {
			last_height = _cheight;
			last_width = _cwidth;
			int scale = 1;
			if (last_height >= 800 && last_width >= 1060)
				scale = 2;
			wScene.setGUIScaleForAll(scale);
			lScene.setGUIScaleForAll(scale);
			nScene.setGUIScaleForAll(scale);
			mScene.setGUIScaleForAll(scale);
			wrdScene.setGUIScaleForAll(scale);
			nwrdScene.setGUIScaleForAll(scale);
			delScene.setGUIScaleForAll(scale);
			errScene.setGUIScaleForAll(scale);
			gmScene.setGUIScaleForAll(scale);
			sScene.setGUIScaleForAll(scale);
			daScene.setGUIScaleForAll(scale);
			cpScene.setGUIScaleForAll(scale);
			statScene.setGUIScaleForAll(scale);
			backgroundScene.setGUIScaleForAll(scale);
		}
		
		switch (selected_menu)
		{
		case MENU_NULL:
			selected_menu = MENU_WELCOME;
			break;
		case MENU_WELCOME:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (wLoginButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_LOGIN;
					l_username[0] = 0;
					l_user_iterrator = 0;
					l_password[0] = 0;
					l_pass_iterrator = 0;
					lUsernameFieldText.setText(l_username);
					lPasswordFieldText.setText(l_password);
				}
				if (wNewButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_NEW_USER;
					n_username[0] = 0;
					n_user_iterrator = 0;
					n_password[0] = 0;
					n_pass_iterrator = 0;
					nUsernameFieldText.setText(n_username);
					nPasswordFieldText.setText(n_password);
				}
				if (wExit.isMouseInside(mouse_x, mouse_y)) game_running = false;
			}
			wLoginButton.setAtlasIndex((wLoginButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			wNewButton.setAtlasIndex((wNewButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			wExit.setAtlasIndex((wExit.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(wScene);
			game_renderer.updateDisplay();
			break;
		case MENU_LOGIN:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (lLoginButton.isMouseInside(mouse_x, mouse_y)) {
					if (dbmo.usernameExists(l_username)) {
						char* dpass = dbmo.getPasswordByUsername(l_username);
						char* upass = hashPassword(l_password);
						if (stringMatches(dpass, upass)) {
							selected_menu = MENU_MAIN;
							// initialize player
							memcpy(username, l_username, 32);
							current_user_id = dbmo.getUserIdByUsername(username);
							user_settings.gui_scale = dbmo.readIntegerValueForUser("Settings", "gui_scale", username);
							user_settings.render_dist = dbmo.readIntegerValueForUser("Settings", "render_dist", username);
							user_statistcs.blocks_mined = dbmo.readIntegerValueForUser("Statistics", "blocks_mined", username);
							user_statistcs.blocks_placed = dbmo.readIntegerValueForUser("Statistics", "blocks_placed", username);
							user_statistcs.play_time = dbmo.readIntegerValueForUser("Statistics", "play_time", username);
							user_statistcs.login_count = dbmo.readIntegerValueForUser("Statistics", "login_count", username) + 1;
							user_logged_in = true;
							// -----------------
							l_username[0] = 0;
							l_password[0] = 0;
							l_user_iterrator = l_pass_iterrator = 0;
							lMessage.setText("");
							lUsernameFieldText.setText(l_username);
							lPasswordFieldText.setText(l_password);
						}
						else {
							l_password[0] = 0;
							l_pass_iterrator = 0;
							lUsernameFieldText.setText(l_username);
							lPasswordFieldText.setText(l_password);
							lMessage.setText("Wrong password!");
						}
						delete[] dpass;
						delete[] upass;
					}
					else {
						l_password[0] = 0;
						l_pass_iterrator = 0;
						lMessage.setText("Username does not exist!");
					}
				}
				if (lBackButton .isMouseInside(mouse_x, mouse_y)) selected_menu = MENU_WELCOME;
				l_selected_text_field = lUsernameField.isMouseInside(mouse_x, mouse_y) ? 1 : lPasswordField.isMouseInside(mouse_x, mouse_y) ? 2 : 0;
				lUsernameField.setAtlasIndex((l_selected_text_field == 1) ? 2 : 3);
				lPasswordField.setAtlasIndex((l_selected_text_field == 2) ? 2 : 3);
			}
			if (l_keyinput = getUsernameKeyAndClear()) {
				if (l_keyinput == 127) {
					if (l_user_iterrator && l_selected_text_field == 1) {
						l_user_iterrator--;
						l_username[l_user_iterrator] = 0;
						lUsernameFieldText.setText(l_username);
					}
					if (l_pass_iterrator && l_selected_text_field == 2) {
						l_pass_iterrator--;
						l_password[l_pass_iterrator] = 0;
						lPasswordFieldText.setText(l_password);
					}
				}
				else if (l_user_iterrator < 15 && l_selected_text_field == 1) {
					l_username[l_user_iterrator] = l_keyinput;
					l_user_iterrator++;
					l_username[l_user_iterrator] = 0;
					lUsernameFieldText.setText(l_username);
				}
				else if (l_pass_iterrator < 15 && l_selected_text_field == 2) {
					l_password[l_pass_iterrator] = l_keyinput;
					l_pass_iterrator++;
					l_password[l_pass_iterrator] = 0;
					lPasswordFieldText.setText(l_password);
				}
			}
			lLoginButton.setAtlasIndex((lLoginButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			lBackButton.setAtlasIndex((lBackButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(lScene);
			game_renderer.updateDisplay();
			break;
		case MENU_NEW_USER:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (nCreateButton.isMouseInside(mouse_x, mouse_y)) {
					if (!dbmo.usernameExists(n_username)) {
						char* upass = hashPassword(n_password);
						dbmo.addUser(n_username, upass);
						delete[] upass;
						selected_menu = MENU_MAIN;
						// initialize new player
						memcpy(username, n_username, 32);
						current_user_id = dbmo.getUserIdByUsername(username);
						user_settings.gui_scale = 1;
						user_settings.render_dist = 8;
						user_statistcs.blocks_mined = 0;
						user_statistcs.blocks_placed = 0;
						user_statistcs.play_time = 0;
						user_statistcs.login_count = 1;
						user_logged_in = true;
						// -----------------
						n_password[0] = 0;
						n_username[0] = 0;
						nUsernameFieldText.setText(n_username);
						nPasswordFieldText.setText(n_password);
						nMessage.setText("");
						n_pass_iterrator = n_user_iterrator = 0;
					}
					else {
						n_password[0] = 0;
						n_pass_iterrator = 0;
						nUsernameFieldText.setText(n_username);
						nPasswordFieldText.setText(n_password);
						nMessage.setText("Username already taken!");
					}
				}
				if (nBackButton.isMouseInside(mouse_x, mouse_y)) selected_menu = MENU_WELCOME;
				n_selected_text_field = nUsernameField.isMouseInside(mouse_x, mouse_y) ? 1 : nPasswordField.isMouseInside(mouse_x, mouse_y) ? 2 : 0;
				nUsernameField.setAtlasIndex((n_selected_text_field == 1) ? 2 : 3);
				nPasswordField.setAtlasIndex((n_selected_text_field == 2) ? 2 : 3);
			}
			if (n_keyinput = getUsernameKeyAndClear()) {
				if (n_keyinput == 127) {
					if (n_user_iterrator && n_selected_text_field == 1) {
						n_user_iterrator--;
						n_username[n_user_iterrator] = 0;
						nUsernameFieldText.setText(n_username);
					}
					if (n_pass_iterrator && n_selected_text_field == 2) {
						n_pass_iterrator--;
						n_password[n_pass_iterrator] = 0;
						nPasswordFieldText.setText(n_password);
					}
				}
				else if (n_user_iterrator < 15 && n_selected_text_field == 1) {
					n_username[n_user_iterrator] = n_keyinput;
					n_user_iterrator++;
					n_username[n_user_iterrator] = 0;
					nUsernameFieldText.setText(n_username);
				}
				else if (n_pass_iterrator < 15 && n_selected_text_field == 2) {
					n_password[n_pass_iterrator] = n_keyinput;
					n_pass_iterrator++;
					n_password[n_pass_iterrator] = 0;
					nPasswordFieldText.setText(n_password);
				}
			}
			nCreateButton.setAtlasIndex((nCreateButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			nBackButton.setAtlasIndex((nBackButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(nScene);
			game_renderer.updateDisplay();
			break;
		case MENU_MAIN:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (mWorldsButton.isMouseInside(mouse_x, mouse_y)) { 
					selected_menu = MENU_WORLDS;
					called = true;
					world_count = user_worlds.size();
					wrdThisWorldButton.setAtlasIndex((world_count < 1) ? 5 : 1);
					wrdLeftButton.setAtlasIndex((world_count < 2) ? 5 : 1);
					wrdRightButton.setAtlasIndex((world_count < 2) ? 5 : 1);
				}
				if (mLogoutButton.isMouseInside(mouse_x, mouse_y)) {
					user_logged_in = false;
					user_worlds.clear();
					dbmo.updateIntegerValueForUser("Settings", "gui_scale", username, user_settings.gui_scale);
					dbmo.updateIntegerValueForUser("Settings", "render_dist", username, user_settings.render_dist);
					dbmo.updateIntegerValueForUser("Statistics", "blocks_mined", username, user_statistcs.blocks_mined);
					dbmo.updateIntegerValueForUser("Statistics", "blocks_placed", username, user_statistcs.blocks_placed);
					dbmo.updateIntegerValueForUser("Statistics", "play_time", username, user_statistcs.play_time);
					dbmo.updateIntegerValueForUser("Statistics", "login_count", username, user_statistcs.login_count);
					selected_menu = MENU_WELCOME;
				}
				if (mSettingsButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_SETTINGS;
					temp_gui_scale = user_settings.gui_scale;
					temp_render_distance = user_settings.render_dist;
					sprintf(temp_buffer, "Render Distance: %d", temp_render_distance);
					sRenderDistance.setText(temp_buffer);
					sprintf(temp_buffer, "HUD Scale: %d", temp_gui_scale);
					sGUIScale.setText(temp_buffer);
				}
				if (mStatisticsButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_STATISTICS;
					char tmp[64];
					sprintf(tmp, "Times logged in: %d", user_statistcs.login_count);
					statLogins.setText(tmp);
					int s = user_statistcs.play_time;
					int m = s / 60;
					int h = m / 60;
					int d = h / 24;
					s %= 60;
					m %= 60;
					h %= 24;
					if (d)
						sprintf(tmp, "Playtime: %ddays %02d:%02d:%02d", d, h, m, s);
					else
						sprintf(tmp, "Playtime: %d:%02d:%02d", h, m, s);
					statTime.setText(tmp);
					sprintf(tmp, "Blocks placed: %d", user_statistcs.blocks_placed);
					statBlockPlaced.setText(tmp);
					sprintf(tmp, "Blocks mined: %d", user_statistcs.blocks_mined);
					statBlockMined.setText(tmp);
				}
			}
			mLogoutButton.setAtlasIndex((mLogoutButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			mWorldsButton.setAtlasIndex((mWorldsButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			mSettingsButton.setAtlasIndex((mSettingsButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			mStatisticsButton.setAtlasIndex((mStatisticsButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(mScene);
			game_renderer.updateDisplay();
			break;
		case MENU_SETTINGS:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (sCancel.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_MAIN;
				}
				if (sApply.isMouseInside(mouse_x, mouse_y)) {
					user_settings.gui_scale = temp_gui_scale;
					user_settings.render_dist = temp_render_distance;
					selected_menu = MENU_MAIN;
				}
				if (sRenderPlus.isMouseInside(mouse_x, mouse_y)) {
					if (temp_render_distance < 16)
						temp_render_distance++;
					sprintf(temp_buffer, "Render Distance: %d", temp_render_distance);
					sRenderDistance.setText(temp_buffer);
				}
				if (sRenderMinus.isMouseInside(mouse_x, mouse_y)) {
					if (temp_render_distance > 4)
						temp_render_distance--;
					sprintf(temp_buffer, "Render Distance: %d", temp_render_distance);
					sRenderDistance.setText(temp_buffer);
				}
				if (sGUIScalePlus.isMouseInside(mouse_x, mouse_y)) {
					if (temp_gui_scale < 4)
						temp_gui_scale++;
					sprintf(temp_buffer, "HUD Scale: %d", temp_gui_scale);
					sGUIScale.setText(temp_buffer);
				}
				if (sGUIScaleMinus.isMouseInside(mouse_x, mouse_y)) {
					if (temp_gui_scale > 1)
						temp_gui_scale--;
					sprintf(temp_buffer, "HUD Scale: %d", temp_gui_scale);
					sGUIScale.setText(temp_buffer);
				}
				if (sChangePassword.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_CHANGE_PASSWORD;
				}
				if (sDeleteAccount.isMouseInside(mouse_x, mouse_y)) {
					da_called = true;
					selected_menu = MENU_DELETE_ACCOUNT;
				}
			}
			sCancel.setAtlasIndex((sCancel.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sApply.setAtlasIndex((sApply.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sRenderPlus.setAtlasIndex((sRenderPlus.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sRenderMinus.setAtlasIndex((sRenderMinus.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sGUIScalePlus.setAtlasIndex((sGUIScalePlus.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sGUIScaleMinus.setAtlasIndex((sGUIScaleMinus.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sChangePassword.setAtlasIndex((sChangePassword.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			sDeleteAccount.setAtlasIndex((sDeleteAccount.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(sScene);
			game_renderer.updateDisplay();
			break;
		case MENU_STATISTICS:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (statBack.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_MAIN;
				}
			}
			statBack.setAtlasIndex((statBack.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(statScene);
			game_renderer.updateDisplay();
			break;
		case MENU_DELETE_ACCOUNT:
			//First call
			if (da_called) {
				da_called = false;
				user_worlds.clear();
				user_worlds = dbmo.getOwnedWorldRecords(current_user_id);
				da_owned_shared = false;
				for (const WorldRecord& wr : user_worlds)
					if (wr.is_shared)
						da_owned_shared = true;
				daYouCant.setText(da_owned_shared ? "You own shared worlds, delete those first" : "Your account and worlds will be deleted");
				daApply.setAtlasIndex(5);
			}
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (daCancel.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_SETTINGS;
				}
				if (daApply.isMouseInside(mouse_x, mouse_y) && !da_owned_shared) {
					selected_menu = MENU_WELCOME;
					for (const WorldRecord& wr : user_worlds) {
						char folder_path[256];
						sprintf(folder_path, "%s%d", DATA_DIR_PATH, wr.world_id);
						if (std::filesystem::remove_all(folder_path)) {
							dbmo.removeWorldRecord(wr.world_id, wr.owner_id);
						}
						else {
							// Handle
						}
					}
					dbmo.removeUser(username);
				}
			}
			if (!da_owned_shared) daApply.setAtlasIndex(daApply.isMouseInside(mouse_x, mouse_y) ? 0 : 1);
			daCancel.setAtlasIndex(daCancel.isMouseInside(mouse_x, mouse_y) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(daScene);
			game_renderer.updateDisplay();
			break;
		case MENU_CHANGE_PASSWORD:
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (cpCancel.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_SETTINGS;
					cp_old_pass_iter = 0;
					cp_old_pass[0] = 0;
					cp_new_pass_iter = 0;
					cp_new_pass[0] = 0;
					cpPreviousField.setText(cp_old_pass);
					cpNewPasswordField.setText(cp_new_pass);
				}
				if (cpApply.isMouseInside(mouse_x, mouse_y)) {
					char* pas = dbmo.getPasswordByUsername(username);
					char* old_pas_hash = hashPassword(cp_old_pass);
					if (!strcmp(pas, old_pas_hash)) {
						dbmo.updatePasswordForUser(username, hashPassword(cp_new_pass));
						selected_menu = MENU_SETTINGS;
					}
					else {
						cpMessage.setText("Wrong password");
					}
					cp_old_pass_iter = 0;
					cp_old_pass[0] = 0;
					cp_new_pass_iter = 0;
					cp_new_pass[0] = 0;
					cpPreviousField.setText(cp_old_pass);
					cpNewPasswordField.setText(cp_new_pass);
				}
				cp_selected_field = cpPreviousBox.isMouseInside(mouse_x, mouse_y) ? 1 : cpNewPasswordBox.isMouseInside(mouse_x, mouse_y) ? 2 : 0;
				cpPreviousBox.setAtlasIndex((cp_selected_field == 1) ? 2 : 3);
				cpNewPasswordBox.setAtlasIndex((cp_selected_field == 2) ? 2 : 3);
			}
			if (cp_key_input = getUsernameKeyAndClear(true)) {
				if (cp_key_input == 127) {
					if (cp_old_pass_iter && cp_selected_field == 1) {
						cp_old_pass_iter--;
						cp_old_pass[cp_old_pass_iter] = 0;
						cpPreviousField.setText(cp_old_pass);
					}
					if (cp_new_pass_iter && cp_selected_field == 2) {
						cp_new_pass_iter--;
						cp_new_pass[cp_new_pass_iter] = 0;
						cpNewPasswordField.setText(cp_new_pass);
					}
				}
				else if (cp_old_pass_iter < 31 && cp_selected_field == 1) {
					cp_old_pass[cp_old_pass_iter] = cp_key_input;
					cp_old_pass_iter++;
					cp_old_pass[cp_old_pass_iter] = 0;
					cpPreviousField.setText(cp_old_pass);
				}
				else if (cp_new_pass_iter < 31 && cp_selected_field == 2) {
					cp_new_pass[cp_new_pass_iter] = cp_key_input;
					cp_new_pass_iter++;
					cp_new_pass[cp_new_pass_iter] = 0;
					cpNewPasswordField.setText(cp_new_pass);
				}
			}
			cpCancel.setAtlasIndex((cpCancel.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			cpApply.setAtlasIndex((cpApply.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(cpScene);
			game_renderer.updateDisplay();
			break;
		case MENU_WORLDS:
			//Reload worlds call
			if (called) {
				called = false;
				user_worlds.clear();
				user_worlds = dbmo.getWorldRecords(current_user_id);
				world_count = user_worlds.size();
				world_select_iter = 0;
				if (world_count > 0) {
					displayWorld = user_worlds.at(0);
					char* temp = dbmo.getUsernameByID(displayWorld.owner_id);
					wrdWorldNameText.setText(displayWorld.world_name);
					wrdWorldOwnerText.setText(temp);
					wrdWorldSharedText.setText(displayWorld.is_shared ? "Shared world" : "Private world");
					delete[] temp;
				}
				else {
					wrdWorldNameText.setText("You have no worlds");
					wrdWorldOwnerText.setText("");
					wrdWorldSharedText.setText("");
				}
				wrdLeftButton.setAtlasIndex(5);
				wrdRightButton.setAtlasIndex(5);
				wrdDeleteButton.setAtlasIndex(5);
				wrdThisWorldButton.setAtlasIndex(5);
			}
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (wrdBackButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_MAIN;
				}
				if (wrdNewWorldButton.isMouseInside(mouse_x, mouse_y)) selected_menu = MENU_NEW_WORLD;
				if (world_count > 0 && world_select_iter < world_count - 1 && wrdRightButton.isMouseInside(mouse_x, mouse_y)) {
					world_select_iter++;
					displayWorld = user_worlds.at(world_select_iter);
					char* temp = dbmo.getUsernameByID(displayWorld.owner_id);
					wrdWorldNameText.setText(displayWorld.world_name);
					wrdWorldOwnerText.setText(temp);
					wrdWorldSharedText.setText(displayWorld.is_shared ? "Shared world" : "Private world");
					delete[] temp;
					if (world_select_iter == world_count - 1)
						wrdRightButton.setAtlasIndex(5);
					if (displayWorld.is_shared && displayWorld.owner_id != current_user_id) {
						wrdDeleteButton.setAtlasIndex(5);
					}
				}
				if (world_count > 0 && world_select_iter > 0 && wrdLeftButton.isMouseInside(mouse_x, mouse_y)) {
					world_select_iter--;
					displayWorld = user_worlds.at(world_select_iter);
					char* temp = dbmo.getUsernameByID(displayWorld.owner_id);
					wrdWorldNameText.setText(displayWorld.world_name);
					wrdWorldOwnerText.setText(temp);
					wrdWorldSharedText.setText(displayWorld.is_shared ? "Shared world" : "Private world");
					delete[] temp;
					if (world_select_iter == 0)
						wrdLeftButton.setAtlasIndex(5);
				}
				if (world_count && world_select_iter >= 0 && world_select_iter < world_count && wrdThisWorldButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_GAME;
					new_world = false;
				}
				if (world_count > 0 && !(displayWorld.is_shared && displayWorld.owner_id != current_user_id) && wrdDeleteButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_DELETE_CONFIRM;
				}
			}
			wrdBackButton.setAtlasIndex((wrdBackButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			wrdNewWorldButton.setAtlasIndex((wrdNewWorldButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			if (world_count > 0) wrdThisWorldButton.setAtlasIndex((wrdThisWorldButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			if (world_count > 0 && !(displayWorld.is_shared && displayWorld.owner_id != current_user_id)) wrdDeleteButton.setAtlasIndex((wrdDeleteButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			if (world_count > 1 && world_select_iter > 0) wrdLeftButton.setAtlasIndex((wrdLeftButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			if (world_count > 1 && world_select_iter < world_count - 1) wrdRightButton.setAtlasIndex((wrdRightButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(wrdScene);
			game_renderer.updateDisplay();
			break;
		case MENU_NEW_WORLD:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (nwrdBackButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_WORLDS;
				}
				if (nwrdCreateButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_GAME;
					new_world = true;
				}
				if (nwrdSharedButton.isMouseInside(mouse_x, mouse_y)) {
					if (is_new_world_shared) {
						is_new_world_shared = false;
						nwrdSharedText.setText("No");
					}
					else {
						is_new_world_shared = true;
						nwrdSharedText.setText("Yes");
					}
				}
				nwrd_selected_field = nwrdNameField.isMouseInside(mouse_x, mouse_y) ? 1 : nwrdSeedField.isMouseInside(mouse_x, mouse_y) ? 2 : 0;
				nwrdNameField.setAtlasIndex((nwrd_selected_field == 1) ? 2 : 3);
				nwrdSeedField.setAtlasIndex((nwrd_selected_field == 2) ? 2 : 3);
			}
			if (nwrd_key_input = getUsernameKeyAndClear(true)) {
				if (nwrd_key_input == 127) {
					if (new_world_name_iter && nwrd_selected_field == 1) {
						new_world_name_iter--;
						new_world_name[new_world_name_iter] = 0;
						nwrdNameFieldText.setText(new_world_name);
					}
					if (new_world_seed_iter && nwrd_selected_field == 2) {
						new_world_seed_iter--;
						new_world_seed[new_world_seed_iter] = 0;
						nwrdSeedFieldText.setText(new_world_seed);
					}
				}
				else if (new_world_name_iter < 31 && nwrd_selected_field == 1) {
					new_world_name[new_world_name_iter] = nwrd_key_input;
					new_world_name_iter++;
					new_world_name[new_world_name_iter] = 0;
					nwrdNameFieldText.setText(new_world_name);
				}
				else if (new_world_seed_iter < 31 && nwrd_selected_field == 2) {
					new_world_seed[new_world_seed_iter] = nwrd_key_input;
					new_world_seed_iter++;
					new_world_seed[new_world_seed_iter] = 0;
					nwrdSeedFieldText.setText(new_world_seed);
				}
			}
			nwrdBackButton.setAtlasIndex((nwrdBackButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			nwrdCreateButton.setAtlasIndex((nwrdCreateButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			nwrdSharedButton.setAtlasIndex((nwrdSharedButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(nwrdScene);
			game_renderer.updateDisplay();
			break;
		case MENU_GAME:
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(gmScene);
			game_renderer.updateDisplay();
			if (new_world) {
				int id = dbmo.addWorldRecord(new_world_name, current_user_id, is_new_world_shared, new_world_seed);
				WorldRecord wr;
				wr.setWorldName(new_world_name);
				wr.setWorldSeed(new_world_seed);
				wr.owner_id = current_user_id;
				wr.world_id = id;
				wr.is_shared = is_new_world_shared;
				game_renderer.setCursorMode(1);
				game_play(wr, user_settings, user_statistcs, current_user_id, game_camera, game_renderer, new_world);
				game_renderer.setCursorMode(0);
				selected_menu = MENU_MAIN;
			}
			else {
				WorldRecord wr;
				wr.setWorldSeed(displayWorld.world_seed);
				wr.setWorldName(displayWorld.world_name);
				wr.owner_id = displayWorld.owner_id;
				wr.is_shared = displayWorld.is_shared;
				wr.world_id = displayWorld.world_id;
				game_renderer.setCursorMode(1);
				game_play(wr, user_settings, user_statistcs, current_user_id, game_camera, game_renderer, new_world);
				game_renderer.setCursorMode(0);
				selected_menu = MENU_MAIN;
			}
			break;
		case MENU_DELETE_CONFIRM:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (delNoButton.isMouseInside(mouse_x, mouse_y)) {
					selected_menu = MENU_WORLDS;
				}
				if (delYesButton.isMouseInside(mouse_x, mouse_y)) {
					char folder_path[256];
					sprintf(folder_path, "%s%d", DATA_DIR_PATH, displayWorld.world_id);
					if (std::filesystem::remove_all(folder_path)) {
						dbmo.removeWorldRecord(displayWorld.world_id, displayWorld.owner_id);
					}
					else {
						// Handle
					}
					selected_menu = MENU_WORLDS;
					called = true;
				}
			}
			delNoButton.setAtlasIndex((delNoButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			delYesButton.setAtlasIndex((delYesButton.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(delScene);
			game_renderer.updateDisplay();
			break;
		case MENU_ERROR:
			//Functionalities
			if (isMouseLeftClickPressed()) {
				clearMouseLeftClickPressed();
				if (errExit.isMouseInside(mouse_x, mouse_y)) game_running = false;
			}
			errExit.setAtlasIndex((errExit.isMouseInside(mouse_x, mouse_y)) ? 0 : 1);
			//Display
			game_renderer.prepare();
			game_renderer.prepare2D();
			game_renderer.renderGUIScene(backgroundScene);
			game_renderer.renderGUIScene(errScene);
			game_renderer.updateDisplay();
			break;
		default:
			break;
		}
	}

	if (user_logged_in) {
		user_logged_in = false;
		user_worlds.clear();
		dbmo.updateIntegerValueForUser("Settings", "gui_scale", username, user_settings.gui_scale);
		dbmo.updateIntegerValueForUser("Settings", "render_dist", username, user_settings.render_dist);
		dbmo.updateIntegerValueForUser("Statistics", "blocks_mined", username, user_statistcs.blocks_mined);
		dbmo.updateIntegerValueForUser("Statistics", "blocks_placed", username, user_statistcs.blocks_placed);
		dbmo.updateIntegerValueForUser("Statistics", "play_time", username, user_statistcs.play_time);
		dbmo.updateIntegerValueForUser("Statistics", "login_count", username, user_statistcs.login_count);
	}

	dbmo.close();

	game_renderer.destroy();

	game_running = false;
}
