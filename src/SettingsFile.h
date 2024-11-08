#include <cstring>

class SettingsFile {
public:
	
	SettingsFile(const char* path) {
		this->path = path;
	}
	
	bool setIntegerValue(const char* entry, int value) {
		return true;
	}
	
	bool setFloatValue(const char* entry, float value) {
		return true;
	}
	
	bool setStringValue(const char* entry, const char* start, int len) {
		return true;
	}
	
	bool getIntegerValue(const char* entry, int& value) {
		if (!strcmp(entry, "render_distance")) {
			value = 8;
			return true;
		}
		if (!strcmp(entry, "max_memory_chunks")) {
			value = 600;
			return true;
		}
		return false;
	}
	
	bool getFloatValue(const char* entry, float& value) {
		return true;
	}
	
	bool getStringValue(const char* entry, const char* start, int max_len) {
		return true;
	}
	
private:
	
	const char* path;
	
};