#include "system_utils.h"

#include "screen_manager.h"
#include "camera.h"
#include "world.h"
#include "model.h"
#include "light_block.h"
#include "shader.h"
#include "shadow.h"
#include "game_time.h"
#include "constants.h"

#include "glad/gl.h"
#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vector_relational.hpp"
#include "glm/geometric.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "stb_image.h"

#include <string>
#include <vector>
#include <utility>

GameData::GameData(
	ScreenManager& screen, std::shared_ptr<Camera>& camera, World& world, std::vector<Model>& models,
	Model& cube, std::shared_ptr<LightBlock> light_block, Shader& light_shader, Shader& skybox_shader,
	Shader& default_shader, GLuint& skybox, Shadow& shadow, GameTime& time
) noexcept :
	screen{std::move(screen)},
	camera{std::move(camera)},
	world{std::move(world)},
	models{std::move(models)},
	cube{std::move(cube)},
	light_block{std::move(light_block)},
	light_shader{std::move(light_shader)},
	skybox_shader{std::move(skybox_shader)},
	default_shader{std::move(default_shader)},
	skybox{std::move(skybox)},
	shadow{std::move(shadow)},
	time{std::move(time)}
{}

GameData::~GameData()
{
	glDeleteTextures(1, &skybox);
}

GameData init()
{
	std::shared_ptr<Camera> camera = std::make_shared<Camera>(0, World::terrain_median_height + World::terrain_amplitude, 0);
	ScreenManager screen(camera);
	World world;

	// game models
	std::vector<Model> models;
	models.push_back(Model("./assets/other_3d/grass.obj", BlockId::Name::Grass));
	models.push_back(Model("./assets/other_3d/dirt.obj", BlockId::Name::Dirt));
	models.push_back(Model("./assets/other_3d/cobblestone.obj", BlockId::Name::Cobblestone));
	for (const Model& model : models) {
		model.setupInstancing(world);
	}
	// light models
	Model cube("./assets/other_3d/cube.obj");

	// light_block
	std::shared_ptr<LightBlock> light_block = std::make_shared<LightBlock>(1, 1, 1);
	//light_block->updateColor(LightBlock::LightType::Directional, 0, LIGHT_BLACK);
	light_block->updateColor(LightBlock::LightType::Spot, 0, LIGHT_BLACK);
	light_block->updatePosition(LightBlock::LightType::Spot, 0, glm::vec4(camera->getPosition(), 1.0));
	light_block->updateDirection(LightBlock::LightType::Spot, 0, glm::normalize(glm::vec4(camera->getFront(), 0.0)));
	light_block->updateColor(LightBlock::LightType::Point, 0, LIGHT_BLACK);
	light_block->updatePosition(LightBlock::LightType::Point, 0, glm::vec4(camera->getPosition(), 1.0));
	light_block->allocate();

	// shaders
	Shader light_shader("./glsl/light.vert", "./glsl/light.frag");
	light_shader.addLights(Shader::ProgramType::Fragment, light_block);
	if (!light_shader.compile()) {
		throw std::runtime_error("failed to compile shader");
	}
	Shader skybox_shader("./glsl/skybox.vert", "./glsl/skybox.frag");
	if (!skybox_shader.compile()) {
		throw std::runtime_error("failed to compile shader");
	};
	Shader default_shader("./glsl/default.vert", "./glsl/default.frag");
	default_shader.addLights(Shader::ProgramType::Fragment, light_block);
	if (!default_shader.compile()) {
		throw std::runtime_error("failed to compile shader");
	};
	default_shader.activate();
	default_shader.setFloat("material.shininess", std::pow(2, 4));

	// skybox
	unsigned int skybox = loadCubemap(
		std::vector<std::string>({
			"./assets/skybox/right.jpg",
			"./assets/skybox/left.jpg",
			"./assets/skybox/top.jpg",
			"./assets/skybox/bottom.jpg",
			"./assets/skybox/front.jpg",
			"./assets/skybox/back.jpg"
		})
	);
	skybox_shader.activate();
	skybox_shader.setInt("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	Shadow shadow(light_block);
	GameTime time(screen.getTime());

	return GameData{screen, camera, world, models, cube, light_block, light_shader, skybox_shader, default_shader, skybox, shadow, time};
}

GLuint loadCubemap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	int width, height, num_components;
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &num_components, 0);
        if (data)
        {
			GLenum format = GL_RGBA;
			if (num_components == 1)
				format = GL_RED;
			else if (num_components == 3)
				format = GL_RGB;
			else if (num_components == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const std::vector<Model>& models, const World& world)
{
	shader.activate();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));

	for (const auto& model : models) {
		model.draw(shader, world.numObjects(model.id));
	}
}

glm::mat4 lightModelMatrix(const DirectionalLight& light, const glm::vec3& camera_pos) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, camera_pos + (glm::vec3(-light.dir) * FAR_PLANE));
	model = glm::scale(model, glm::vec3(10.0f));
	return model;
}

template <typename T>
glm::mat4 lightModelMatrix(const T& light, [[maybe_unused]] const glm::vec3& camera_pos) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(light.pos));
	return model;
}
template
glm::mat4 lightModelMatrix(const SpotLight& light, const glm::vec3& camera_pos);
template
glm::mat4 lightModelMatrix(const PointLight& light, const glm::vec3& camera_pos);

template <typename T>
void drawLight(const Model& model, const Shader& shader, const std::vector<T>& lights, const glm::vec3& camera_pos)
{
	using LightSizeType = std::remove_cvref_t<decltype(lights)>::size_type;
	for (LightSizeType i=0; i < lights.size(); i++) {
		constexpr glm::vec4 zero(0.0f);
		const T cur_light = lights[i];
		if ((glm::all(glm::equal(cur_light.color.ambient, zero))) &&
			(glm::all(glm::equal(cur_light.color.diffuse, zero))) &&
			(glm::all(glm::equal(cur_light.color.specular, zero)))) {
			return;
		}
		shader.setVec4("light_color", cur_light.color.diffuse);
		const glm::mat4 model_mat = lightModelMatrix(cur_light, camera_pos);
			
		shader.setMat4("model", std::move(model_mat));
		model.draw(shader);
	}
}
template
void drawLight(const Model& model, const Shader& shader, const std::vector<DirectionalLight>& lights, const glm::vec3& camera_pos);
template
void drawLight(const Model& model, const Shader& shader, const std::vector<SpotLight>& lights, const glm::vec3& camera_pos);
template
void drawLight(const Model& model, const Shader& shader, const std::vector<PointLight>& lights, const glm::vec3& camera_pos);