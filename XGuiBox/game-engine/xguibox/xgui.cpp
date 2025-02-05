#include "xgui.h"
#include "../resources/imgui_resource/imgui.h"
#include <sstream>
#include <windef.h>
#include <WinUser.h>


void xgui::add_font(const char* font, int size_of_font)
{
	font_modulation_class new_font;
	new_font.font_mode = new_font.mode_of_adding_font::NAME;

	new_font.font_name	   = font;
	new_font.size_of_font  = size_of_font;

	ImFont* font_date	   = new ImFont();
	new_font.font_addr     = font_date;

	fonts.push_back(new_font);

	delete font_date;

}

void xgui::add_font(void* font_data, int font_data_size, float size_of_font)
{
	font_modulation_class new_font;
	new_font.font_mode = new_font.mode_of_adding_font::MEMORY;

	new_font.font_data      = font_data;
	new_font.font_data_size = font_data_size;
	new_font.size_of_font   = size_of_font;

	ImFont* font_date       = new ImFont();
	new_font.font_addr      = font_date;

	fonts.push_back(new_font);

	delete font_date;

}

void xgui::create_frame(const char* name, ImVec2 size)
{

	index_of_movement_between_elements.y = 0;

	ImVec2 real_size = ImVec2(real_size_of_window_x, real_size_of_window_y);

	auto back = ImGui::GetBackgroundDrawList ();
	auto forw = ImGui::GetForegroundDrawList ();


	back->AddRectFilled(ImVec2(0, 0), real_size,			   ImColor(255, 255, 255));
}

bool xgui::checkbox(const char* name, bool* value)
{
	ImVec2 size_of_title = ImGui::CalcTextSize(name);

	auto back = ImGui::GetBackgroundDrawList();
	auto forw = ImGui::GetForegroundDrawList();
	ImVec2 current_pos = ImVec2(13, 40 + index_of_movement_between_elements.y);
	back->AddRect(current_pos, ImVec2(15 + current_pos.x, 15 + current_pos.y), ImColor(0, 0, 0));
	back->AddText(ImVec2(current_pos.x + 25, current_pos.y + 7 - size_of_title.y / 2), ImColor(0, 0, 0, 255), name);

	ImVec2 box_size = ImVec2(15, 15); // Размер квадратика чекбокса
	ImVec2 totalsize = ImVec2(size_of_title.x + 30 + current_pos.x, box_size.y + current_pos.y);

	index_of_movement_between_elements.y += 30;

	// Проверка нажатия и переключение состояния
	if (ImGui::IsMouseHoveringRect(current_pos, ImVec2(current_pos.x + totalsize.x, current_pos.y + totalsize.y)) && ImGui::IsMouseClicked(0))
	{
		*value = !*value;
	}

	// Отображение галочки, если значение true
	if (*value)
	{
		forw->AddLine(ImVec2(current_pos.x + 3, current_pos.y + 10), ImVec2(current_pos.x + 8, current_pos.y + 15), ImColor(0, 0, 0, 255), 2.0f);
		forw->AddLine(ImVec2(current_pos.x + 8, current_pos.y + 15), ImVec2(current_pos.x + 17, current_pos.y + 5), ImColor(0, 0, 0, 255), 2.0f);

	}
	return true;
}

bool xgui::button(const char* name)
{
	ImVec2 size_of_title = ImGui::CalcTextSize(name);

	auto back = ImGui::GetBackgroundDrawList();
	auto forw = ImGui::GetForegroundDrawList();

	ImVec2 current_pos = ImVec2(13, 40 + index_of_movement_between_elements.y);
	ImVec2 totalsize = ImVec2(size_of_title.x + 10 + current_pos.x, 30 + current_pos.y);

	
	back->AddRect(current_pos, totalsize, ImColor(0, 0, 0));
	back->AddText(ImVec2(current_pos.x + 5, current_pos.y + 15 - size_of_title.y / 2), ImColor(0, 0, 0, 255), name);


	index_of_movement_between_elements.y += 30;

	return true;
}