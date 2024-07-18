#ifndef GAME_TIME_H
#define GAME_TIME_H

class GameTime
{
public:
	// construct with current real time in seconds
	GameTime(const float current_time) noexcept;

	// pass updated real time in seconds to get current game time
	float update(const float new_time);
	float getSunXDir() const;
	float getSunYDir() const;

	static constexpr int minute = 60;
	static constexpr int hour = minute * 60;
	static constexpr int day = hour * 24;
	static constexpr int half_day = day / 2.0f;
	inline static const float game_daytime = minute;
	inline static const float game_nighttime = minute * 0.0001f;
private:
	float real_time;
	float game_time;
	inline static const float game_daytime_conversion_factor = half_day / game_daytime;
	inline static const float game_nighttime_conversion_factor = half_day / game_nighttime;
};

#endif