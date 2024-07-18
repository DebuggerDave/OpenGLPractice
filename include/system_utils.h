#ifndef SYSTEM_UTILITY_H
#define SYSTEM_UTILITY_H

#include "screen_manager.h"
#include "camera.h"
#include "world.h"
#include "model.h"
#include "light_block.h"
#include "shader.h"
#include "shadow.h"
#include "game_time.h"

#include "glm/fwd.hpp"
#include "glad/gl.h"

#include <iosfwd>
#include <vector>

struct GameData {
	GameData(
		ScreenManager& screen, std::shared_ptr<Camera>& camera, World& world, std::vector<Model>& models,
		Model& cube, std::shared_ptr<LightBlock> light_block, Shader& light_shader, Shader& skybox_shader,
		Shader& default_shader, GLuint& skybox, Shadow& shadow, GameTime& time
	) noexcept;
	~GameData();
	GameData(const GameData& other) = delete;
	GameData(GameData&& other) noexcept = default;
	GameData& operator=(const GameData& other) = delete;
	GameData& operator=(GameData&& other) = delete;

	ScreenManager screen;
	std::shared_ptr<Camera> camera;
	World world;
	std::vector<Model> models;
	Model cube;
	std::shared_ptr<LightBlock> light_block;
	Shader light_shader;
	Shader skybox_shader;
	Shader default_shader;
	GLuint skybox;
	Shadow shadow;
	GameTime time;
};

// initalize all game data
GameData init();
GLuint loadCubemap(const std::vector<std::string>& faces);
void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const std::vector<Model>& models, const World& world);
// Helper function for drawLight
glm::mat4 lightModelMatrix(const DirectionalLight& light, const glm::vec3& camera_pos);
// Helper function for drawLight
template <typename T>
glm::mat4 lightModelMatrix(const T& light, const glm::vec3& camera_pos);
template <typename T>
void drawLight(const Model& model, const Shader& shader, const std::vector<T>& lights, const glm::vec3& camera_pos);

#endif