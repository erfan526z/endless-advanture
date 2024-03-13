#pragma once

#include "FastNoiseLite.h"

double softrange(double f, double bound);

float calculateSunlight(double daytime, double delay = 0.0);

float calculateLocalTemperature(double daytime, double randd, int year_day, float base_temp, float base_rain);

double calculateLocalRain(double t, double x, double z, float b, FastNoiseLite& f, bool only_positive = false);

float getRainAmount(double start, double end, double x, double z, float base_rain, FastNoiseLite& fnl);

const char* getDisplayTitleForRain(float val);

struct ClimateTimeInfo
{
	int year;
	int day;
	float time;
	float temperature;
	float rainfall;
	float delta_time;
};

// World Timestamp
struct ChunkTimeStamp 
{
	int year; // 0 ~ ... (Max Int)
	int day; // 0 ~ 27
	float time; // 0.0 ~ 1440.0

	double distance_since(ChunkTimeStamp& b) {
		double d = (double) (year - b.year) * 40320.0;
		d += (double) (day - b.day) * 1440.0;
		d += (double) (time - b.time);
		return d;
	}

	double to_double() {
		return (double)(year) * 40320.0 + (double)(day) * 1440.0 + (double)time;
	}
};

// Tickable Block
struct TickableBlock 
{
	unsigned short int block_id;
	unsigned short int y;
	char x, z;
	char stat1, stat2;
	float stat3, stat4, stat5;
	ChunkTimeStamp last_update;
};
