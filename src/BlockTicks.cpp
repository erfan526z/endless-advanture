#include "BlockTicks.h"

double softrange(double f, double bound) {
	constexpr double twopi = 3.141592653589793116 * 2.0;
	return (-cos(f * (twopi / bound)) + 1.0) / 2.0;
}

float calculateSunlight(double daytime, double delay) {
	float sun_light;
	double dt = daytime + delay;
	if (dt > 1440.0)
		dt -= 1440.0;
	if (dt < 0)
		dt += 1440.0;
	sun_light = softrange(dt, 1440.0);
	sun_light = sun_light * 0.98f + 0.02f;
	return sun_light;
}

float calculateLocalTemperature(double daytime, double randd, int year_day, float base_temp, float base_rain) {
	float daytimeeffect = calculateSunlight(daytime, -110.0);
	float fac1 = (4000.0f - base_rain) * 0.0025;
	year_day += 3;
	if (year_day < 0) year_day += 28;
	float fac2 = (daytime / 1440.0f + (float)year_day);
	float sr = softrange(fac2, 28.0);
	float result = base_temp + (daytimeeffect * 2.0f - 1.0f) * fac1 + (sr - 0.5f) * 22.0f * (0.5f + fac1 / 8.0f);
	return result;
}

double calculateLocalRain(double t, double x, double z, float b, FastNoiseLite& f, bool only_positive) {
	constexpr double m = 2000.0; // General Game Rain Middle Value (min: 0, max: 4000)
	double rain = f.GetNoise(x, z, t);
	double r = cbrt(b / m) - 1.0f;
	rain += r;
	if (only_positive && rain < 0.0)
		return 0.0;
	return rain;
}

float getRainAmount(double start, double end, double x, double z, float base_rain, FastNoiseLite& fnl) {
	constexpr double fac2 = 3.18; // Factor calculated by testing
	constexpr double accuracy = 100.0; // How many times check the rainfall status
	constexpr double threshold_low = 12.0; // 2 in game hours
	constexpr double threshold_high = 4032.0; // 1 in game year

	double sum = 0;
	double step = (end - start) / accuracy;
	if (step < threshold_low) {
		return fnl.GetNoise(x, z, end) * (end - start);
	}
	else if (step > threshold_high) {
		return base_rain;
	}
	else {
		for (double d = start; d < end; d += step) {
			sum += calculateLocalRain(d, x, z, base_rain, fnl, true);
		}
	}

	return (float)((sum / accuracy) * (end - start) / fac2);
}

const char* getDisplayTitleForRain(float val) {
	if (val < -0.5f) return "Sunny";
	if (val < -0.25f) return "Mostly Sunny";
	if (val < 0.0f) return "Cloudy";
	if (val < 0.2f) return "Light Rain";
	if (val < 0.4f) return "Rain";
	if (val < 0.6f) return "Heavy Rain";
	return "Thunder";
}

