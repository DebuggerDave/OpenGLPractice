#include "system_utils.h"
#include "constants.h"

#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/vec4.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

int main()
{
	GameData game_data = init();

	while (!game_data.screen.shouldClose())
	{
		// time update
		static float last_frame_time = 0.0f;
		const float frame_time = game_data.screen.getTime();
		const float delta_time = frame_time - last_frame_time;
		last_frame_time = frame_time;
		game_data.time.update(frame_time);

		// input update
		game_data.screen.processInput(delta_time);

		// light update
		game_data.light_block->updateDirection(LightBlock::LightType::Directional, 0, glm::normalize(glm::vec4(game_data.time.getSunXDir(), game_data.time.getSunYDir(), 0.0f, 0.0f)));
		game_data.light_block->updatePosition(LightBlock::LightType::Spot, 0, glm::vec4(game_data.camera->getPosition(), 1.0f));
		game_data.light_block->updateDirection(LightBlock::LightType::Spot, 0, glm::normalize(glm::vec4(game_data.camera->getFront(), 0.0f)));

		// transform update
		glm::mat4 projection = glm::perspective(glm::radians(game_data.camera->getZoom()), static_cast<float>(SCR_WIDTH) / SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
		glm::mat4 view = game_data.camera->getViewMatrix();
		game_data.world.updateNormalMats(view);

		// shadow render
		game_data.shadow.renderDepthmap(game_data.camera->getPosition(), game_data.models, game_data.world);

		// world render
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + 16);
		glBindTexture(GL_TEXTURE_2D, game_data.shadow.getDepthMap());
		game_data.default_shader.activate();
		game_data.default_shader.setInt("depth_map", 16);
		game_data.default_shader.setMat4("light_view", game_data.shadow.getView());
		game_data.default_shader.setMat4("light_projection", game_data.shadow.getProjection());
		renderScene(view, projection, game_data.default_shader, game_data.models, game_data.world);

		// light render
		game_data.light_shader.activate();
		game_data.light_shader.setMat4("view", view);
		game_data.light_shader.setMat4("projection", projection);
		drawLight(game_data.cube, game_data.light_shader, game_data.light_block->read().directional_lights, game_data.camera->getPosition());
		drawLight(game_data.cube, game_data.light_shader, game_data.light_block->read().spot_lights, game_data.camera->getPosition());
		drawLight(game_data.cube, game_data.light_shader, game_data.light_block->read().point_lights, game_data.camera->getPosition());

		// skybox render
		GLint cull_mode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cull_mode);
		glCullFace(GL_FRONT);
		game_data.skybox_shader.activate();
		game_data.skybox_shader.setMat4("view", glm::mat4(glm::mat3(view)));
		game_data.skybox_shader.setMat4("projection", projection);
		game_data.cube.draw(game_data.skybox_shader);
		glCullFace(cull_mode);

		game_data.screen.endFrame();
	}

	return 0;
}