#ifndef SHADOW_H
#define SHADOW_H

#include "shader.h"
#include "light_block.h"
class Model;
class World;

#include "glad/gl.h"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <vector>
#include <memory>

#define DEBUG_TEX_RENDER false

class Shadow
{
public:
	Shadow(const std::shared_ptr<LightBlock>& light_block);
	~Shadow();
	Shadow(const Shadow& other) = delete;
	Shadow(Shadow&& other) noexcept;
	Shadow& operator=(const Shadow& other) = delete;
	Shadow& operator=(Shadow&& other) = delete;

	GLuint getDepthMap();
	const glm::mat4& getView();
	const glm::mat4& getProjection();
	// render to depth_map handle
	void renderDepthmap(const glm::vec3& camera_position, const std::vector<Model>& models, const World& world);

	inline static constexpr glm::vec4 border_color{1.0f, 1.0f, 1.0f, 1.0f};
	static constexpr float shadow_near_plane = 5.0f;
	static constexpr float shadow_render_distance = 100.0f;
	static constexpr unsigned int shadow_resolution = 1024;
	static constexpr unsigned int shadow_width = shadow_resolution;
	static constexpr unsigned int shadow_height = shadow_resolution;
private:
	Shader shader{"./glsl/shadow.vert", "./glsl/shadow.frag"};
	glm::mat4 view;
	glm::mat4 projection;
	std::shared_ptr<LightBlock> light_block;
	GLuint depth_map;
	GLuint depth_map_fbo;

#if DEBUG_TEX_RENDER
	bool setupTest();
	void renderTest();
    GLuint quad_vao;
	GLuint quad_vbo;
	Shader test_shader{"./glsl/quad.vert", "./glsl/quad.frag"};
	// simple quad for testing shadows
	inline static const float quad[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
#endif
};

#endif