#include "game_time.h"

#include "constants.h"

#include "glm/trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp"

GameTime::GameTime(const float current_time) : real_time(current_time), game_time(0.0f) {}

float GameTime::update(const float new_time) {
	float game_delta_time = new_time - real_time;
	real_time = new_time;
	
	while (game_delta_time > 0) {
		const bool is_day = game_time < HALF_DAY;
		const float remaining_game_time = HALF_DAY - std::fmod(game_time, HALF_DAY);
		// convert to game time
		if (is_day) {
			game_delta_time *= game_daytime_conversion_factor;
		} else {
			game_delta_time *= game_nighttime_conversion_factor;
		}
		const float time_step = (game_delta_time <= remaining_game_time) ? game_delta_time : remaining_game_time;
		game_delta_time -= time_step;
		game_time = std::fmod(game_time + time_step, DAY);
		// convert back to real time
		if (is_day) {
			game_delta_time /=  game_daytime_conversion_factor;
		} else {
			game_delta_time /=  game_nighttime_conversion_factor;
		}
	}

	return game_time;
}

float GameTime::getSunXDir() const {
	return -glm::cos(game_time * (2.0f * glm::pi<float>()) / DAY);
}

float GameTime::getSunYDir() const {
	return -glm::sin(game_time * (2.0f * glm::pi<float>()) / DAY);
}
