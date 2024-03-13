#include "GameData.h"
#include <iostream>

struct FileItemStack {
	unsigned int item_id;
	int count;
};

void gamedata::loadItemInventory(ItemInventory* ic, const char* path)
{
	FILE* fp = nullptr;
	fp = fopen(path, "rb");
	if (!fp) return;
	long item_count;
	fseek(fp, 0, SEEK_END);
	item_count = (ftell(fp)) / sizeof(FileItemStack);
	FileItemStack* fis = new FileItemStack[item_count];
	rewind(fp);
	ic->clear();
	ic->reinit(item_count);
	fread(fis, sizeof(FileItemStack), item_count, fp);
	fclose(fp);
	for (int i = 0; i < item_count; i++) {
		if (fis[i].count) {
			ic->itemStackAt(i).set(gamedata::items.indexer[fis[i].item_id], fis[i].count);
		}
	}
	delete[] fis;
}

void gamedata::saveItemInventory(ItemInventory* ic, const char* path)
{
	int item_count = ic->getSize();
	FileItemStack* fis = new FileItemStack[item_count];
	for (int i = 0; i < item_count; i++) {
		if (!ic->itemStackAt(i).getItem()) {
			fis[i].count = fis[i].item_id = 0;
		}
		else {
			fis[i].item_id = ic->itemStackAt(i).getItem()->getID();
			fis[i].count = ic->itemStackAt(i).getCount();
		}	
	}
	FILE* fp = nullptr;
	fp = fopen(path, "wb");
	if (!fp) return;
	fwrite(fis, sizeof(FileItemStack), item_count, fp);
	fclose(fp);
	delete[] fis;
}

Block::Block(unsigned short int id, const char* nameptr, unsigned short int material, unsigned int display_texture, const Item* drop, int drop_count, bool is_storage)
{
	this->id = id;
	this->name_ptr = nameptr;
	this->material = material;
	this->display_texture = display_texture;
	texture_b = texture_c = 0;
	texture_a = display_texture;
	texture_mode = gamedata::TEXTUREMODE_SINGLE;
	breaking_time = 1.0f;
	this->drop_item = drop;
	this->drop_count = drop_count;
	this->is_storage = is_storage;
	model_type = 0;
	has_collision = renderable = touchable = true;
	has_transparency = false;
	harvestable = false;
}

const char* Block::getNamePtr() const
{
	return name_ptr;
}

unsigned short int Block::getID() const
{
	return id;
}

unsigned short int Block::getMaterial() const
{
	return material;
}

unsigned int Block::getDisplayTexture() const
{
	return display_texture;
}

Block::Block()
{
	name_ptr = nullptr;
	id = 0;
	material = 0;
	display_texture = 0;
	texture_mode = 0;
	texture_a = 0;
	texture_b = 0;
	texture_c = 0;
	breaking_time = 0.0f;
	drop_item = nullptr;
	drop_count = 0;
	is_storage = false;
	model_type = 0;
	has_collision = renderable = touchable = true;
	has_transparency = false;
	harvestable = false;
}

bool Block::isStorage() const
{
	return is_storage;
}

void Block::setTextureMode(int mode, unsigned int a, unsigned int b, unsigned int c)
{
	texture_mode = mode;
	texture_a = a;
	texture_b = b;
	texture_c = c;
}

unsigned int Block::getBlockTexture(int direction) const
{
	if (texture_mode == gamedata::TEXTUREMODE_SINGLE) {
		return texture_a;
	}
	else if (texture_mode == gamedata::TEXTUREMODE_TOP_SIDES) {
		if (direction == gamedata::DIRECTION_TOP || direction == gamedata::DIRECTION_BOTTOM)
			return texture_a;
		else
			return texture_b;
	}
	else if (texture_mode == gamedata::TEXTUREMODE_TOP_BOTTOM_SIDES) {
		if (direction == gamedata::DIRECTION_TOP)
			return texture_a;
		else if (direction == gamedata::DIRECTION_BOTTOM)
			return texture_b;
		else
			return texture_c;
	}
	return 0;
}

void Block::setBreaingTime(float seconds)
{
	breaking_time = seconds;
}

float Block::getBreakingTime() const
{
	return breaking_time;
}

const Item* Block::getDropItem() const
{
	return drop_item;
}

int Block::getDropCount() const
{
	return drop_count;
}

void Block::setModelType(int model_type)
{
	this->model_type = model_type;
}

int Block::getModelType() const
{
	return model_type;
}

void Block::setTransparency(bool has_transparency)
{
	this->has_transparency = has_transparency;
}

bool Block::hasTransparency() const
{
	return has_transparency;
}

void Block::setCollision(bool has_collision)
{
	this->has_collision = has_collision;
}

bool Block::hasCollision() const
{
	return has_collision;
}

void Block::setRenderable(bool is_renderable)
{
	this->renderable = is_renderable;
}

bool Block::isRenderable() const
{
	return renderable;
}

void Block::setTouchable(bool is_touchable)
{
	this->touchable = is_touchable;
}

bool Block::isTouchable() const
{
	return touchable;
}

void Block::setHarvestable(bool is_harvestable)
{
	harvestable = is_harvestable;
}

bool Block::isHarvestable() const
{
	return harvestable;
}

const unsigned short int* Block::getBlockID()
{
	return &id;
}

Item::Item(unsigned int id, const char* nameptr, unsigned int texture, int stack_size, const unsigned short int* id_of_block)
{
	this->id = id;
	this->nameptr = nameptr;
	this->texture = texture;
	this->stack_size = stack_size;
	this->id_of_block = id_of_block;
	eatable = 0;
	weapon = 0;
	tool = 0;
}

Item::Item()
{
	id = 0;
	texture = 0;
	id_of_block = 0;
	nameptr = nullptr;
	stack_size = 0;
	eatable = 0;
	weapon = 0;
	tool = 0;
}

unsigned int Item::getID() const
{
	return id;
}

unsigned int Item::getTexture() const
{
	return texture;
}

const char* Item::getName() const
{
	return nameptr;
}

int Item::getStackSize() const
{
	return stack_size;
}

unsigned short int Item::getIdOfBlock() const
{
	return *id_of_block;
}

bool Item::blockIDNotNull() const
{
	if (id_of_block) return true;
	return false;
}

void Item::setTool(int material)
{
	tool = material;
}

void Item::setEatable(int value)
{
	eatable = value;
}

void Item::setWeapon(int damage)
{
	weapon = damage;
}

int Item::getToolMaterial() const
{
	return tool;
}

int Item::getEatValue() const
{
	return eatable;
}

int Item::getWeaponDamage() const
{
	return weapon;
}

ItemStack::ItemStack() {
	this->item = nullptr;
	this->count = 0;
}

ItemStack::ItemStack(const Item* item, int count) {
	this->item = item;
	this->count = count;
}

void ItemStack::set(const Item* item, int count) {
	this->item = item;
	this->count = count;
}

const Item* ItemStack::getItem() const {
	return item;
}

int ItemStack::getCount() const {
	return count;
}

bool ItemStack::isEmpty() const {
	return count == 0;
}

int ItemStack::addCount(int count, bool clear_if_empty) {
	this->count += count;
	if (this->count > item->getStackSize()) {
		int respond = (this->count - item->getStackSize());
		this->count = item->getStackSize();
		return respond;
	}
	else if (this->count < 0) {
		int respond = this->count;
		this->count = 0;
		if (clear_if_empty)
			clear();
		return respond;
	}
	if (clear_if_empty && this->count == 0)
		clear();
	return 0;
}

void ItemStack::clear() {
	item = nullptr;
	count = 0;
}

ItemInventory::ItemInventory(int inventory_size) {
	this->size = inventory_size;
	this->content = new ItemStack[this->size];
	this->initialized = true;
}

ItemInventory::ItemInventory() {
	this->initialized = false;
	this->content = nullptr;
	this->size = 0;
}

ItemInventory::~ItemInventory() {
	if(content)
		delete[] content;
}

bool ItemInventory::init(int inventory_size) {
	if (this->initialized) return false;
	this->size = inventory_size;
	this->content = new ItemStack[this->size];
	this->initialized = true;
	return true;
}

bool ItemInventory::reinit(int new_size) {
	if (this->initialized) {
		delete[] content;
		initialized = false;
	}
	this->size = new_size;
	this->content = new ItemStack[this->size];
	this->initialized = true;
	return true;
}

void ItemInventory::clear() {
	for (int i = 0; i < this->size; i++) {
		this->content[i].clear();
	}
}

int ItemInventory::getSize() {
	return size;
}

int ItemInventory::addItem(const Item* item, int count1) {
	int to_add = count1;
	for (int i = 0; i < this->size; i++) {
		if (this->content[i].getItem() && this->content[i].getItem()->getID() == item->getID()) {
			int space = item->getStackSize() - this->content[i].getCount();
			if (to_add > space) {
				this->content[i].addCount(space);
				to_add -= space;
			}
			else {
				this->content[i].addCount(to_add);
				return 0;
			}
		}
	}
	for (int i = 0; i < this->size; i++) {
		if (this->content[i].getItem() == nullptr) {
			int space = item->getStackSize();
			if (to_add > space) {
				this->content[i] = ItemStack(item, space);
				to_add -= space;
			}
			else {
				this->content[i] = ItemStack(item, to_add);
				return 0;
			}
		}
	}
	return to_add;
}

ItemStack& ItemInventory::itemStackAt(int i) {
	return content[i];
}

ItemStack* ItemInventory::getPointer() {
	return content;
}

void ItemInventory::swap(int ia, int ib)
{
	ItemStack temp = content[ia];
	content[ia] = content[ib];
	content[ib] = temp;
}

Recipe::Recipe() {
	for (int i = 0; i < 9; i++) {
		input[i] = new ItemStack();
		output[i] = new ItemStack();
	}
}

void Recipe::setInput(int idx, const Item* item, int count) {
	input[idx]->set(item, count);
}

void Recipe::setOutput(int idx, const Item* item, int count) {
	output[idx]->set(item, count);
}

bool Recipe::inputMatches(ItemStack* board) {
	for (int i = 0; i < 9; i++) {
		if (!board[i].getItem() && !input[i]->getItem())
			continue;
		if (!board[i].getItem() || !input[i]->getItem())
			return false;
		if (board[i].getItem() != input[i]->getItem())
			return false;
		if (board[i].getCount() < input[i]->getCount())
			return false;
	}
	return true;
}

bool Recipe::outputMatches(ItemStack* board) {
	for (int i = 0; i < 9; i++) {
		if (!board[i].getItem() && !output[i]->getItem())
			continue;
		if (!board[i].getItem() || !output[i]->getItem())
			return false;
		if (board[i].getItem() != output[i]->getItem())
			return false;
		if (board[i].getCount() < output[i]->getCount())
			return false;
	}
	return true;
}

ItemStack* Recipe::getOutput(int idx) {
	return output[idx];
}

void Recipe::fillOutput(ItemStack* board) {
	for (int i = 0; i < 9; i++) {
		if (output[i]->isEmpty())
			board[i].clear();
		else
			board[i].set(output[i]->getItem(), output[i]->getCount());
	}
}

void Recipe::subInput(ItemStack* board) {
	for (int i = 0; i < 9; i++) {
		if (input[i]->isEmpty())
			continue;
		if (!board[i].isEmpty())
			board[i].addCount(-1, true);
	}
}

Recipes::Recipes() {
	len = 0;
	list = new Recipe[gamedata::INDEXER_LIMIT];
	int auto_add = 0;

	list[auto_add].setInput(0, &gamedata::items.spruce_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.spruce_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.pine_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.pine_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.maple_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.maple_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.birch_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.birch_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.dwarf_birch_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.birch_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.acacia_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.acacia_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.jungle_log, 1);
	list[auto_add].setOutput(0, &gamedata::items.jungle_planks, 4);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.spruce_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.spruce_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.pine_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.birch_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.birch_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.acacia_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.acacia_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.maple_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.maple_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.jungle_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.jungle_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.stick, 8);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(1, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(2, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(4, &gamedata::items.stick, 1);
	list[auto_add].setInput(7, &gamedata::items.stick, 1);
	list[auto_add].setOutput(0, &gamedata::items.stone_picaxe, 1);
	auto_add++;

	list[auto_add].setInput(1, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(4, &gamedata::items.stick, 1);
	list[auto_add].setInput(7, &gamedata::items.stick, 1);
	list[auto_add].setOutput(0, &gamedata::items.stone_shovel, 1);
	auto_add++;

	list[auto_add].setInput(1, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(4, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(7, &gamedata::items.stick, 1);
	list[auto_add].setOutput(0, &gamedata::items.stone_sword, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(1, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(4, &gamedata::items.stick, 1);
	list[auto_add].setInput(7, &gamedata::items.stick, 1);
	list[auto_add].setOutput(0, &gamedata::items.stone_hoe, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(1, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(3, &gamedata::items.cobblestone, 1);
	list[auto_add].setInput(4, &gamedata::items.stick, 1);
	list[auto_add].setInput(7, &gamedata::items.stick, 1);
	list[auto_add].setOutput(0, &gamedata::items.stone_axe, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.strawberry, 1);
	list[auto_add].setOutput(0, &gamedata::items.strawberry_seeds, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.blueberry, 1);
	list[auto_add].setOutput(0, &gamedata::items.dwarf_blueberry_seeds, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.bearberry, 1);
	list[auto_add].setOutput(0, &gamedata::items.bearberry_seeds, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.spruce_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.spruce_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.pine_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.pine_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.dwarf_birch_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.dwarf_birch_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.maple_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.maple_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.birch_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.birch_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.jungle_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.jungle_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.acacia_leaves, 1);
	list[auto_add].setOutput(0, &gamedata::items.grasses, 3);
	list[auto_add].setOutput(1, &gamedata::items.acacia_sapling, 1);
	auto_add++;

	list[auto_add].setInput(0, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(1, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(2, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(3, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(5, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(6, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(7, &gamedata::items.pine_planks, 1);
	list[auto_add].setInput(8, &gamedata::items.pine_planks, 1);
	list[auto_add].setOutput(0, &gamedata::items.barrel, 1);
	auto_add++;

	len = auto_add;
}

ItemData::ItemData()
{
	int auto_add = 0;
	const unsigned short int* no_block = nullptr;
	indexer = new Item * [gamedata::INDEXER_LIMIT];

	indexer[auto_add] = &stone;
	stone = Item(auto_add, "Stone", gamedata::TEXTURE_STONE, 64, gamedata::blocks.stone.getBlockID());
	auto_add++;

	indexer[auto_add] = &cobblestone;
	cobblestone = Item(auto_add, "Cobblestone", gamedata::TEXTURE_COBBLESTONE, 64, gamedata::blocks.cobblestone.getBlockID());
	auto_add++;

	indexer[auto_add] = &dirt;
	dirt = Item(auto_add, "Dirt", gamedata::TEXTURE_DIRT, 64, gamedata::blocks.dirt.getBlockID());
	auto_add++;

	indexer[auto_add] = &grass;
	grass = Item(auto_add, "Grass", gamedata::TEXTURE_GRASS_SIDE, 64, gamedata::blocks.grass.getBlockID());
	auto_add++;

	indexer[auto_add] = &spruce_log;
	spruce_log = Item(auto_add, "Spruce Log", gamedata::TEXTURE_SPRUCE_LOG, 64, gamedata::blocks.spruce_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &pine_log;
	pine_log = Item(auto_add, "Pine Log", gamedata::TEXTURE_PINE_LOG, 64, gamedata::blocks.pine_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &dwarf_birch_log;
	dwarf_birch_log = Item(auto_add, "Dwarf Birch Log", gamedata::TEXTURE_DWARFBIRCH_LOG, 64, gamedata::blocks.dwarf_birch_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &maple_log;
	maple_log = Item(auto_add, "Maple Log", gamedata::TEXTURE_MAPLE_LOG, 64, gamedata::blocks.maple_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &birch_log;
	birch_log = Item(auto_add, "Birch Log", gamedata::TEXTURE_BIRCH_LOG, 64, gamedata::blocks.birch_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &acacia_log;
	acacia_log = Item(auto_add, "Acacia Log", gamedata::TEXTURE_ACACIA_LOG, 64, gamedata::blocks.acacia_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &jungle_log;
	jungle_log = Item(auto_add, "Jungle Log", gamedata::TEXTURE_JUNGLE_LOG, 64, gamedata::blocks.jungle_log.getBlockID());
	auto_add++;

	indexer[auto_add] = &spruce_leaves;
	spruce_leaves = Item(auto_add, "Spruce Leaves", gamedata::TEXTURE_SPRUCE_LEAF, 64, gamedata::blocks.spruce_leaves.getBlockID());
	auto_add++;

	indexer[auto_add] = &pine_leaves;
	pine_leaves = Item(auto_add, "Pine Leaves", gamedata::TEXTURE_PINE_LEAF, 64, gamedata::blocks.pine_leaves.getBlockID());
	auto_add++;

	indexer[auto_add] = &dwarf_birch_leaves;
	dwarf_birch_leaves = Item(auto_add, "Dwarf Birch Leaves", gamedata::TEXTURE_DWARFBIRCH_LEAF, 64, gamedata::blocks.dwarf_birch_leaves.getBlockID());
	auto_add++;

	indexer[auto_add] = &maple_leaves;
	maple_leaves = Item(auto_add, "Maple Leaves", gamedata::TEXTURE_MAPLE_LEAF_GREEN, 64, gamedata::blocks.maple_leaves_green_y.getBlockID());
	auto_add++;

	indexer[auto_add] = &birch_leaves;
	birch_leaves = Item(auto_add, "Brich Leaves", gamedata::TEXTURE_BIRCH_LEAF, 64, gamedata::blocks.birch_leaves_green_y.getBlockID());
	auto_add++;

	indexer[auto_add] = &acacia_leaves;
	acacia_leaves = Item(auto_add, "Acacia Leaves", gamedata::TEXTURE_ACACIA_LEAF, 64, gamedata::blocks.acacia_leaves.getBlockID());
	auto_add++;

	indexer[auto_add] = &jungle_leaves;
	jungle_leaves = Item(auto_add, "Jungle Leaves", gamedata::TEXTURE_JUNGLE_LEAF, 64, gamedata::blocks.jungle_leaves.getBlockID());
	auto_add++;

	indexer[auto_add] = &tundra_soil;
	tundra_soil = Item(auto_add, "Tundra Soil", gamedata::TEXTURE_TUNDRA_SOIL, 64, gamedata::blocks.tundra_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &taiga_soil;
	taiga_soil = Item(auto_add, "Taiga Soil", gamedata::TEXTURE_TAIGA_SOIL, 64, gamedata::blocks.taiga_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &deciduous_forest_soil;
	deciduous_forest_soil = Item(auto_add, "Deciduous Forest Soil", gamedata::TEXTURE_DECIDUOUS_FOREST_SOIL, 64, gamedata::blocks.deciduous_forest_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &grassland_soil;
	grassland_soil = Item(auto_add, "Grassland Soil", gamedata::TEXTURE_GRASSLAND_SOIL, 64, gamedata::blocks.grassland_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &dessert_soil;
	dessert_soil = Item(auto_add, "Dessert Soil", gamedata::TEXTURE_DESSERT_SOIL, 64, gamedata::blocks.dessert_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &savanna_soil;
	savanna_soil = Item(auto_add, "Savanna Soil", gamedata::TEXTURE_SAVANNA_SOIL, 64, gamedata::blocks.savanna_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &tropical_soil;
	tropical_soil = Item(auto_add, "Tropical Soil", gamedata::TEXTURE_TROPICAL_SOIL, 64, gamedata::blocks.tropical_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &chaparral_soil;
	chaparral_soil = Item(auto_add, "Chaparral Soil", gamedata::TEXTURE_CHAPARRAL_SOIL, 64, gamedata::blocks.chaparral_soil.getBlockID());
	auto_add++;

	indexer[auto_add] = &barrel;
	barrel = Item(auto_add, "Barrel", gamedata::TEXTURE_BARREL_SIDE, 64, gamedata::blocks.barrel.getBlockID());
	auto_add++;

	indexer[auto_add] = &spruce_planks;
	spruce_planks = Item(auto_add, "Spruce Planks", gamedata::TEXTURE_SPRUCE_PLANKS, 64, gamedata::blocks.spruce_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &pine_planks;
	pine_planks = Item(auto_add, "Pine Planks", gamedata::TEXTURE_PINE_PLANKS, 64, gamedata::blocks.pine_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &birch_planks;
	birch_planks = Item(auto_add, "Birch Planks", gamedata::TEXTURE_BIRCH_PLANKS, 64, gamedata::blocks.birch_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &maple_planks;
	maple_planks = Item(auto_add, "Maple Planks", gamedata::TEXTURE_MAPLE_PLANKS, 64, gamedata::blocks.maple_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &acacia_planks;
	acacia_planks = Item(auto_add, "Acacia Planks", gamedata::TEXTURE_ACACIA_PLANKS, 64, gamedata::blocks.acacia_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &jungle_planks;
	jungle_planks = Item(auto_add, "Jungle Planks", gamedata::TEXTURE_JUNGLE_PLANKS, 64, gamedata::blocks.jungle_planks.getBlockID());
	auto_add++;

	indexer[auto_add] = &grasses;
	grasses = Item(auto_add, "Organic Scraps", gamedata::TEXTURE_PLANT_SCRAPS, 64, no_block);
	auto_add++;

	indexer[auto_add] = &stick;
	stick = Item(auto_add, "Stick", gamedata::TEXTURE_STICK, 64, no_block);
	auto_add++;

	indexer[auto_add] = &stone_picaxe;
	stone_picaxe = Item(auto_add, "Stone Pickaxe", gamedata::TEXTURE_STONE_PICKAXE, 1, no_block);
	stone_picaxe.setTool(gamedata::MATERIAL_ROCK);
	auto_add++;

	indexer[auto_add] = &stone_shovel;
	stone_shovel = Item(auto_add, "Stone Shovel", gamedata::TEXTURE_STONE_SHOVEL, 1, no_block);
	stone_shovel.setTool(gamedata::MATERIAL_SOIL);
	auto_add++;

	indexer[auto_add] = &stone_sword;
	stone_sword = Item(auto_add, "Stone Sword", gamedata::TEXTURE_STONE_SWORD, 1, no_block);
	stone_sword.setWeapon(5);
	auto_add++;

	indexer[auto_add] = &stone_hoe;
	stone_hoe = Item(auto_add, "Stone Hoe", gamedata::TEXTURE_STONE_HOE, 1, no_block);
	stone_hoe.setTool(10);
	auto_add++;

	indexer[auto_add] = &stone_axe;
	stone_axe = Item(auto_add, "Stone Axe", gamedata::TEXTURE_STONE_AXE, 1, no_block);
	stone_axe.setTool(gamedata::MATERIAL_WOOD);
	auto_add++;

	indexer[auto_add] = &strawberry;
	strawberry = Item(auto_add, "Strawberry", gamedata::TEXTURE_STRAWBERRY, 64, no_block);
	strawberry.setEatable(1);
	auto_add++;

	indexer[auto_add] = &strawberry_seeds;
	strawberry_seeds = Item(auto_add, "Strawberry Seeds", gamedata::TEXTURE_STRAWBERRY_SEEDS, 64, gamedata::blocks.strawberry_bush.getBlockID());
	auto_add++;

	indexer[auto_add] = &blueberry;
	blueberry = Item(auto_add, "Blueberry", gamedata::TEXTURE_BLUEBERRY, 64, no_block);
	blueberry.setEatable(1);
	auto_add++;

	indexer[auto_add] = &dwarf_blueberry_seeds;
	dwarf_blueberry_seeds = Item(auto_add, "Dwarf Blueberry Seeds", gamedata::TEXTURE_BLUEBERRY_SEEDS, 64, gamedata::blocks.dwarf_blueberry_bush.getBlockID());
	auto_add++;

	indexer[auto_add] = &bearberry;
	bearberry = Item(auto_add, "Bearberry", gamedata::TEXTURE_BEARBERRY, 64, no_block);
	bearberry.setEatable(1);
	auto_add++;

	indexer[auto_add] = &bearberry_seeds;
	bearberry_seeds = Item(auto_add, "Bearberry Seeds", gamedata::TEXTURE_BEARBERRY_SEEDS, 64, gamedata::blocks.bearberry_bush.getBlockID());
	auto_add++;

	indexer[auto_add] = &dwarf_birch_sapling;
	dwarf_birch_sapling = Item(auto_add, "Dwarf Birch Sapling", gamedata::TEXTURE_DWARF_BIRCH_SAPLING, 64, gamedata::blocks.dwarf_brich_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &spruce_sapling;
	spruce_sapling = Item(auto_add, "Spruce Sapling", gamedata::TEXTURE_SPRUCE_SAPLING, 64, gamedata::blocks.spruce_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &pine_sapling;
	pine_sapling = Item(auto_add, "Pine Sapling", gamedata::TEXTURE_PINE_SAPLING, 64, gamedata::blocks.pine_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &maple_sapling;
	maple_sapling = Item(auto_add, "Maple Sapling", gamedata::TEXTURE_MAPLE_SAPLING, 64, gamedata::blocks.maple_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &birch_sapling;
	birch_sapling = Item(auto_add, "Birch Sapling", gamedata::TEXTURE_BIRCH_SAPLING, 64, gamedata::blocks.birch_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &acacia_sapling;
	acacia_sapling = Item(auto_add, "Acacia Sapling", gamedata::TEXTURE_ACACIA_SAPLING, 64, gamedata::blocks.acacia_sapling.getBlockID());
	auto_add++;

	indexer[auto_add] = &jungle_sapling;
	jungle_sapling = Item(auto_add, "Jungle Sapling", gamedata::TEXTURE_JUNGLE_SAPLING, 64, gamedata::blocks.jungle_sapling.getBlockID());
	auto_add++;

}

BlockData::BlockData()
{
	int auto_add = 0;
	indexer = new Block * [gamedata::INDEXER_LIMIT];

	// Air
	indexer[auto_add] = &air;
	air = Block(auto_add, "Air", 0, gamedata::TEXTURE_NULL, nullptr);
	air.setRenderable(false);
	air.setTransparency(true);
	air.setCollision(false);
	air.setTouchable(false);
	auto_add++;

	// Stone
	indexer[auto_add] = &stone;
	stone = Block(auto_add, "Stone", gamedata::MATERIAL_ROCK, gamedata::TEXTURE_STONE, &gamedata::items.cobblestone);
	stone.setBreaingTime(4.0f);
	auto_add++;

	// Cobblestone
	indexer[auto_add] = &cobblestone;
	cobblestone = Block(auto_add, "Cobblestone", gamedata::MATERIAL_ROCK, gamedata::TEXTURE_COBBLESTONE, &gamedata::items.cobblestone);
	cobblestone.setBreaingTime(4.0f);
	auto_add++;

	// Dirt
	indexer[auto_add] = &dirt;
	dirt = Block(auto_add, "Dirt", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_DIRT, &gamedata::items.dirt);
	dirt.setBreaingTime(1.0f);
	auto_add++;

	// Grass
	indexer[auto_add] = &grass;
	grass = Block(auto_add, "Grass", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_GRASS_SIDE, &gamedata::items.dirt);
	grass.setBreaingTime(1.0f);
	grass.setTextureMode(gamedata::TEXTUREMODE_TOP_BOTTOM_SIDES, gamedata::TEXTURE_GRASS_TOP, gamedata::TEXTURE_DIRT, gamedata::TEXTURE_GRASS_SIDE);
	auto_add++;

	// Spruce Log
	indexer[auto_add] = &spruce_log;
	spruce_log = Block(auto_add, "Spruce Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_SPRUCE_LOG, &gamedata::items.spruce_log);
	spruce_log.setBreaingTime(3.0f);
	spruce_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_SPRUCE_LOG_TOP, gamedata::TEXTURE_SPRUCE_LOG);
	auto_add++;

	// Pine Log
	indexer[auto_add] = &pine_log;
	pine_log = Block(auto_add, "Pine Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_PINE_LOG, &gamedata::items.pine_log);
	pine_log.setBreaingTime(3.0f);
	pine_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_PINE_LOG_TOP, gamedata::TEXTURE_PINE_LOG);
	auto_add++;

	// Dwarf Birch Log
	indexer[auto_add] = &dwarf_birch_log;
	dwarf_birch_log = Block(auto_add, "Dwarf Birch Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_DWARFBIRCH_LOG, &gamedata::items.dwarf_birch_log);
	dwarf_birch_log.setBreaingTime(3.0f);
	dwarf_birch_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_DWARFBIRCH_LOG_TOP, gamedata::TEXTURE_DWARFBIRCH_LOG);
	auto_add++;

	// Maple Log
	indexer[auto_add] = &maple_log;
	maple_log = Block(auto_add, "Maple Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_MAPLE_LOG, &gamedata::items.maple_log);
	maple_log.setBreaingTime(3.0f);
	maple_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_MAPLE_LOG_TOP, gamedata::TEXTURE_MAPLE_LOG);
	auto_add++;

	// Birch Log
	indexer[auto_add] = &birch_log;
	birch_log = Block(auto_add, "Birch Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_BIRCH_LOG, &gamedata::items.birch_log);
	birch_log.setBreaingTime(3.0f);
	birch_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_BIRCH_LOG_TOP, gamedata::TEXTURE_BIRCH_LOG);
	auto_add++;

	// Acacia Log
	indexer[auto_add] = &acacia_log;
	acacia_log = Block(auto_add, "Acacia Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_ACACIA_LOG, &gamedata::items.acacia_log);
	acacia_log.setBreaingTime(3.0f);
	acacia_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_ACACIA_LOG_TOP, gamedata::TEXTURE_ACACIA_LOG);
	auto_add++;

	// Jungle Log
	indexer[auto_add] = &jungle_log;
	jungle_log = Block(auto_add, "Jungle Log", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_JUNGLE_LOG, &gamedata::items.jungle_log);
	jungle_log.setBreaingTime(3.0f);
	jungle_log.setTextureMode(gamedata::TEXTUREMODE_TOP_SIDES, gamedata::TEXTURE_JUNGLE_LOG_TOP, gamedata::TEXTURE_JUNGLE_LOG);
	auto_add++;

	// Spruce Leaf
	indexer[auto_add] = &spruce_leaves;
	spruce_leaves = Block(auto_add, "Spruce Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_SPRUCE_LEAF, &gamedata::items.spruce_leaves);
	spruce_leaves.setBreaingTime(0.5f);
	auto_add++;

	// Spruce Leaf Frozen
	indexer[auto_add] = &spruce_leaves_frosty;
	spruce_leaves_frosty = Block(auto_add, "Frosty Spruce Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_SPRUCE_LEAF_FROZEN, &gamedata::items.spruce_leaves);
	spruce_leaves_frosty.setBreaingTime(0.5f);
	auto_add++;

	// Pine Leaf
	indexer[auto_add] = &pine_leaves;
	pine_leaves = Block(auto_add, "Pine Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_PINE_LEAF, &gamedata::items.pine_leaves);
	pine_leaves.setBreaingTime(0.5f);
	auto_add++;

	// Pine Leaf Frozen
	indexer[auto_add] = &pine_leaves_frosty;
	pine_leaves_frosty = Block(auto_add, "Frosty Pine Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_PINE_LEAF_FROZEN, &gamedata::items.pine_leaves);
	pine_leaves_frosty.setBreaingTime(0.5f);
	auto_add++;

	// Dwarf Birch Leaf
	indexer[auto_add] = &dwarf_birch_leaves;
	dwarf_birch_leaves = Block(auto_add, "Dwarf Birch Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARFBIRCH_LEAF, &gamedata::items.dwarf_birch_leaves);
	dwarf_birch_leaves.setBreaingTime(0.5f);
	auto_add++;

	// Dwarf Birch Leaf Frozen
	indexer[auto_add] = &dwarf_birch_leaves_frosty;
	dwarf_birch_leaves_frosty = Block(auto_add, "Frosty Dwarf Birch Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARFBIRCH_LEAF_FROZEN, &gamedata::items.dwarf_birch_leaves);
	dwarf_birch_leaves_frosty.setBreaingTime(0.5f);
	auto_add++;

	// MAPLE TREE

	// Maple Leaf #GY: Green Leaves (Yellow in Fall)
	indexer[auto_add] = &maple_leaves_green_y;
	maple_leaves_green_y = Block(auto_add, "Maple Green Leaves (Yellow)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_GREEN, &gamedata::items.maple_leaves);
	maple_leaves_green_y.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #Y: Yellow Leaves
	indexer[auto_add] = &maple_leaves_yellow;
	maple_leaves_yellow = Block(auto_add, "Maple Yellow Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_YELLOW, &gamedata::items.maple_leaves);
	maple_leaves_yellow.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #WY: Winter Leaves (Yellow in Fall)
	indexer[auto_add] = &maple_leaves_winter_y;
	maple_leaves_winter_y = Block(auto_add, "Maple Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_WINTER, &gamedata::items.maple_leaves);
	maple_leaves_winter_y.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #GO: Green Leaves (Orange in Fall)
	indexer[auto_add] = &maple_leaves_green_o;
	maple_leaves_green_o = Block(auto_add, "Maple Green Leaves (Orange)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_GREEN, &gamedata::items.maple_leaves);
	maple_leaves_green_o.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #O: Orange Leaves
	indexer[auto_add] = &maple_leaves_orange;
	maple_leaves_orange = Block(auto_add, "Maple Orange Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_ORANGE, &gamedata::items.maple_leaves);
	maple_leaves_orange.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #WO: Winter Leaves (Orange in Fall)
	indexer[auto_add] = &maple_leaves_winter_o;
	maple_leaves_winter_o = Block(auto_add, "Maple Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_WINTER, &gamedata::items.maple_leaves);
	maple_leaves_winter_o.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #GR: Green Leaves (Red in Fall)
	indexer[auto_add] = &maple_leaves_green_r;
	maple_leaves_green_r = Block(auto_add, "Maple Green Leaves (Red)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_GREEN, &gamedata::items.maple_leaves);
	maple_leaves_green_r.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #R: Red Leaves
	indexer[auto_add] = &maple_leaves_red;
	maple_leaves_red = Block(auto_add, "Maple Red Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_RED, &gamedata::items.maple_leaves);
	maple_leaves_red.setBreaingTime(0.5f);
	auto_add++;

	// Maple Leaf #WR: Winter Leaves (Red in Fall)
	indexer[auto_add] = &maple_leaves_winter_r;
	maple_leaves_winter_r = Block(auto_add, "Maple Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_LEAF_WINTER, &gamedata::items.maple_leaves);
	maple_leaves_winter_r.setBreaingTime(0.5f);
	auto_add++;

	// BIRCH TREE

	// Birch Leaf #GY: Green Leaves (Yellow in Fall)
	indexer[auto_add] = &birch_leaves_green_y;
	birch_leaves_green_y = Block(auto_add, "Birch Green Leaves (Yellow)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF, &gamedata::items.birch_leaves);
	birch_leaves_green_y.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #Y: Yellow Leaves
	indexer[auto_add] = &birch_leaves_yellow;
	birch_leaves_yellow = Block(auto_add, "Birch Yellow Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_YELLOW, &gamedata::items.birch_leaves);
	birch_leaves_yellow.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #WY: Winter Leaves (Yellow in Fall)
	indexer[auto_add] = &birch_leaves_winter_y;
	birch_leaves_winter_y = Block(auto_add, "Birch Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_WINTER, &gamedata::items.birch_leaves);
	birch_leaves_winter_y.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #GO: Green Leaves (Orange in Fall)
	indexer[auto_add] = &birch_leaves_green_o;
	birch_leaves_green_o = Block(auto_add, "Birch Green Leaves (Orange)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF, &gamedata::items.birch_leaves);
	birch_leaves_green_o.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #O: Orange Leaves
	indexer[auto_add] = &birch_leaves_orange;
	birch_leaves_orange = Block(auto_add, "Birch Orange Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_ORANGE, &gamedata::items.birch_leaves);
	birch_leaves_orange.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #WO: Winter Leaves (Orange in Fall)
	indexer[auto_add] = &birch_leaves_winter_o;
	birch_leaves_winter_o = Block(auto_add, "Birch Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_WINTER, &gamedata::items.birch_leaves);
	birch_leaves_winter_o.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #GR: Green Leaves (Red in Fall)
	indexer[auto_add] = &birch_leaves_green_r;
	birch_leaves_green_r = Block(auto_add, "Birch Green Leaves (Red)", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF, &gamedata::items.birch_leaves);
	birch_leaves_green_r.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #R: Red Leaves
	indexer[auto_add] = &birch_leaves_red;
	birch_leaves_red = Block(auto_add, "Birch Red Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_RED, &gamedata::items.birch_leaves);
	birch_leaves_red.setBreaingTime(0.5f);
	auto_add++;

	// Birch Leaf #WR: Winter Leaves (Red in Fall)
	indexer[auto_add] = &birch_leaves_winter_r;
	birch_leaves_winter_r = Block(auto_add, "Birch Branches", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_LEAF_WINTER, &gamedata::items.birch_leaves);
	birch_leaves_winter_r.setBreaingTime(0.5f);
	auto_add++;

	// Acacia Leaf
	indexer[auto_add] = &acacia_leaves;
	acacia_leaves = Block(auto_add, "Acacia Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_ACACIA_LEAF, &gamedata::items.acacia_leaves);
	acacia_leaves.setBreaingTime(0.5f);
	auto_add++;

	// Jungle Leaf
	indexer[auto_add] = &jungle_leaves;
	jungle_leaves = Block(auto_add, "Jungle Leaves", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_JUNGLE_LEAF, &gamedata::items.jungle_leaves);
	jungle_leaves.setBreaingTime(0.5f);
	auto_add++;

	// SOILS

	// Tundra Soil
	indexer[auto_add] = &tundra_soil;
	tundra_soil = Block(auto_add, "Tundra Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_TUNDRA_SOIL, &gamedata::items.tundra_soil);
	tundra_soil.setBreaingTime(1.5f);
	auto_add++;

	// Taiga Soil
	indexer[auto_add] = &taiga_soil;
	taiga_soil = Block(auto_add, "Taiga Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_TAIGA_SOIL, &gamedata::items.taiga_soil);
	taiga_soil.setBreaingTime(1.25f);
	auto_add++;

	// Deciduous Forest Soil
	indexer[auto_add] = &deciduous_forest_soil;
	deciduous_forest_soil = Block(auto_add, "Deciduous Forest Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_DECIDUOUS_FOREST_SOIL, &gamedata::items.deciduous_forest_soil);
	deciduous_forest_soil.setBreaingTime(1.0f);
	auto_add++;

	// Grassland Soil
	indexer[auto_add] = &grassland_soil;
	grassland_soil = Block(auto_add, "Grassland Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_GRASSLAND_SOIL, &gamedata::items.grassland_soil);
	grassland_soil.setBreaingTime(1.0f);
	auto_add++;

	// Dessert Soil (Sand)
	indexer[auto_add] = &dessert_soil;
	dessert_soil = Block(auto_add, "Dessert Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_DESSERT_SOIL, &gamedata::items.dessert_soil);
	dessert_soil.setBreaingTime(0.75f);
	auto_add++;

	// Savanna Soil
	indexer[auto_add] = &savanna_soil;
	savanna_soil = Block(auto_add, "Savanna Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_SAVANNA_SOIL, &gamedata::items.savanna_soil);
	savanna_soil.setBreaingTime(1.0f);
	auto_add++;

	// Tropical Forest Soil
	indexer[auto_add] = &tropical_soil;
	tropical_soil = Block(auto_add, "Tropical Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_TROPICAL_SOIL, &gamedata::items.tropical_soil);
	tropical_soil.setBreaingTime(1.0f);
	auto_add++;

	// Chaparral Soil ()
	indexer[auto_add] = &chaparral_soil;
	chaparral_soil = Block(auto_add, "Chaparral Soil", gamedata::MATERIAL_SOIL, gamedata::TEXTURE_CHAPARRAL_SOIL, &gamedata::items.chaparral_soil);
	chaparral_soil.setBreaingTime(1.0f);
	auto_add++;

	//

	// Barrel (Item Container)
	indexer[auto_add] = &barrel;
	barrel = Block(auto_add, "Barrel", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_BARREL_SIDE, &gamedata::items.barrel, 1, true);
	barrel.setBreaingTime(3.0f);
	barrel.setTextureMode(gamedata::TEXTUREMODE_TOP_BOTTOM_SIDES, gamedata::TEXTURE_BARREL_TOP, gamedata::TEXTURE_BARREL_BOT, gamedata::TEXTURE_BARREL_SIDE);
	auto_add++;

	// Spruce Planks
	indexer[auto_add] = &spruce_planks;
	spruce_planks = Block(auto_add, "Spruce Planks", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_SPRUCE_PLANKS, &gamedata::items.spruce_planks);
	spruce_planks.setBreaingTime(2.0f);
	auto_add++;

	// Pine Planks
	indexer[auto_add] = &pine_planks;
	pine_planks = Block(auto_add, "Pine Planks", gamedata::MATERIAL_WOOD, gamedata::TEXTURE_PINE_PLANKS, &gamedata::items.pine_planks);
	pine_planks.setBreaingTime(2.0f);
	auto_add++;

	// Water
	indexer[auto_add] = &water;
	water = Block(auto_add, "Water", NULL, gamedata::TEXTURE_WATER, nullptr);
	water.setModelType(gamedata::MODEL_LIQUID);
	water.setBreaingTime(1000000.0f);
	water.setCollision(false);
	water.setTransparency(true);
	auto_add++;

	// Tallgrass (Common)
	indexer[auto_add] = &tallgrass;
	tallgrass = Block(auto_add, "Tallgrass", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_TALLGRASS_A, &gamedata::items.grasses);
	tallgrass.setBreaingTime(0.15f);
	tallgrass.setCollision(false);
	tallgrass.setTransparency(true);
	tallgrass.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Tallgrass (Dead)
	indexer[auto_add] = &tallgrass_dead;
	tallgrass_dead = Block(auto_add, "Dead Tallgrass", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_TALLGRASS_DRY_A, &gamedata::items.grasses);
	tallgrass_dead.setBreaingTime(0.15f);
	tallgrass_dead.setCollision(false);
	tallgrass_dead.setTransparency(true);
	tallgrass_dead.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Tallgrass (Shorter)
	indexer[auto_add] = &tallgrass_short;
	tallgrass_short = Block(auto_add, "Short Tallgrass", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_TALLGRASS_B, &gamedata::items.grasses);
	tallgrass_short.setBreaingTime(0.15f);
	tallgrass_short.setCollision(false);
	tallgrass_short.setTransparency(true);
	tallgrass_short.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Tallgrass (Shorter Dead)
	indexer[auto_add] = &tallgrass_short_dead;
	tallgrass_short_dead = Block(auto_add, "Dead Short Tallgrass", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_TALLGRASS_DRY_B, &gamedata::items.grasses);
	tallgrass_short_dead.setBreaingTime(0.15f);
	tallgrass_short_dead.setCollision(false);
	tallgrass_short_dead.setTransparency(true);
	tallgrass_short_dead.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Strawberry Bush (Normal, No Fruit)
	indexer[auto_add] = &strawberry_bush;
	strawberry_bush = Block(auto_add, "Strawberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_STRAWBERRY_BUSH_NORMAL, &gamedata::items.strawberry_seeds, 1);
	strawberry_bush.setBreaingTime(0.65f);
	strawberry_bush.setCollision(false);
	strawberry_bush.setTransparency(true);
	strawberry_bush.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Strawberry Bush (Fruits Grown)
	indexer[auto_add] = &strawberry_bush_fruit;
	strawberry_bush_fruit = Block(auto_add, "Strawberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_STRAWBERRY_BUSH_FRUIT, &gamedata::items.strawberry, 4);
	strawberry_bush_fruit.setBreaingTime(0.65f);
	strawberry_bush_fruit.setCollision(false);
	strawberry_bush_fruit.setTransparency(true);
	strawberry_bush_fruit.setModelType(gamedata::MODEL_PLANT_2FACE);
	strawberry_bush_fruit.setHarvestable(true);
	auto_add++;

	// Strawberry Bush (Dead / Winter Stats)
	indexer[auto_add] = &strawberry_bush_dead;
	strawberry_bush_dead = Block(auto_add, "Strawberry Dead Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_STRAWBERRY_BUSH_DRY, &gamedata::items.grasses, 3);
	strawberry_bush_dead.setBreaingTime(0.65f);
	strawberry_bush_dead.setCollision(false);
	strawberry_bush_dead.setTransparency(true);
	strawberry_bush_dead.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Dwarf Blueberry Bush (Normal, No Fruit)
	indexer[auto_add] = &dwarf_blueberry_bush;
	dwarf_blueberry_bush = Block(auto_add, "Dwarf Blueberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BUSH_NORMAL, &gamedata::items.dwarf_blueberry_seeds, 1);
	dwarf_blueberry_bush.setBreaingTime(0.65f);
	dwarf_blueberry_bush.setCollision(false);
	dwarf_blueberry_bush.setTransparency(true);
	dwarf_blueberry_bush.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Dwarf Blueberry Bush (Fruits Grown)
	indexer[auto_add] = &dwarf_blueberry_bush_fruit;
	dwarf_blueberry_bush_fruit = Block(auto_add, "Dwarf Blueberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BLUEBERRY_BUSH_FRUIT, &gamedata::items.blueberry, 4);
	dwarf_blueberry_bush_fruit.setBreaingTime(0.65f);
	dwarf_blueberry_bush_fruit.setCollision(false);
	dwarf_blueberry_bush_fruit.setTransparency(true);
	dwarf_blueberry_bush_fruit.setModelType(gamedata::MODEL_PLANT_2FACE);
	dwarf_blueberry_bush_fruit.setHarvestable(true);
	auto_add++;

	// Dwarf Blueberry Bush (Dead / Winter Stats)
	indexer[auto_add] = &dwarf_blueberry_bush_frozen;
	dwarf_blueberry_bush_frozen = Block(auto_add, "Dwarf Blueberry Dead Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BUSH_FROSTY, &gamedata::items.grasses, 3);
	dwarf_blueberry_bush_frozen.setBreaingTime(0.65f);
	dwarf_blueberry_bush_frozen.setCollision(false);
	dwarf_blueberry_bush_frozen.setTransparency(true);
	dwarf_blueberry_bush_frozen.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Bearberry Bush (Normal, No Fruit)
	indexer[auto_add] = &bearberry_bush;
	bearberry_bush = Block(auto_add, "Bearberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BUSH_NORMAL, &gamedata::items.bearberry_seeds, 1);
	bearberry_bush.setBreaingTime(0.65f);
	bearberry_bush.setCollision(false);
	bearberry_bush.setTransparency(true);
	bearberry_bush.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Bearberry Bush (Fruits Grown)
	indexer[auto_add] = &bearberry_bush_fruit;
	bearberry_bush_fruit = Block(auto_add, "Bearberry Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BEARBERRY_BUSH_FRUIT, &gamedata::items.bearberry, 4);
	bearberry_bush_fruit.setBreaingTime(0.65f);
	bearberry_bush_fruit.setCollision(false);
	bearberry_bush_fruit.setTransparency(true);
	bearberry_bush_fruit.setModelType(gamedata::MODEL_PLANT_2FACE);
	bearberry_bush_fruit.setHarvestable(true);
	auto_add++;

	// Bearberry Bush (Dead / Winter Stats)
	indexer[auto_add] = &bearberry_bush_frozen;
	bearberry_bush_frozen = Block(auto_add, "Bearberry Dead Bush", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BUSH_FROSTY, &gamedata::items.grasses, 3);
	bearberry_bush_frozen.setBreaingTime(0.65f);
	bearberry_bush_frozen.setCollision(false);
	bearberry_bush_frozen.setTransparency(true);
	bearberry_bush_frozen.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Lichen (Normal)
	indexer[auto_add] = &lichen;
	lichen = Block(auto_add, "Lichen", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_LICHEN, &gamedata::items.grasses, 1);
	lichen.setBreaingTime(0.65f);
	lichen.setCollision(false);
	lichen.setTransparency(true);
	lichen.setModelType(gamedata::MODEL_SURFACE_ONLY);
	auto_add++;

	// Lichen (Winter Frost)
	indexer[auto_add] = &lichen_frosty;
	lichen_frosty = Block(auto_add, "Frozen Lichen", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_LICHEN_FROSTY, &gamedata::items.grasses, 1);
	lichen_frosty.setBreaingTime(0.95f);
	lichen_frosty.setCollision(false);
	lichen_frosty.setTransparency(true);
	lichen_frosty.setModelType(gamedata::MODEL_SURFACE_ONLY);
	auto_add++;

	// Moss (Normal)
	indexer[auto_add] = &surface_moss;
	surface_moss = Block(auto_add, "Moss", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_SURFACE_MOSS, &gamedata::items.grasses, 1);
	surface_moss.setBreaingTime(0.65f);
	surface_moss.setCollision(false);
	surface_moss.setTransparency(true);
	surface_moss.setModelType(gamedata::MODEL_SURFACE_ONLY);
	auto_add++;

	// Moss (Winter Frost)
	indexer[auto_add] = &surface_moss_frosty;
	surface_moss_frosty = Block(auto_add, "Frozen Moss", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_SURFACE_MOSS_FROSTY, &gamedata::items.grasses, 1);
	surface_moss_frosty.setBreaingTime(0.95f);
	surface_moss_frosty.setCollision(false);
	surface_moss_frosty.setTransparency(true);
	surface_moss_frosty.setModelType(gamedata::MODEL_SURFACE_ONLY);
	auto_add++;

	// SAPLINGS
	
	// Dwarf Brich Sapling
	indexer[auto_add] = &dwarf_brich_sapling;
	dwarf_brich_sapling = Block(auto_add, "Dwarf Birch Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_DWARF_BIRCH_SAPLING, &gamedata::items.dwarf_birch_sapling, 1);
	dwarf_brich_sapling.setBreaingTime(0.65f);
	dwarf_brich_sapling.setCollision(false);
	dwarf_brich_sapling.setTransparency(true);
	dwarf_brich_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Spruce Sapling
	indexer[auto_add] = &spruce_sapling;
	spruce_sapling = Block(auto_add, "Spruce Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_SPRUCE_SAPLING, &gamedata::items.spruce_sapling, 1);
	spruce_sapling.setBreaingTime(0.65f);
	spruce_sapling.setCollision(false);
	spruce_sapling.setTransparency(true);
	spruce_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Pine Sapling
	indexer[auto_add] = &pine_sapling;
	pine_sapling = Block(auto_add, "Pine Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_PINE_SAPLING, &gamedata::items.pine_sapling, 1);
	pine_sapling.setBreaingTime(0.65f);
	pine_sapling.setCollision(false);
	pine_sapling.setTransparency(true);
	pine_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Maple Sapling
	indexer[auto_add] = &maple_sapling;
	maple_sapling = Block(auto_add, "Maple Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_MAPLE_SAPLING, &gamedata::items.maple_sapling, 1);
	maple_sapling.setBreaingTime(0.65f);
	maple_sapling.setCollision(false);
	maple_sapling.setTransparency(true);
	maple_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Brich Sapling
	indexer[auto_add] = &birch_sapling;
	birch_sapling = Block(auto_add, "Birch Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_BIRCH_SAPLING, &gamedata::items.birch_sapling, 1);
	birch_sapling.setBreaingTime(0.65f);
	birch_sapling.setCollision(false);
	birch_sapling.setTransparency(true);
	birch_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Acacia Sapling
	indexer[auto_add] = &acacia_sapling;
	acacia_sapling = Block(auto_add, "Acacia Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_ACACIA_SAPLING, &gamedata::items.acacia_sapling, 1);
	acacia_sapling.setBreaingTime(0.65f);
	acacia_sapling.setCollision(false);
	acacia_sapling.setTransparency(true);
	acacia_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;

	// Jungle Sapling
	indexer[auto_add] = &jungle_sapling;
	jungle_sapling = Block(auto_add, "Jungle Sapling", gamedata::MATERIAL_ORGANIC, gamedata::TEXTURE_JUNGLE_SAPLING, &gamedata::items.jungle_sapling, 1);
	jungle_sapling.setBreaingTime(0.65f);
	jungle_sapling.setCollision(false);
	jungle_sapling.setTransparency(true);
	jungle_sapling.setModelType(gamedata::MODEL_PLANT_2FACE);
	auto_add++;
}

bool BlockData::isTickable(unsigned short int i)
{
	// TODO: add 'tickable' field to 'Block', set default value to false, plus a getter and setter.
	// then: return gamedata::blocks.indexer[i].isTickable();
	return
		(i == gamedata::blocks.dirt.getID()) ||
		(i == gamedata::blocks.grass.getID()) ||
		(i == gamedata::blocks.strawberry_bush.getID()) ||
		(i == gamedata::blocks.strawberry_bush_dead.getID()) ||
		(i == gamedata::blocks.strawberry_bush_fruit.getID()) ||
		(i == gamedata::blocks.dwarf_blueberry_bush.getID()) ||
		(i == gamedata::blocks.dwarf_blueberry_bush_frozen.getID()) ||
		(i == gamedata::blocks.dwarf_blueberry_bush_fruit.getID()) ||
		(i == gamedata::blocks.bearberry_bush.getID()) ||
		(i == gamedata::blocks.bearberry_bush_frozen.getID()) ||
		(i == gamedata::blocks.bearberry_bush_fruit.getID()) ||
		(i == gamedata::blocks.dwarf_birch_leaves.getID()) ||
		(i == gamedata::blocks.dwarf_birch_leaves_frosty.getID()) ||
		(i == gamedata::blocks.spruce_leaves.getID()) ||
		(i == gamedata::blocks.spruce_leaves_frosty.getID()) ||
		(i == gamedata::blocks.pine_leaves.getID()) ||
		(i == gamedata::blocks.pine_leaves_frosty.getID()) ||
		(i == gamedata::blocks.maple_leaves_green_o.getID()) ||
		(i == gamedata::blocks.maple_leaves_green_r.getID()) ||
		(i == gamedata::blocks.maple_leaves_green_y.getID()) ||
		(i == gamedata::blocks.maple_leaves_orange.getID()) ||
		(i == gamedata::blocks.maple_leaves_red.getID()) ||
		(i == gamedata::blocks.maple_leaves_yellow.getID()) ||
		(i == gamedata::blocks.maple_leaves_winter_o.getID()) ||
		(i == gamedata::blocks.maple_leaves_winter_r.getID()) ||
		(i == gamedata::blocks.maple_leaves_winter_y.getID()) ||
		(i == gamedata::blocks.birch_leaves_green_o.getID()) ||
		(i == gamedata::blocks.birch_leaves_green_r.getID()) ||
		(i == gamedata::blocks.birch_leaves_green_y.getID()) ||
		(i == gamedata::blocks.birch_leaves_orange.getID()) ||
		(i == gamedata::blocks.birch_leaves_red.getID()) ||
		(i == gamedata::blocks.birch_leaves_yellow.getID()) ||
		(i == gamedata::blocks.birch_leaves_winter_o.getID()) ||
		(i == gamedata::blocks.birch_leaves_winter_r.getID()) ||
		(i == gamedata::blocks.birch_leaves_winter_y.getID()) ||
		(i == gamedata::blocks.tallgrass.getID()) ||
		(i == gamedata::blocks.tallgrass_dead.getID()) ||
		(i == gamedata::blocks.tallgrass_short.getID()) ||
		(i == gamedata::blocks.tallgrass_short_dead.getID()) ||
		(i == gamedata::blocks.surface_moss.getID()) ||
		(i == gamedata::blocks.surface_moss_frosty.getID()) ||
		(i == gamedata::blocks.lichen.getID()) ||
		(i == gamedata::blocks.lichen_frosty.getID())
		;
}

