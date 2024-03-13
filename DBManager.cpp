#include "DBManager.h"
#include <cstring>
#include <string>

void WorldRecord::setWorldName(const char* src) {
	int len = strlen(src);
	if (len > 63) len = 63;
	len++;
	memcpy(world_name, src, len);
	world_name[63] = 0;
}

void WorldRecord::setWorldSeed(const char* seed)
{
	int len = strlen(seed);
	if (len > 63) len = 63;
	len++;
	memcpy(world_seed, seed, len);
	world_seed[63] = 0;
}

DataBaseManager::DataBaseManager(bool enable_debug_messages) {
	this->debug_messages = enable_debug_messages;
	this->db = nullptr;
}

DataBaseManager::~DataBaseManager() {
	// Nothing special for now
}

bool DataBaseManager::open(const char* filepath) {
	char *errMsg = nullptr;
	
	int rc = sqlite3_open(filepath, &db);
	
	if (rc) {
		if (debug_messages) fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	} else {
		if (debug_messages) fprintf(stdout, "Opened database successfully\n");
		return true;
	}
}

bool DataBaseManager::close() {
	if (!db) return false;
	sqlite3_close(db);
	db = nullptr;
	return true;
}

void DataBaseManager::checkTables() {
	char *errMsg = 0;	
	
	const char *createUsersTableSQL = "CREATE TABLE IF NOT EXISTS Users ("
									  "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
									  "username TEXT NOT NULL,"
									  "password TEXT NOT NULL"
									  ");";
									  
	const char *createSettingsTableSQL = "CREATE TABLE IF NOT EXISTS Settings ("
										 "setting_id INTEGER PRIMARY KEY AUTOINCREMENT,"
										 "user_id INTEGER,"
										 "render_dist INTEGER,"
										 "gui_scale INTEGER,"
										 "FOREIGN KEY(user_id) REFERENCES Users(user_id)"
										 // Add other setting fields here
										 ");";

	const char *createStatisticsTableSQL = "CREATE TABLE IF NOT EXISTS Statistics ("
										   "statistic_id INTEGER PRIMARY KEY AUTOINCREMENT,"
										   "user_id INTEGER,"
										   "play_time INTEGER,"
										   "login_count INTEGER,"
										   "blocks_mined INTEGER,"
										   "blocks_placed INTEGER,"
										   "FOREIGN KEY(user_id) REFERENCES Users(user_id)"
										   // Add other statistic fields here
										   ");";
	
	const char *createWorldsTableSQL = "CREATE TABLE IF NOT EXISTS Worlds ("
									   "world_id INTEGER PRIMARY KEY AUTOINCREMENT,"
									   "world_name TEXT,"
									   "owner_id INTEGER,"
									   "is_shared INTEGER,"
									   "seed TEXT,"
									   "FOREIGN KEY(owner_id) REFERENCES Users(user_id)"
									   ");";
	
	int rc = sqlite3_exec(db, createUsersTableSQL, 0, 0, &errMsg);

	if (rc != SQLITE_OK) {
		if (debug_messages) fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
	} else {
		if (debug_messages) fprintf(stdout, "Users table created successfully\n");
	}
	
	rc = sqlite3_exec(db, createSettingsTableSQL, 0, 0, &errMsg);

	if (rc != SQLITE_OK) {
		if (debug_messages) fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
	} else {
		if (debug_messages) fprintf(stdout, "Settings table created successfully\n");
	}
	
	rc = sqlite3_exec(db, createStatisticsTableSQL, 0, 0, &errMsg);

	if (rc != SQLITE_OK) {
		if (debug_messages) fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
	} else {
		if (debug_messages) fprintf(stdout, "Statistics table created successfully\n");
	}
	
	rc = sqlite3_exec(db, createWorldsTableSQL, 0, 0, &errMsg);

	if (rc != SQLITE_OK) {
		if (debug_messages) fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
	} else {
		if (debug_messages) fprintf(stdout, "Worlds table created successfully\n");
	}
}

bool DataBaseManager::usernameExists(const char* username) {
	char *errMsg = 0;

	std::string query = "SELECT COUNT(*) FROM Users WHERE username = '" + std::string(username) + "';";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		if (debug_messages) fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
		return false;
	}

	int result = 0;

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	if (result > 0) {
		if (debug_messages) std::cout << "Username exists in the Users table\n";
		return true;
	} else {
		if (debug_messages) std::cout << "Username does not exist in the Users table\n";
		return false;
	}
}

bool DataBaseManager::addUser(const char* username, const char* hashedPassword) {
	// Insert user record into Users table
	std::string addUserQuery = "INSERT INTO Users (username, password) VALUES ('" + std::string(username) + "', '" + std::string(hashedPassword) + "');";
	int rc = sqlite3_exec(db, addUserQuery.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "Error adding user: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	// Create Settings record for the user
	std::string createSettingsQuery = "INSERT INTO Settings (user_id) SELECT user_id FROM Users WHERE username = '" + std::string(username) + "';";
	rc = sqlite3_exec(db, createSettingsQuery.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "Error creating settings record: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	// Create Statistics record for the user (similar logic as Settings)
	std::string createStatsQuery = "INSERT INTO Statistics (user_id) SELECT user_id FROM Users WHERE username = '" + std::string(username) + "';";
	rc = sqlite3_exec(db, createStatsQuery.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "Error creating statistics record: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	if (debug_messages) std::cout << "User added successfully!" << std::endl;
	return true;
}

bool DataBaseManager::removeUser(const char* username) {
	// Check if the user owns any worlds
	std::string checkWorldsQuery = "SELECT COUNT(*) FROM Worlds WHERE owner_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
	int result = 0;
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, checkWorldsQuery.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	if (result == 0) {
		// No worlds owned by the user, delete Settings record
		std::string deleteSettingsQuery = "DELETE FROM Settings WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
		rc = sqlite3_exec(db, deleteSettingsQuery.c_str(), nullptr, nullptr, nullptr);
		if (rc != SQLITE_OK) {
			if (debug_messages) std::cerr << "Error deleting settings record: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}

		// Delete Statistics record (similar logic as Settings)
		std::string deleteStatsQuery = "DELETE FROM Statistics WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
		rc = sqlite3_exec(db, deleteStatsQuery.c_str(), nullptr, nullptr, nullptr);
		if (rc != SQLITE_OK) {
			if (debug_messages) std::cerr << "Error deleting statistics record: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}

		// Finally, delete the user record from Users table
		std::string deleteUserQuery = "DELETE FROM Users WHERE username = '" + std::string(username) + "';";
		rc = sqlite3_exec(db, deleteUserQuery.c_str(), nullptr, nullptr, nullptr);
		if (rc != SQLITE_OK) {
			if (debug_messages) std::cerr << "Error deleting user: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}

		if (debug_messages) std::cout << "User removed successfully!" << std::endl;
	} else {
		if (debug_messages) std::cout << "User owns worlds, cannot be removed." << std::endl;
	}
	return true;
}

char* DataBaseManager::getPasswordByUsername(const char* username) {
	char* password = nullptr;
	std::string query = "SELECT password FROM Users WHERE username = '" + std::string(username) + "';";
	
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return nullptr;
	}
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char *result = (sqlite3_column_text(stmt, 0));
		if (result) {
			const char* cs = reinterpret_cast<const char*>(result);
			password = new char[strlen(cs) + 1];
			memcpy(password, cs, strlen(cs) + 1);;
		}
	}

	sqlite3_finalize(stmt);
	return password;
}

char* DataBaseManager::getUsernameByID(int user_id)
{
	char* username = nullptr;
	std::string query = "SELECT username FROM Users WHERE user_id = '" + std::to_string(user_id) + "';";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return nullptr;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* result = (sqlite3_column_text(stmt, 0));
		if (result) {
			const char* cs = reinterpret_cast<const char*>(result);
			username = new char[strlen(cs) + 1];
			memcpy(username, cs, strlen(cs) + 1);;
		}
	}

	sqlite3_finalize(stmt);
	return username;
}

bool DataBaseManager::updatePasswordForUser(const char* username, const char* hashed_password) {
	std::string query = "UPDATE Users SET password = '" + std::string(hashed_password) + "' WHERE username = '" + std::string(username) + "';";
	
	int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	return true;
}

bool DataBaseManager::updateTextValueForUser(const char* table, const char* field, const char* username, const char* value) {
	std::string query = "UPDATE " + std::string(table) + " SET " + std::string(field) + " = '" + std::string(value)
			 + "' WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
	
	int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	return true;
}

char* DataBaseManager::readTextValueForUser(const char* table, const char* field, const char* username) {
	char* value = nullptr;
	std::string query = "SELECT " + std::string(field) + " FROM " + std::string(table) + " WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
	
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return nullptr;
	}
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char *result = sqlite3_column_text(stmt, 0);
		if (result) {
			const char* cs = reinterpret_cast<const char*>(result);
			value = new char[strlen(cs) + 1];
			memcpy(value, cs, strlen(cs) + 1);;
		}
	}

	sqlite3_finalize(stmt);
	return value;
}

bool DataBaseManager::updateIntegerValueForUser(const char* table, const char* field, const char* username, int value) {
	std::string query = "UPDATE " + std::string(table) + " SET " + std::string(field) + " = " + std::to_string(value)
			 + " WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
	
	int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return false; // Return false on error
	}

	return true;
}

int DataBaseManager::readIntegerValueForUser(const char* table, const char* field, const char* username) {
	int value = 0;
	std::string query = "SELECT " + std::string(field) + " FROM " + std::string(table) + " WHERE user_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";
	
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return 0; // Return default value (0) on error
	}
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		value = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return value;
}

int DataBaseManager::readIntegerValueFromWorlds(const char* field, const char* username)
{
	int value = 0;
	std::string query = "SELECT " + std::string(field) + " FROM Worlds WHERE owner_id = (SELECT user_id FROM Users WHERE username = '" + std::string(username) + "');";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return 0; // Return default value (0) on error
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		value = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return value;
}

int DataBaseManager::readSqliteSequence(const char* table)
{
	int value = 0;
	std::string query = "SELECT seq FROM sqlite_sequence WHERE name = '" + std::string(table) + "';";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		if (debug_messages) std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return 0; // Return default value (0) on error
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		value = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return value;
}

int DataBaseManager::addWorldRecord(const char* worldName, int userId, int isShared, const char* seed) {
    std::string query = "INSERT INTO Worlds (world_name, owner_id, is_shared, seed) VALUES ('" + std::string(worldName) + "', " + std::to_string(userId) + ", " + std::to_string(isShared) + ", '" + std::string(seed) + "');";
    
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false; // Return false on error
    }

	int world_id = readSqliteSequence("Worlds");

    return world_id;
}

bool DataBaseManager::removeWorldRecord(int worldId, int userId) {
    std::string query = "DELETE FROM Worlds WHERE world_id = " + std::to_string(worldId) + " AND owner_id = " + std::to_string(userId) + ";";
    
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false; // Return false on error
    }

    return true;
}

std::vector<WorldRecord> DataBaseManager::getWorldRecords(int userId) {
    std::vector<WorldRecord> worldRecords;

    std::string query = "SELECT world_id, world_name, owner_id, is_shared, seed FROM Worlds WHERE owner_id = " + std::to_string(userId) + " OR (is_shared = 1 AND owner_id != " + std::to_string(userId) + ");";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return worldRecords; // Return empty vector on error
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WorldRecord record;
        record.world_id = sqlite3_column_int(stmt, 0);
        const unsigned char* worldName = sqlite3_column_text(stmt, 1);
		const char* cwn = reinterpret_cast<const char*>(worldName);
        if (cwn) {
            record.setWorldName(reinterpret_cast<const char*>(cwn));
        }
        record.owner_id = sqlite3_column_int(stmt, 2);
        record.is_shared = sqlite3_column_int(stmt, 3);
		const unsigned char* worldSeed = sqlite3_column_text(stmt, 4);
		cwn = reinterpret_cast<const char*>(worldSeed);
		if (cwn) {
			record.setWorldSeed(reinterpret_cast<const char*>(cwn));
		}
        worldRecords.push_back(record);
    }

    sqlite3_finalize(stmt);
    return worldRecords;
}

std::vector<WorldRecord> DataBaseManager::getOwnedWorldRecords(int userId) {
    std::vector<WorldRecord> worldRecords;

    std::string query = "SELECT world_id, world_name, owner_id, is_shared FROM Worlds WHERE owner_id = " + std::to_string(userId) + ";";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return worldRecords; // Return empty vector on error
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WorldRecord record;
        record.world_id = sqlite3_column_int(stmt, 0);
        const unsigned char* worldName = sqlite3_column_text(stmt, 1);
        if (worldName) {
            record.setWorldName(reinterpret_cast<const char*>(worldName));
        }
        record.owner_id = sqlite3_column_int(stmt, 2);
        record.is_shared = sqlite3_column_int(stmt, 3);
		const unsigned char* worldSeed = sqlite3_column_text(stmt, 4);
		const char* cwn = reinterpret_cast<const char*>(worldSeed);
		if (cwn) {
			record.setWorldSeed(reinterpret_cast<const char*>(cwn));
		}
        worldRecords.push_back(record);
    }

    sqlite3_finalize(stmt);
    return worldRecords;
}

int DataBaseManager::getUserIdByUsername(const char* username) {
    int user_id = -1; // Default value or error indicator

    std::string query = "SELECT user_id FROM Users WHERE username = '" + std::string(username) + "';";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return user_id; // Return default or error indicator
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return user_id;
}

void DataBaseManager::printTable(const char* table) {
	std::string query = "SELECT * FROM " + std::string(table) + ";";
	
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	
	// Get number of columns in the result set
	int columns = sqlite3_column_count(stmt);

	// Print column names
	for (int i = 0; i < columns; ++i) {
		std::cout << sqlite3_column_name(stmt, i) << "\t";
	}
	std::cout << std::endl;

	// Print rows
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		for (int i = 0; i < columns; ++i) {
			const unsigned char* value = sqlite3_column_text(stmt, i);
			if (value) {
				std::cout << value << "\t";
			} else {
				std::cout << "NULL\t";
			}
		}
		std::cout << std::endl;
	}

	sqlite3_finalize(stmt);
}

bool stringMatches(const char* c1, const char* c2)
{
	int l = strlen(c1);
	if (strlen(c2) != l) return false;
	for (int i = 0; i < l; i++)
		if (c1[i] != c2[i]) return false;
	return true;
}

char* hashPassword(const char* password)
{
	if (!password) return nullptr;

	char* buffer = new char[129];
	unsigned int buffer16x4[16];

	for (int i = 0; i < 16; i++)
		buffer16x4[i] = 0;

	for (int i = 0; i < strlen(password); i++) {
		char r = password[i];
		// A number in 0b000000 to 0b111111 range
		char seed_char = (r >= 'A' && 'r' <= 'Z') ? r - 'A' : (r >= 'a' && r <= 'z') ? r - 'a' + 26 : (r >= '0' && r <= '9') ? r - '0' + 52 : (r == ' ') ? 62 : 63;
		buffer16x4[0] = (buffer16x4[0] >> 2) + buffer16x4[0] * 41 + seed_char * 2467 + 6337;
		buffer16x4[1] = (buffer16x4[1] >> 2) + buffer16x4[1] * 3727 + seed_char * 3481 + 29363;
		buffer16x4[2] = buffer16x4[2] * 1709 + seed_char * 149 + 23291;
		buffer16x4[3] = (buffer16x4[3] << 1) + buffer16x4[3] * 491 + seed_char * 2999 + 11953;
		buffer16x4[4] = buffer16x4[4] * 397 + seed_char * 2609 + 3907;
		buffer16x4[5] = (buffer16x4[5] >> 3) + buffer16x4[5] * 383 + seed_char * 1423 + 18719;
		buffer16x4[6] = (buffer16x4[6] << 1) + buffer16x4[6] * 1447 + seed_char * 1733 + 14771;
		buffer16x4[7] = (buffer16x4[7] << 2) + buffer16x4[7] * 3917 + seed_char * 1667 + 26309;
		buffer16x4[8] = (buffer16x4[8] << 1) + buffer16x4[8] * 709 + seed_char * 3821 + 31327;
		buffer16x4[9] = buffer16x4[9] * 673 + seed_char * 3163 + 7717;
		buffer16x4[10] = (buffer16x4[10] << 1) + buffer16x4[10] * 1549 + seed_char * 3659 + 32687;
		buffer16x4[11] = buffer16x4[11] * 859 + seed_char * 727 + 9743;
		buffer16x4[12] = (buffer16x4[12] << 1) + buffer16x4[12] * 317 + seed_char * 3037 + 22193;
		buffer16x4[13] = buffer16x4[13] * 2111 + seed_char * 1049 + 8951;
		buffer16x4[14] = buffer16x4[14] * 3449 + seed_char * 3821 + 15901;
		buffer16x4[15] = (buffer16x4[15] >> 1) + buffer16x4[15] * 3359 + seed_char * 3011 + 31121;
	}

	for (int i = 0; i < 16; i++) {
		int i8 = i * 8;
		buffer[i8] = (char)((buffer16x4[i]) % 0x10);
		buffer[i8 + 1] = (char)((buffer16x4[i] >> 4) % 0x10);
		buffer[i8 + 2] = (char)((buffer16x4[i] >> 8) % 0x10);
		buffer[i8 + 3] = (char)((buffer16x4[i] >> 12) % 0x10);
		buffer[i8 + 4] = (char)((buffer16x4[i] >> 16) % 0x10);
		buffer[i8 + 5] = (char)((buffer16x4[i] >> 20) % 0x10);
		buffer[i8 + 6] = (char)((buffer16x4[i] >> 24) % 0x10);
		buffer[i8 + 7] = (char)((buffer16x4[i] >> 28) % 0x10);
	}

	for (int i = 0; i < 128; i++) {
		buffer[i] = (buffer[i] < 10 ) ? buffer[i] + '0' : buffer[i] - 10 + 'A';
	}

	buffer[128] = 0;

	return buffer;
}
