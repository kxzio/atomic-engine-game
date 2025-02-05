#pragma once
#include "../resources/imgui_resource/imgui.h"
class font_modulation_class
{
public:

	enum mode_of_adding_font
	{
		NAME,
		MEMORY
	};

	int font_mode = -1;

	const char* font_name; int size_of_font; void* font_data; int font_data_size;

	ImFont* font_addr;
};