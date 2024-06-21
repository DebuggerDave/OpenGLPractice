#include "pch.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "utils.h"
#include "light_block.h"
#include "light_uniform_buffer.h"
#include "world.h"
#include "component.h"
#include "constants.h"
#include "system_utils.h"

#include <vector>
#include <memory>

int main()
{
	Camera camera(0, World::terrain_median_height + World::terrain_aplitude, 0);

	GLFWwindow* window = init(camera);
	if (!window) return -1;

	World world;

	static Model grass("./assets/other_3d/grass.obj");
	static Model dirt("./assets/other_3d/dirt.obj");
	static Model cobblestone("./assets/other_3d/cobblestone.obj");
	static Model cube("./assets/other_3d/cube.obj");

	std::shared_ptr<LightBlock> light_block = LightBlock::makeShared(1, 1, 1);
	LightColor black{glm::vec4{0.0f}, glm::vec4(0.0f), glm::vec4(0.0f)};
	light_block->updateColor(LightBlock::LightType::Spot, 0, black);
	light_block->updateColor(LightBlock::LightType::Point, 0, black);
	light_block->allocate();

	Shader light_shader("./glsl/light.vert", "./glsl/light.frag");
	light_shader.addLights(Shader::ProgramType::Fragment, light_block);
	light_shader.compile();
	Shader skybox_shader("./glsl/skybox.vert", "./glsl/skybox.frag");
	skybox_shader.compile();
	Shader default_shader("./glsl/default.vert", "./glsl/default.frag");
	default_shader.addLights(Shader::ProgramType::Fragment, light_block);
	default_shader.compile();
	//Shader normal_shader("./glsl/normal.vert", "./glsl/normal.frag", "./glsl/normal.geom");
	//normal_shader.compile();
	Shader shadow_shader("./glsl/shadow.vert", "./glsl/shadow.frag");
	shadow_shader.compile();

	default_shader.activate();
	default_shader.setFloat("material.shininess", std::pow(2, 4));

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

	// shadow frame buffer
	// ----
	unsigned int depth_map_fbo;
	glGenFramebuffers(1, &depth_map_fbo);

	unsigned int depth_map;
	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_SHORT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glm::vec4 border_color(1.0f, 1.0f, 1.0f, 1.0f);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));

	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float delta_time = 0.0f;
	float last_frame_time = 0.0f;
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		showImgui(*light_block);

		// per-frame time logic
		float current_frame_time = glfwGetTime();
		delta_time = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;
		// input
		processInput(window, delta_time, camera);

		// start rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
		glm::mat4 view = camera.getViewMatrix();

		// shadows
		// ----
		glCullFace(GL_FRONT);
		glPolygonOffset(1.0f, 1.0f);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		float shadow_render_distance = 100;
		glm::vec3 world_up(0.0, 1.0, 0.0);
		glm::vec3 light_position((glm::vec3(-light_block->read().directional_lights[0].dir) * ((shadow_render_distance/2) + SHADOW_NEAR_PLANE)) + camera.getPosition());
		glm::vec3 light_front(light_block->read().directional_lights[0].dir);
		glm::vec3 light_right(glm::normalize(glm::cross(light_front, world_up)));
		glm::vec3 light_up(glm::normalize(glm::cross(light_right, light_front)));
		glm::mat4 light_view = glm::lookAt(light_position, light_position + light_front, light_up);
		glm::mat4 light_projection = glm::ortho(-shadow_render_distance/2, shadow_render_distance/2, -shadow_render_distance/2, shadow_render_distance/2, SHADOW_NEAR_PLANE, shadow_render_distance + SHADOW_NEAR_PLANE);
		renderScene(light_view, light_projection, shadow_shader, grass, dirt, world);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glPolygonOffset(0.0f, 0.0f);
		glCullFace(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		default_shader.activate();
		default_shader.setInt("depth_map", 16);
		glActiveTexture(GL_TEXTURE0 + 16);
		glBindTexture(GL_TEXTURE_2D, depth_map);
		default_shader.setMat4("light_view", light_view);
		default_shader.setMat4("light_projection", light_projection);
		renderScene(view, projection, default_shader, grass, dirt, world);

		// lights
		// ----
		light_shader.activate();
		light_shader.setMat4("view", view);
		light_shader.setMat4("projection", projection);

		// TODO this stuff is redundant, fix it
		// point lights
		const LightBlockData& light_data = light_block->read();
		using PointLightSizeType = std::remove_cvref_t<decltype(light_data.point_lights)>::size_type;
		for (PointLightSizeType i=0; i<light_data.point_lights.size(); i++) {
			glm::vec4 zero(0.0f);
			PointLight cur_light = light_data.point_lights[i];
			if (glm::all(glm::equal(cur_light.pos, glm::vec4(camera.getPosition(), 1.0))) || (
					(glm::all(glm::equal(cur_light.color.ambient, zero))) &&
					(glm::all(glm::equal(cur_light.color.diffuse, zero))) &&
					(glm::all(glm::equal(cur_light.color.specular, zero)))
				)
			) {
				continue;
			}
			light_shader.setVec4("light_color", cur_light.color.diffuse);
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(cur_light.pos));
			model = glm::scale(model, glm::vec3(0.2f));

			light_shader.setMat4("model", model);
			cube.draw(light_shader);
		}

		// spot lights
		using SpotLightSizeType = std::remove_cvref_t<decltype(light_data.spot_lights)>::size_type;
		for (SpotLightSizeType i=0; i<light_data.spot_lights.size(); i++) {
			glm::vec4 zero(0.0f);
			SpotLight cur_light = light_data.spot_lights[i];
			if (glm::all(glm::equal(cur_light.pos, glm::vec4(camera.getPosition(), 1.0))) || (
					(glm::all(glm::equal(cur_light.color.ambient, zero))) &&
					(glm::all(glm::equal(cur_light.color.diffuse, zero))) &&
					(glm::all(glm::equal(cur_light.color.specular, zero)))
				)
			) {
				continue;
			}
			light_shader.setVec4("light_color", cur_light.color.diffuse);
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(cur_light.pos));
			model = glm::scale(model, glm::vec3(0.2f));

			light_shader.setMat4("model", model);
			cube.draw(light_shader);
		}

		// directional lights
		using DirLightSizeType = std::remove_cvref_t<decltype(light_data.directional_lights)>::size_type;
		for (DirLightSizeType i=0; i<light_data.directional_lights.size(); i++) {
			glm::vec4 zero(0.0f);
			DirectionalLight cur_light = light_data.directional_lights[i];
			if ((glm::all(glm::equal(cur_light.color.ambient, zero))) &&
				(glm::all(glm::equal(cur_light.color.diffuse, zero))) &&
				(glm::all(glm::equal(cur_light.color.specular, zero)))) {
				continue;
			}
			light_shader.setVec4("light_color", cur_light.color.diffuse);
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, camera.getPosition() + (glm::vec3(-cur_light.dir) * 100.0f));
			model = glm::scale(model, glm::vec3(10.0f));
			
			light_shader.setMat4("model", model);
			cube.draw(light_shader);
		}

		// skybox
		glCullFace(GL_FRONT);
		skybox_shader.activate();
		skybox_shader.setMat4("view", glm::mat4(glm::mat3(view)));
		skybox_shader.setMat4("projection", projection);
		cube.draw(skybox_shader);
		glCullFace(GL_BACK);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}