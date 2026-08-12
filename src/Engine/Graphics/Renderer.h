#ifndef ENGINE_GRAPHICS_RENDERER_H
#define ENGINE_GRAPHICS_RENDERER_H

#include "GraphicsConfig.h"
#include "BasicEntity.h"
#include "Camera.h"
#include "GUI.h"
#include "Input.h"
#include "RawModel.h"
#include "Shader.h"
#include "Texture.h"

class Renderer {
private:
	int width;
	int height;
	int active_shader;
	bool focused;
	bool active;
	bool def_gui_en;
	float sun_light;
	float weather_factor;
	GLFWwindow* window;
	Shader* shader_3d;
	Shader* shader_2d;
	Shader* shader_3dbg;
	Camera* currentCamera;
	RawModel* default_gui;
	RawModel* bg_render_assist;
	glm::vec3 sky_color;
	glm::vec3 horizon_color;

	void calculate_sky_color(float rain_fac, float time_fac) {
		sky_color.x = (1.0f - rain_fac) * time_fac * 0.4f + (rain_fac) * time_fac * 0.5f;
		sky_color.y = (1.0f - rain_fac) * time_fac * 0.6f + (rain_fac) * time_fac * 0.4f;
		sky_color.z = 0.9f - rain_fac * 0.65;

		horizon_color.x = 0.7f - rain_fac * 0.3f - time_fac * 0.1f;
		horizon_color.y = 0.7f - rain_fac * 0.3f - time_fac * 0.1f;
		horizon_color.z = 0.8f - rain_fac * 0.4f;
	}

public:

	Renderer() {
		shader_2d = nullptr;
		shader_3d = nullptr;
		shader_3dbg = nullptr;
		width = 800;
		height = 600;
		sky_color = glm::vec3(0.5f, 0.6f, 0.8f);
		horizon_color = glm::vec3(0.7f, 0.7f, 0.7f);
		active = false;
		focused = false;
		_focus_pointer = &focused;
		active_shader = 0;
		_key_status = new int[512];
		currentCamera = nullptr;
		window = nullptr;
		default_gui = nullptr;
		bg_render_assist = nullptr;
		def_gui_en = false;
		sun_light = 1.0f;
		weather_factor = 0.0f;
	}

	bool initializeWindow(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT, const char* title = "My Game") {
		this->width = width;
		this->height = height;
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		glViewport(0, 0, width, height);

		glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);

		glfwSetWindowFocusCallback(window, _window_focus_callback);

		glfwSetKeyCallback(window, _keyboard_key_callback);

		glfwSetCursorPosCallback(window, inGameCursorPositionCallback);

		glfwSetMouseButtonCallback(window, _mouse_key_callback);

		glfwSwapInterval(1);

		glActiveTexture(GL_TEXTURE0);

		active = true;

		def_gui_en = false;
		default_gui = new RawModel();
		float ver[] = {
			-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			1.0f , -1.0f, 0.0f,  1.0f, 1.0f,
			1.0f ,  1.0f, 0.0f,  1.0f, 0.0f,

			-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			1.0f ,  1.0f, 0.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,  0.0f, 0.0f
		};
		default_gui->initFromData(ver, 30);

		bg_render_assist = new RawModel();
		float ver2[] = {
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, 1.0f
		};
		bg_render_assist->initFromData(ver2, 12, 2);

		return 0;
	}

	void initialize3DShader(const char* vshaderpath, const char* fshaderpath) {
		shader_3d = new Shader(vshaderpath, fshaderpath);
	}

	void initialize2DShader(const char* vshaderpath, const char* fshaderpath) {
		shader_2d = new Shader(vshaderpath, fshaderpath);
	}

	void initialize3DBackgroundShader(const char* vshaderpath, const char* fshaderpath) {
		shader_3dbg = new Shader(vshaderpath, fshaderpath);
	}

	void prepare() {
		glClearColor(sky_color.x * sun_light, sky_color.y * sun_light, sky_color.z * sun_light, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		calculate_sky_color(weather_factor, sun_light);
	}

	void prepare2D() {
		active_shader = 2;
		shader_2d->enableShaderProgram();
		glDisable(GL_DEPTH_TEST);
	}

	void prepare3D() {
		active_shader = 3;
		shader_3d->enableShaderProgram();
		glEnable(GL_DEPTH_TEST);
		shader_3d->loadMatrix4f("view", currentCamera->getViewMatrix());
		shader_3d->loadMatrix4f("projection", currentCamera->getProjectionMatrix());
		shader_3d->loadUniform1f("light_factor", sun_light);
		if (_aspect_ratio_updated) {
			currentCamera->updateAspectRatio((float)_cwidth / (float)_cheight);
			shader_3d->loadMatrix4f("projection", currentCamera->getProjectionMatrix());
		}
		shader_3d->loadUniform1f("fogDensity", 0.007f);
		shader_3d->loadUniform3f("fog_color", horizon_color);
	}

	void prepareBackground() {
		active_shader = 30;
		shader_3dbg->enableShaderProgram();
		shader_3dbg->loadUniform2f("screen", (float)_cwidth, (float)_cheight);
		shader_3dbg->loadUniform1f("light_factor", sun_light);
		shader_3dbg->loadUniform3f("color_sky", sky_color);
		shader_3dbg->loadUniform3f("color_horizon", horizon_color);
		shader_3dbg->loadUniform3f("view_direction", currentCamera->getLookingVector());
		glDisable(GL_DEPTH_TEST);
	}

	void setLightLevel(float factor) {
		sun_light = factor;
	}

	void setWeatherLevel(float weatherstate) {
		weather_factor = weatherstate;
	}

	void setCursorMode(int mode) {
		if (mode == 1) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			_mouse_captured = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			_mouse_captured = false;
		}
	}

	void renderBackground() {
		bg_render_assist->bindModel();
		glDrawArrays(GL_TRIANGLES, 0, bg_render_assist->getSize());
		bg_render_assist->unbindModel();
	}

	void renderGUIScene(GUIScene& scene) {

		// Prepare 
		
		default_gui->bindModel();
		unsigned int current_bound_texture = 0;

		// GUIs

		int guis = scene.guiCnt();
		for (int gui_idx = 0; gui_idx < guis; gui_idx++) {
			if (!scene.guiAt(gui_idx).hasSameTexture(current_bound_texture)) {
				glBindTexture(GL_TEXTURE_2D, scene.guiAt(gui_idx).getTexture()->getID());
				current_bound_texture = scene.guiAt(gui_idx).getTexture()->getID();
			}
			glm::vec4 bounds = scene.guiAt(gui_idx).calculateBounds(_cwidth, _cheight);
			glm::vec4 atlasv = scene.guiAt(gui_idx).calculateAtlas();
			shader_2d->loadUniform4f("trans_values", bounds.x, bounds.y, bounds.z, bounds.w);
			shader_2d->loadUniform4f("atlas_values", atlasv.x, atlasv.y, atlasv.z, atlasv.w);
			glDrawArrays(GL_TRIANGLES, 0, default_gui->getSize());
		}

		// Texts

		int texts = scene.textCnt();
		for (int text_idx = 0; text_idx < texts; text_idx++) {
			if (!scene.textAt(text_idx).hasSameTexture(current_bound_texture)) {
				glBindTexture(GL_TEXTURE_2D, scene.textAt(text_idx).getTexture()->getID());
				current_bound_texture = scene.textAt(text_idx).getTexture()->getID();
			}
			int stlen = scene.textAt(text_idx).getStrlen();
			glm::vec4 baseBounds = scene.textAt(text_idx).calculateBounds(_cwidth, _cheight);
			for (int j = 0; j < stlen; j++) {
				shader_2d->loadUniform4f("trans_values", baseBounds.x, baseBounds.y, baseBounds.z + scene.textAt(text_idx).getPositionOffset(_cwidth, j), baseBounds.w);
				glm::vec4 atlasValues = scene.textAt(text_idx).calculateAtlas(j);
				shader_2d->loadUniform4f("atlas_values", atlasValues.x, atlasValues.y, atlasValues.z, atlasValues.w);
				glDrawArrays(GL_TRIANGLES, 0, default_gui->getSize());
			}
		}

		// End

		glBindTexture(GL_TEXTURE_2D, 0);
		default_gui->unbindModel();

	}

	void renderBasicEntity(BasicEntity& entity) {

		glm::vec4 atlas_values = entity.getTexture()->getAtlasCoordinate(entity.getAtlasIndex());

		shader_3d->loadUniform2f("coordFactors", atlas_values.z, atlas_values.w);
		shader_3d->loadUniform2f("coordOffsets", atlas_values.x, atlas_values.y * entity.atlasYMultiplyer());
		
		shader_3d->loadMatrix4f("transform", entity.getTransformationMatrix());
		entity.getModel()->bindModel();
		glBindTexture(GL_TEXTURE_2D, entity.getTexture()->getID());
		glDrawArrays(GL_TRIANGLES, 0, entity.getModel()->getSize());
		glBindTexture(GL_TEXTURE_2D, 0);
		entity.getModel()->unbindModel();
	}
	
	void renderChunk(Texture* texture, glm::mat4 transform, unsigned int vbolen, unsigned int vao, bool first = false) {
		if (first) {
			glBindTexture(GL_TEXTURE_2D, texture->getID());
			shader_3d->loadUniform2f("coordFactors", 1.0f, 1.0f);
			shader_3d->loadUniform2f("coordOffsets", 0.0f, 0.0f);
		}

		shader_3d->loadMatrix4f("transform", transform);

		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, vbolen);

		glBindVertexArray(0);
	}

	void updateDisplay() {
		_aspect_ratio_updated = false;

		glfwSwapBuffers(window);
		glfwPollEvents();

		active_shader = 0;
	}

	void destroy() {
		if (default_gui) {
			default_gui->destroyModel();
			delete default_gui;
			default_gui = nullptr;
		}

		if (bg_render_assist) {
			bg_render_assist->destroyModel();
			delete bg_render_assist;
			bg_render_assist = nullptr;
		}

		if (shader_3d) {
			delete shader_3d;
			shader_3d = nullptr;
		}

		if (shader_2d) {
			delete shader_2d;
			shader_2d = nullptr;
		}

		if (shader_3dbg) {
			shader_3dbg->deleteShaderProgram();
			delete shader_3dbg;
			shader_3dbg = nullptr;
		}

		if (active) {
			glfwTerminate();
			delete[] _key_status;
			_key_status = nullptr;
		}

		active = false;
	}

	void setCurrentCamera(Camera* camera) {
		camera->updateAspectRatio((float)width / (float)height);
		_ptr_active_camera = camera;
		this->currentCamera = camera;
	}

	bool isCloseRequested() {
		return glfwWindowShouldClose(window);
	}

	GLFWwindow* getWindow() {
		return window;
	}

	void getMouseCoordinate(float& x, float& y) {
		double cx, cy;
		glfwGetCursorPos(window, &cx, &cy);
		x = (float)(cx);
		y = (float)(cy);
	}

	void deactivateShaders() {
		active_shader = 0;
		glUseProgram(0);
	}

	~Renderer() {
		destroy();
	}

};

#endif
