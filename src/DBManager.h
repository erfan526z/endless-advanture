#ifndef DBMAN_H
#define DBMAN_H

#include "sqlite3.h"
#include <vector>
#include <iostream>

bool stringMatches(const char* c1, const char* c2);

char* hashPassword(const char* password);

struct WorldRecord {
	int world_id;
	int owner_id;
	int is_shared;
	char world_seed[64];
	char world_name[64];
	
	void setWorldName(const char* src);

	void setWorldSeed(const char* seed);
};

struct SettingsRecord {
	int render_dist;
	int gui_scale;
};

struct StatisticsRecord {
	int play_time; // Playtime in seconds
	int login_count; // How many logins a player had
	int blocks_placed; // Blocks that player placed
	int blocks_mined; // Blocks that player mined
};

class DataBaseManager
{
public:
	
	DataBaseManager(bool enable_debug_messages = false);
	
	~DataBaseManager();
	
	bool open(const char* filepath);
	
	bool close();
	
	void checkTables();
	
	bool usernameExists(const char* username);
	
	bool addUser(const char* username, const char* hashedPassword);
	
	bool removeUser(const char* username);
	
	char* getPasswordByUsername(const char* username);

	char* getUsernameByID(int user_id);
	
	bool updatePasswordForUser(const char* username, const char* hashed_password);
	
	bool updateTextValueForUser(const char* table, const char* field, const char* username, const char* value);
	
	char* readTextValueForUser(const char* table, const char* field, const char* username);
	
	bool updateIntegerValueForUser(const char* table, const char* field, const char* username, int value);
	
	int readIntegerValueForUser(const char* table, const char* field, const char* username);

	int readIntegerValueFromWorlds(const char* field, const char* username);

	int readSqliteSequence(const char* table);
	
	std::vector<WorldRecord> getWorldRecords(int userId);
	
	std::vector<WorldRecord> getOwnedWorldRecords(int userId);
	
	int addWorldRecord(const char* worldName, int userId, int isShared, const char* seed);
	
	bool removeWorldRecord(int worldId, int userId);
	
	int getUserIdByUsername(const char* username);
	
	void printTable(const char* table);
	
private:
	
	sqlite3 *db;
	
	bool debug_messages;
	
};

#endif

