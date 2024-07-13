#ifndef GAME_TIME_H
#define GAME_TIME_H

#include "constants.h"

class GameTime
{
public:
	// construct with current real time in seconds
	GameTime(const float current_time);

	// pass updated real time in seconds to get current game time
	float update(const float new_time);
	float getSunXDir() const;
	float getSunYDir() const;

private:
	float real_time;
	float game_time;
	inline static const float game_daytime = MINUTE;
	inline static const float game_nighttime = MINUTE * 0.0001f;
	inline static const float game_daytime_conversion_factor = HALF_DAY / game_daytime;
	inline static const float game_nighttime_conversion_factor = HALF_DAY / game_nighttime;
};

#endif