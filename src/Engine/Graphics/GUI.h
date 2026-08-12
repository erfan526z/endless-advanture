#ifndef ENGINE_GRAPHICS_GUI_H
#define ENGINE_GRAPHICS_GUI_H

#include "GraphicsConfig.h"
#include "Texture.h"

class GUIImage {
public:

	GUIImage() {
		texture = nullptr;
		g_pos_x = g_pos_y = 0;
		g_width = g_height = 16;
		g_factor = 1;
		g_xalign = g_yalign = 0;
		g_atlas = 0;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	GUIImage(Texture* texture, int x_position, int y_position, int width, int height, int x_alignment, int y_alignment, int gui_scale, int atlas_index) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_width = width;
		g_height = height;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		g_atlas = atlas_index;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	GUIImage(Texture* texture, int x_position, int y_position, int height, int x_alignment, int y_alignment, int gui_scale, int atlas_index) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_width = height;
		g_height = height;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		g_atlas = atlas_index;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	void setPosition(int x, int y) {
		g_pos_x = x;
		g_pos_y = y;
		bound_changed = true;
	}

	void setSize(int height, int width = -1) {
		if (width == -1)
			g_width = g_height = height;
		else {
			g_width = width;
			g_height = height;
		}
		bound_changed = true;
	}

	void setAlignment(int xalign, int yalign) {
		g_xalign = xalign;
		g_yalign = yalign;
		bound_changed = true;
	}

	void setGUIScale(int factor) {
		g_factor = factor;
		bound_changed = true;
	}

	void setAtlasIndex(int index) {
		if (g_atlas == index) return;
		g_atlas = index;
		atlas_changed = true;
	}

	glm::vec4 calculateBounds(int screen_width, int screen_height) {
		if (last_screen_width == screen_width && last_screen_height == screen_height && !bound_changed)
			return bounds;
		bound_changed = false;
		last_screen_width = screen_width;
		last_screen_height = screen_height;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_width = g_factor * g_width;
		int n_height = g_factor * g_height;
		int position_pixel_x = (!g_xalign) ? (screen_width / 2 + n_pos_x) : (g_xalign == 1) ? (screen_width - n_pos_x) - n_width / 2 : n_pos_x + n_width / 2;
		int position_pixel_y = (!g_yalign) ? (screen_height / 2 + n_pos_y) : (g_yalign == 1) ? (screen_height - n_pos_y) - n_height / 2 : n_pos_y + n_height / 2;
		float px = (float)(position_pixel_x * 2) / (float)(screen_width) - 1.0f;
		float py = (float)(position_pixel_y * 2) / (float)(screen_height) - 1.0f;
		float sx = (float)n_width / (float)(screen_width);
		float sy = (float)n_height / (float)(screen_height);
		bounds = glm::vec4(sx, sy, px, py);
		return bounds;
	}

	glm::vec4 calculateAtlas() {
		if (!atlas_changed) return atlas;
		atlas_changed = false;
		if (!texture) return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		return atlas = texture->getAtlasCoordinate(g_atlas);
	}

	Texture* getTexture() {
		return texture;
	}

	bool hasSameTexture(unsigned int t_id) {
		return texture->getID() == t_id;
	}

	bool isMouseInside(float mouse_x, float mouse_y) {
		mouse_y = (float)last_screen_height - mouse_y;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_width = g_factor * g_width;
		int n_height = g_factor * g_height;
		int start_x = (!g_xalign) ? (last_screen_width / 2 + n_pos_x) - n_width / 2 :
			(g_xalign == 1) ? (last_screen_width - n_pos_x) - n_width : n_pos_x;
		int start_y = (!g_yalign) ? (last_screen_height / 2 + n_pos_y) - n_height / 2 : 
			(g_yalign == 1) ? (last_screen_height - n_pos_y) - n_height : n_pos_y;
		int end_x = start_x + n_width;
		int end_y = start_y + n_height;
		return (mouse_x >= start_x && mouse_x <= end_x && mouse_y >= start_y && mouse_y <= end_y);
	}

private:

	Texture* texture; // Pointer to the texture

	int g_pos_x; // Horizontal position in pixels

	int g_pos_y; // Vertical position in pixels

	int g_width; // Width in pixels

	int g_height; // Height in pixels

	int g_factor; // Scale factor (All pixel values will multiply by this, default: 1)

	int g_xalign; // Horizontal alignment

	int g_yalign; // Vertical alignement

	int g_atlas; // Texture Atlas Index

	bool bound_changed; // If nothing is updated, there is no need to recalculate bounds.

	bool atlas_changed; // If atlas index is the same, there is no need to recalculate altas vec4.

	glm::vec4 bounds;

	glm::vec4 atlas;

	int last_screen_width = 0;

	int last_screen_height = 0;

};

class GUIText {
public:

	GUIText() {
		texture = nullptr;
		g_pos_x = g_pos_y = 0;
		g_char_width = 16;
		g_factor = 1;
		g_xalign = g_yalign = 0;
		bound_changed = true;
		bounds = glm::vec4(0.0f);
		text[0] = 0;
	}

	GUIText(Texture* texture, const char* text, int x_position, int y_position, int char_width, int x_alignment, int y_alignment, int gui_scale) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_char_width = char_width;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		bound_changed = true;
		bounds = glm::vec4(0.0f);

		if (!text) {
			this->text[0] = 0;
			return;
		}

		int l1 = strlen(text) + 1;
		if (l1 >= 256) l1 = 256;
		memcpy(this->text, text, l1); // +1 is because we also copy '\0' at the end.
		this->text[255] = 0; // if the input text size was bigger than buffer size, it needs this to prevent access violation errors.
	}

	void setPosition(int x, int y) {
		g_pos_x = x;
		g_pos_y = y;
		bound_changed = true;
	}

	void setSize(int char_width) {
		g_char_width = char_width;
		bound_changed = true;
	}

	void setAlignment(int xalign, int yalign) {
		g_xalign = xalign;
		g_yalign = yalign;
		bound_changed = true;
	}

	void setGUIScale(int factor) {
		g_factor = factor;
		bound_changed = true;
	}

	void setText(const char* text) {
		if (!text) {
			this->text[0] = 0;
			return;
		}
		int l1 = strlen(text) + 1;
		if (l1 >= 256) l1 = 256;
		memcpy(this->text, text, l1); // +1 is because we also copy '\0' at the end.
		this->text[255] = 0; // if the input text size was bigger than buffer size, it needs this to prevent access violation errors.
		bound_changed = true;
	}

	glm::vec4 calculateBounds(int screen_width, int screen_height) {
		if (last_screen_width == screen_width && last_screen_height == screen_height && !bound_changed)
			return bounds;
		bound_changed = false;
		last_screen_width = screen_width;
		last_screen_height = screen_height;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_char_width = g_factor * g_char_width;
		int position_pixel_x = (!g_xalign) ? (screen_width / 2 + n_pos_x) - (getStrlen() * n_char_width - n_char_width) / 2 : (g_xalign == 1) ? (screen_width - n_pos_x) - n_char_width / 2 : n_pos_x + n_char_width / 2;
		int position_pixel_y = (!g_yalign) ? (screen_height / 2 + n_pos_y) : (g_yalign == 1) ? (screen_height - n_pos_y) - n_char_width / 2 : n_pos_y + n_char_width / 2;
		float px = (float)(position_pixel_x * 2) / (float)(screen_width)-1.0f;
		float py = (float)(position_pixel_y * 2) / (float)(screen_height)-1.0f;
		float sx = (float)n_char_width / (float)(screen_width);
		float sy = (float)n_char_width / (float)(screen_height);
		bounds = glm::vec4(sx, sy, px, py);
		return bounds;
	}

	float getPositionOffset(int screen_width, int character) {
		int pixels = character * g_char_width * g_factor;
		return (float)(pixels * 2) / (float)(screen_width);
	}

	glm::vec4 calculateAtlas(int character) {
		if (character >= 256) return glm::vec4(0.0f);
		return texture->getAtlasCoordinate(text[character]);
	}

	Texture* getTexture() {
		return texture;
	}

	int getStrlen() {
		return strlen(text);
	}

	bool hasSameTexture(unsigned int t_id) {
		return texture->getID() == t_id;
	}

private:

	char text[256];

	Texture* texture; // Pointer to the font texture

	int g_pos_x; // Horizontal position in pixels

	int g_pos_y; // Vertical position in pixels

	int g_char_width; // Character width in pixels

	int g_factor; // Scale factor (All pixel values will multiply by this, default: 1)

	int g_xalign; // Horizontal alignment

	int g_yalign; // Vertical alignement

	bool bound_changed; // If nothing is updated, there is no need to recalculate bounds.

	glm::vec4 bounds;

	int last_screen_width = 0;

	int last_screen_height = 0;
};

class GUIScene {

public:

	GUIScene() {}

	~GUIScene() {
		guis.clear();
		texts.clear();
	}

	void add(GUIImage& gui) {
		guis.push_back(&gui);
	}

	void add(GUIText& text) {
		texts.push_back(&text);
	}

	void clear() {
		guis.clear();
		texts.clear();
	}

	GUIImage& guiAt(int i) {
		return *guis.at(i);
	}

	GUIText& textAt(int i) {
		return *texts.at(i);
	}

	int guiCnt() {
		return guis.size();
	}

	int textCnt() {
		return texts.size();
	}

	void setGUIScaleForAll(int gui_scale) {
		for (GUIImage* gui : guis)
			gui->setGUIScale(gui_scale);
		for (GUIText* text : texts)
			text->setGUIScale(gui_scale);
	}

private:
	std::vector<GUIImage*> guis;
	std::vector<GUIText*> texts;

};

#endif
