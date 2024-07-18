#include "shadow.h"

#include "shader.h"
#include "light_block.h"
#include "system_utils.h"
#include "constants.h"

#include "glad/gl.h"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <exception>
#include <vector>
#include <memory>
#include <utility>

Shadow::Shadow(const std::shared_ptr<LightBlock>& light_block) : light_block(light_block)
{
	if (!shader.compile()) {
		throw std::runtime_error("Failed to construct Shadow class, unable to compile shader");
	}
	if (!light_block) {
		throw std::runtime_error("Failed to construct Shadow class, light_block is null");
	}

	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));

	glGenFramebuffers(1, &depth_map_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Failed to construct Shadew class, Framebuffer is not complete!");
	}

	// TODO do i need this?
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if DEBUG_TEX_RENDER
	if (!setupTest()) {
		throw std::runtime_error("Failed to construct Shadow class, unable to setup test");
	}
#endif
}

Shadow::~Shadow()
{
	glDeleteTextures(1, &depth_map);
	glDeleteFramebuffers(1, &depth_map_fbo);
#if DEBUG_TEX_RENDER
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_vbo);
#endif
}

Shadow::Shadow(Shadow&& other) noexcept :
	shader(std::move(other.shader)),
	view(std::move(other.view)),
	projection(std::move(other.projection)),
	light_block(std::move(other.light_block)),
	depth_map(std::exchange(other.depth_map, 0)),
	depth_map_fbo(std::exchange(other.depth_map_fbo, 0))
#if DEBUG_TEX_RENDER
	,
	quad_vao(std::exchange(other.quad_vao, 0)),
	quad_vbo(std::exchange(other.quad_vbo, 0)),
	test_shader(std::move(other.test_shader))
#endif
{}

GLuint Shadow::getDepthMap()
{
	return depth_map;
}

const glm::mat4& Shadow::getView()
{
	return view;
}

const glm::mat4& Shadow::getProjection()
{
	return projection;
}

void Shadow::renderDepthmap(const glm::vec3& camera_position, const std::vector<Model>& models, const World& world)
{
	glm::vec3 light_position(camera_position +
		(
			glm::vec3(-light_block->read().directional_lights[0].dir) *
			( (shadow_render_distance/2) + shadow_near_plane )
		));

	view = glm::lookAt(light_position, camera_position, WORLD_UP);
	const float half_length = shadow_render_distance/2;
	projection = glm::ortho(-half_length, half_length, -half_length, half_length,
		shadow_near_plane, shadow_render_distance + shadow_near_plane);

	// save old data
	GLint cull_mode;
	glGetIntegerv(GL_CULL_FACE_MODE, &cull_mode);
	GLfloat offset_factor;
	glGetFloatv(GL_POLYGON_OFFSET_FACTOR, & offset_factor);
	GLfloat offset_units;
	glGetFloatv(GL_POLYGON_OFFSET_UNITS, & offset_units);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glCullFace(GL_FRONT);
	glPolygonOffset(1.0f, 1.0f);
	glViewport(0, 0, shadow_width, shadow_height);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	renderScene(view, projection, shader, models, world);

	// restore old data
	glViewport(viewport[0], viewport[1], viewport[2],viewport[3]);
	glPolygonOffset(offset_factor, offset_units);
	glCullFace(cull_mode);

	// TODO do i need this
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#if DEBUG_TEX_RENDER

bool Shadow::setupTest()
{
	if (!test_shader.compile()) {
		LOG("Failed to compile test shader")
		return false;
	}
	test_shader.setInt("quad_tex", 0);

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	return true;
}

void Shadow::renderTest()
{
	test_shader.activate();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(quad_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

#endif