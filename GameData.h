#pragma once

class Item
{
public:

	Item(unsigned int id, const char* nameptr, unsigned int texture, int stack_size, const unsigned short int *id_of_block);

	Item();

	unsigned int getID() const;

	unsigned int getTexture() const;

	const char* getName() const;

	int getStackSize() const;

	unsigned short int getIdOfBlock() const;

	bool blockIDNotNull() const;

	void setTool(int material);

	void setEatable(int value);

	void setWeapon(int damage);

	int getToolMaterial() const;

	int getEatValue() const;

	int getWeaponDamage() const;

private:

	unsigned int id;
	unsigned int texture;
	const unsigned short int *id_of_block;
	const char* nameptr;
	int stack_size;
	int tool;
	int weapon;
	int eatable;
};

class Block
{
public:

	Block(unsigned short int id, const char* nameptr, unsigned short int material, unsigned int display_texture, const Item* drop, int drop_count = 1, bool is_storage = false);

	Block();

	const char* getNamePtr() const;

	unsigned short int getID() const;

	unsigned short int getMaterial() const;

	unsigned int getDisplayTexture() const;

	bool isStorage() const;

	void setTextureMode(int mode, unsigned int a, unsigned int b = 0, unsigned int c = 0);

	unsigned int getBlockTexture(int direction) const;

	void setBreaingTime(float seconds);

	float getBreakingTime() const;

	const Item* getDropItem() const;

	int getDropCount() const;

	void setModelType(int model_type);

	int getModelType() const;

	void setTransparency(bool has_transparency);

	bool hasTransparency() const;

	void setCollision(bool has_collision);

	bool hasCollision() const;

	void setRenderable(bool is_renderable);

	bool isRenderable() const;

	void setTouchable(bool is_touchable);

	bool isTouchable() const;

	void setHarvestable(bool is_harvestable);

	bool isHarvestable() const;

	const unsigned short int* getBlockID();

private:

	const char* name_ptr;
	unsigned short int id;
	unsigned short int material;
	unsigned int display_texture;
	int texture_mode;
	unsigned int texture_a;
	unsigned int texture_b;
	unsigned int texture_c;
	bool is_storage;
	bool has_transparency;
	bool has_collision;
	bool renderable;
	bool touchable;
	bool harvestable;
	float breaking_time;
	const Item* drop_item;
	int drop_count;
	int model_type;

};

class ItemStack
{
public:
	ItemStack();

	ItemStack(const Item* item, int count);

	void set(const Item* item, int count);

	const Item* getItem() const;

	int getCount() const;

	bool isEmpty() const;

	int addCount(int count = 1, bool clear_if_empty = false);

	void clear();

private:

	const Item* item;
	int count;
};

class ItemInventory
{
public:
	ItemInventory(int inventory_size);

	ItemInventory();

	~ItemInventory();

	bool init(int inventory_size);

	bool reinit(int new_size);

	void clear();

	int getSize();

	int addItem(const Item* item, int count = 1);

	ItemStack& itemStackAt(int i);

	ItemStack* getPointer();

	void swap(int ia, int ib);

private:

	ItemStack* content;
	int size;
	bool initialized;
};

class Recipe {
private:
	ItemStack* input[9];
	ItemStack* output[9];

public:
	Recipe();

	void setInput(int idx, const Item* item, int count);

	void setOutput(int idx, const Item* item, int count);

	bool inputMatches(ItemStack* board);

	bool outputMatches(ItemStack* board);

	void fillOutput(ItemStack* board);

	void subInput(ItemStack* board);

	ItemStack* getOutput(int idx);
};

struct Recipes {
	Recipe* list;
	int len;

	Recipes();
};


struct ItemData {
	ItemData();

	Item stone;
	Item cobblestone;
	Item dirt;
	Item grass;
	Item spruce_log;
	Item pine_log;
	Item dwarf_birch_log;
	Item maple_log;
	Item birch_log;
	Item acacia_log;
	Item jungle_log;
	Item tundra_soil;
	Item taiga_soil;
	Item deciduous_forest_soil;
	Item grassland_soil;
	Item dessert_soil;
	Item savanna_soil;
	Item tropical_soil;
	Item chaparral_soil;
	Item barrel;
	Item spruce_planks;
	Item pine_planks;
	Item birch_planks;
	Item maple_planks;
	Item acacia_planks;
	Item jungle_planks;

	Item stick;
	Item stone_picaxe;
	Item stone_shovel;
	Item stone_sword;
	Item stone_hoe;
	Item stone_axe;
	Item spruce_leaves;
	Item pine_leaves;
	Item dwarf_birch_leaves;
	Item maple_leaves;
	Item birch_leaves;
	Item acacia_leaves;
	Item jungle_leaves;
	Item grasses;
	Item strawberry;
	Item strawberry_seeds;
	Item blueberry;
	Item dwarf_blueberry_seeds;
	Item bearberry;
	Item bearberry_seeds;
	Item dwarf_birch_sapling;
	Item spruce_sapling;
	Item pine_sapling;
	Item maple_sapling;
	Item birch_sapling;
	Item acacia_sapling;
	Item jungle_sapling;

	Item** indexer;
};

struct BlockData {
	BlockData();

	bool isTickable(unsigned short int i);

	Block air;
	Block stone;
	Block cobblestone;
	Block dirt;
	Block grass;
	Block spruce_log;
	Block pine_log;
	Block dwarf_birch_log;
	Block maple_log;
	Block birch_log;
	Block acacia_log;
	Block jungle_log;
	Block spruce_leaves;
	Block spruce_leaves_frosty;
	Block pine_leaves;
	Block pine_leaves_frosty;
	Block dwarf_birch_leaves;
	Block dwarf_birch_leaves_frosty;
	Block maple_leaves_green_y;
	Block maple_leaves_green_o;
	Block maple_leaves_green_r;
	Block maple_leaves_yellow;
	Block maple_leaves_orange;
	Block maple_leaves_red;
	Block maple_leaves_winter_y;
	Block maple_leaves_winter_o;
	Block maple_leaves_winter_r;
	Block birch_leaves_green_y;
	Block birch_leaves_green_o;
	Block birch_leaves_green_r;
	Block birch_leaves_yellow;
	Block birch_leaves_orange;
	Block birch_leaves_red;
	Block birch_leaves_winter_y;
	Block birch_leaves_winter_o;
	Block birch_leaves_winter_r;
	Block acacia_leaves;
	Block jungle_leaves;
	Block tundra_soil;
	Block taiga_soil;
	Block deciduous_forest_soil;
	Block grassland_soil;
	Block dessert_soil;
	Block savanna_soil;
	Block tropical_soil;
	Block chaparral_soil;
	Block barrel;
	Block spruce_planks;
	Block pine_planks;
	Block birch_planks;
	Block maple_planks;
	Block acacia_planks;
	Block jungle_planks;
	Block water;
	Block tallgrass;
	Block tallgrass_dead;
	Block tallgrass_short;
	Block tallgrass_short_dead;
	Block short_grass;
	Block short_grass_dead;
	Block strawberry_bush;
	Block strawberry_bush_fruit;
	Block strawberry_bush_dead;
	Block bearberry_bush;
	Block bearberry_bush_fruit;
	Block bearberry_bush_frozen;
	Block dwarf_blueberry_bush;
	Block dwarf_blueberry_bush_fruit;
	Block dwarf_blueberry_bush_frozen;
	Block surface_moss;
	Block surface_moss_frosty;
	Block surface_frost;
	Block lichen;
	Block lichen_frosty;
	Block dwarf_brich_sapling;
	Block spruce_sapling;
	Block pine_sapling;
	Block maple_sapling;
	Block birch_sapling;
	Block acacia_sapling;
	Block jungle_sapling;

	Block** indexer;
};

namespace gamedata {

	void loadItemInventory(ItemInventory* ic, const char* path);

	void saveItemInventory(ItemInventory* ic, const char* path);

	constexpr int DIRECTION_TOP = 0;
	constexpr int DIRECTION_NEGATIVE_X = 1;
	constexpr int DIRECTION_POSITIVE_X = 2;
	constexpr int DIRECTION_NEGATIVE_Z = 3;
	constexpr int DIRECTION_POSITIVE_Z = 4;
	constexpr int DIRECTION_BOTTOM = 5;

	constexpr int TEXTUREMODE_SINGLE = 0;
	constexpr int TEXTUREMODE_TOP_SIDES = 1;
	constexpr int TEXTUREMODE_TOP_BOTTOM_SIDES = 2;

	constexpr int TEXTURE_NULL = 511;
	constexpr int TEXTURE_STONE = 0;
	constexpr int TEXTURE_COBBLESTONE = 1;
	constexpr int TEXTURE_DIRT = 2;
	constexpr int TEXTURE_GRASS_TOP = 4;
	constexpr int TEXTURE_GRASS_SIDE = 3;

	constexpr int TEXTURE_TUNDRA_SOIL = 32;
	constexpr int TEXTURE_TAIGA_SOIL = 33;
	constexpr int TEXTURE_DECIDUOUS_FOREST_SOIL = 34;
	constexpr int TEXTURE_GRASSLAND_SOIL = 35;
	constexpr int TEXTURE_DESSERT_SOIL = 36;
	constexpr int TEXTURE_SAVANNA_SOIL = 37;
	constexpr int TEXTURE_TROPICAL_SOIL = 38;
	constexpr int TEXTURE_CHAPARRAL_SOIL = 39;

	constexpr int TEXTURE_SPRUCE_LOG = 64;
	constexpr int TEXTURE_SPRUCE_LOG_TOP = 65;
	constexpr int TEXTURE_SPRUCE_LEAF = 66;
	constexpr int TEXTURE_SPRUCE_LEAF_FROZEN = 67;
	constexpr int TEXTURE_PINE_LOG = 68;
	constexpr int TEXTURE_PINE_LOG_TOP = 69;
	constexpr int TEXTURE_PINE_LEAF = 70;
	constexpr int TEXTURE_PINE_LEAF_FROZEN = 71;
	constexpr int TEXTURE_DWARFBIRCH_LOG = 72;
	constexpr int TEXTURE_DWARFBIRCH_LOG_TOP = 73;
	constexpr int TEXTURE_DWARFBIRCH_LEAF = 74;
	constexpr int TEXTURE_DWARFBIRCH_LEAF_FROZEN = 75;
	constexpr int TEXTURE_MAPLE_LOG = 76;
	constexpr int TEXTURE_MAPLE_LOG_TOP = 77;
	constexpr int TEXTURE_MAPLE_LEAF_GREEN = 78;
	constexpr int TEXTURE_MAPLE_LEAF_YELLOW = 79;
	constexpr int TEXTURE_MAPLE_LEAF_ORANGE = 80;
	constexpr int TEXTURE_MAPLE_LEAF_RED = 81;
	constexpr int TEXTURE_MAPLE_LEAF_WINTER = 82;
	constexpr int TEXTURE_BIRCH_LOG = 83;
	constexpr int TEXTURE_BIRCH_LOG_TOP = 84;
	constexpr int TEXTURE_BIRCH_LEAF = 85;
	constexpr int TEXTURE_BIRCH_LEAF_YELLOW = 86;
	constexpr int TEXTURE_BIRCH_LEAF_ORANGE = 87;
	constexpr int TEXTURE_BIRCH_LEAF_RED = 88;
	constexpr int TEXTURE_BIRCH_LEAF_WINTER = 89;
	constexpr int TEXTURE_ACACIA_LOG = 90;
	constexpr int TEXTURE_ACACIA_LOG_TOP = 91;
	constexpr int TEXTURE_ACACIA_LEAF = 92;
	constexpr int TEXTURE_JUNGLE_LOG = 93;
	constexpr int TEXTURE_JUNGLE_LOG_TOP = 94;
	constexpr int TEXTURE_JUNGLE_LEAF = 95;
	
	constexpr int TEXTURE_TALLGRASS_A = 128;
	constexpr int TEXTURE_TALLGRASS_B = 129;
	constexpr int TEXTURE_TALLGRASS_DRY_A = 160;
	constexpr int TEXTURE_TALLGRASS_DRY_B = 161;
	constexpr int TEXTURE_STRAWBERRY_BUSH_NORMAL = 130;
	constexpr int TEXTURE_STRAWBERRY_BUSH_FRUIT = 131;
	constexpr int TEXTURE_STRAWBERRY_BUSH_DRY = 162;
	constexpr int TEXTURE_SURFACE_FROST = 163;
	constexpr int TEXTURE_DWARF_BUSH_NORMAL = 132;
	constexpr int TEXTURE_BEARBERRY_BUSH_FRUIT = 133;
	constexpr int TEXTURE_DWARF_BUSH_FROSTY = 164;
	constexpr int TEXTURE_SHORTGRASS = 165;
	constexpr int TEXTURE_DWARF_BLUEBERRY_BUSH_FRUIT = 134;
	constexpr int TEXTURE_LICHEN = 136;
	constexpr int TEXTURE_SURFACE_MOSS = 137;
	constexpr int TEXTURE_LICHEN_FROSTY = 168;
	constexpr int TEXTURE_SURFACE_MOSS_FROSTY = 169;

	constexpr int TEXTURE_DWARF_BIRCH_SAPLING = 138;
	constexpr int TEXTURE_SPRUCE_SAPLING = 139;
	constexpr int TEXTURE_PINE_SAPLING = 140;
	constexpr int TEXTURE_MAPLE_SAPLING = 141;
	constexpr int TEXTURE_BIRCH_SAPLING = 142;
	constexpr int TEXTURE_ACACIA_SAPLING = 143;
	constexpr int TEXTURE_JUNGLE_SAPLING = 144;

	constexpr int TEXTURE_WATER = 192;

	constexpr int TEXTURE_BARREL_TOP = 257;
	constexpr int TEXTURE_BARREL_SIDE = 256;
	constexpr int TEXTURE_BARREL_BOT = 258;
	constexpr int TEXTURE_SPRUCE_PLANKS = 260;
	constexpr int TEXTURE_PINE_PLANKS = 261;
	constexpr int TEXTURE_BIRCH_PLANKS = 262;
	constexpr int TEXTURE_MAPLE_PLANKS = 263;
	constexpr int TEXTURE_ACACIA_PLANKS = 264;
	constexpr int TEXTURE_JUNGLE_PLANKS = 265;

	constexpr int TEXTURE_STICK = 512;
	constexpr int TEXTURE_STONE_PICKAXE = 513;
	constexpr int TEXTURE_STONE_SHOVEL = 514; // In memory of the 'Wild Lord Of Shovels' (Nothing important, easter egg)
	constexpr int TEXTURE_STONE_SWORD = 515;
	constexpr int TEXTURE_STONE_HOE = 516;
	constexpr int TEXTURE_STONE_AXE = 517;

	constexpr int TEXTURE_STRAWBERRY = 544;
	constexpr int TEXTURE_BEARBERRY = 545;
	constexpr int TEXTURE_BLUEBERRY = 546;
	constexpr int TEXTURE_BLUEBERRY_SEEDS = 547;
	constexpr int TEXTURE_STRAWBERRY_SEEDS = 548;
	constexpr int TEXTURE_BEARBERRY_SEEDS = 549;
	constexpr int TEXTURE_PLANT_SCRAPS = 550;

	constexpr int MATERIAL_ROCK = 1;
	constexpr int MATERIAL_WOOD = 2;
	constexpr int MATERIAL_SOIL = 3;
	constexpr int MATERIAL_ORGANIC = 4;

	constexpr int MODEL_SOLID = 0;
	constexpr int MODEL_SLAB_BOTTOM = 1;
	constexpr int MODEL_SLAB_TOP = 2;
	constexpr int MODEL_PLANT_2FACE = 3;
	constexpr int MODEL_PLANT_4FACE = 4;
	constexpr int MODEL_LIQUID = 5;
	constexpr int MODEL_SURFACE_ONLY = 6;
	constexpr int MODEL_PLANT_SURFACE_2FACE = 7;

	constexpr int INDEXER_LIMIT = 1024;

	constexpr int WATER_LEVEL = 112;

	static ItemData items;

	static BlockData blocks;

	static Recipes recipes;
}