#pragma once
#include <vector>
#include "font_modulation_class.h"
#include "frameflags.h"
#include <wtypes.h>
#include <iostream>

class xgui
{
public:


	void add_font(const char* font, int size_of_font);
	void add_font(void* font_data, int font_data_size, float size_of_font);
	std::vector < font_modulation_class > fonts;

	void create_frame(const char* name, ImVec2 size);
	void edit_style(frame_flags color, ImColor new_color);

	HWND hwnd_of_process;
	int real_size_of_window_x;
	int real_size_of_window_y;


	ImVec2 index_of_movement_between_elements;
	bool checkbox(const char* name, bool* value);
	bool button  (const char* name);
	bool slider  (const char* name, float min, float max);

private:


};
inline xgui g_xgui;
