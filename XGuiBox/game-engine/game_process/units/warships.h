#pragma once
#include "../map_processing.h"
#include "../../tools/tools.h"

class warships_processing
{
public:
	void process_warships(int index, window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
	{
		for (int i = 0; i < g_map.units.size(); i++)
		{
			if (function_count == 2)
				return;

			if (!g_map.units[i].warship)
				continue;

			auto pos = ImVec2(countries->at(g_map.units[i].owner_country_id).position.x * animated_map_scale - (countries->at(g_map.units[i].owner_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
						      countries->at(g_map.units[i].owner_country_id).position.y * animated_map_scale - (countries->at(g_map.units[i].owner_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);


			auto building_pos = countries->at(g_map.units[i].owner_country_id).buildings[g_map.units[i].owner_building_id].pos;
			if (!g_map.units[i].spawnpos_converted_to_map)
			{
				g_map.units[i].converted_spawn_pos = ImVec2((g_map.units[i].spawn_pos.x - pos.x) / animated_map_scale, (g_map.units[i].spawn_pos.y - pos.y) / animated_map_scale);
				g_map.units[i].spawnpos_converted_to_map = true;
			}
			 
			ImVec2 final_pos = ImVec2(pos.x + g_map.units[i].converted_spawn_pos.x * animated_map_scale + g_map.units[i].move_offset.x * animated_map_scale, pos.y + g_map.units[i].converted_spawn_pos.y * animated_map_scale + g_map.units[i].move_offset.y * animated_map_scale);

			g_map.units[i].position = final_pos;

			if (ImGui::IsMouseClicked(1) && g_map.units[i].selected)
			{
				g_map.units[i].stored_cursor = cursor_pos;
				g_map.units[i].pos_converted_to_map = false;
			}

			ImVec2 mapped_pos = ImVec2((g_map.units[i].stored_cursor.x - pos.x) / animated_map_scale, (g_map.units[i].stored_cursor.y - pos.y) / animated_map_scale);

			bool target_is_empty = g_map.units[i].stored_cursor.x == 0 && g_map.units[i].stored_cursor.y == 0;
			if (!g_map.units[i].pos_converted_to_map)
			{
				{
					g_map.units[i].target_pos = mapped_pos;
					g_map.units[i].pos_converted_to_map = true;
				}
			}

			ImVec2 final_pos2;

			if (!target_is_empty)
				final_pos2 = ImVec2(pos.x + g_map.units[i].target_pos.x * animated_map_scale, pos.y + g_map.units[i].target_pos.y * animated_map_scale);
			else
				final_pos2 = final_pos;

			if (g_map.units[i].target_pos.x != 0 && g_map.units[i].target_pos.y != 0)
			{
				// Вектор направления к цели
				float dx = (final_pos2.x - g_map.units[i].position.x) / animated_map_scale;
				float dy = (final_pos2.y - g_map.units[i].position.y) / animated_map_scale;

				// Вычисление длины вектора
				float length = sqrt(dx * dx + dy * dy);

				if (length > 0.0f) // Чтобы избежать деления на 0
				{
					// Нормализация вектора
					float nx = dx / length;
					float ny = dy / length;

					// Задаем скорость движения (0.5 * animated_map_scale)
					float speed = 0.05f;

					bool blockLeft = false;
					bool blockRight = false;
					bool blockUp = false;
					bool blockDown = false;

					//collision with other boats
					for (int boat2 = 0; boat2 < g_map.units.size(); boat2++)
					{
						if (!g_map.units[i].warship && g_map.units[boat2].warship)
							continue;

						if (i == boat2)
							continue;


						float left1 = g_map.units[i].position.x - (4.5 * animated_map_scale);
						float right1 = g_map.units[i].position.x + (4.5 * animated_map_scale);
						float top1 = g_map.units[i].position.y + (4.5 * animated_map_scale);
						float bottom1 = g_map.units[i].position.y - (4.5 * animated_map_scale);

						float left2 = g_map.units[boat2].position.x - (4.5 * animated_map_scale);
						float right2 = g_map.units[boat2].position.x + (4.5 * animated_map_scale);
						float top2 = g_map.units[boat2].position.y + (4.5 * animated_map_scale);
						float bottom2 = g_map.units[boat2].position.y - (4.5 * animated_map_scale);

						if (right1 >= left2 && left1 <= right2 && bottom1 <= top2 && top1 >= bottom2)
						{

							if (right1 > left2 && left1 < left2) 
								blockRight = true;

							if (left1 < right2 && right1 > right2) 
								blockLeft = true;

							if (bottom1 < top2 && top1 > top2) 
								blockUp = true;

							if (top1 > bottom2 && bottom1 < bottom2) 
								blockDown = true;
						}
					}

					if (g_map.units[i].old_tick != g_map.global_tick)
					{
						if (nx > 0)
						{
							if (!blockRight)
								g_map.units[i].move_offset.x += nx * speed;
						}
						else if (nx < 0)
						{
							if (!blockLeft)
								g_map.units[i].move_offset.x += nx * speed;
						}

						if (ny > 0)
						{
							if (!blockDown)
								g_map.units[i].move_offset.y += ny * speed;
						}
						else if (ny < 0)
						{
							if (!blockUp)
								g_map.units[i].move_offset.y += ny * speed;
						}


						g_map.units[i].old_tick = g_map.global_tick;
					}
				}

			}


			ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.units[i].position, 2 * animated_map_scale, ImColor(150, 200, 180), 15);

			std::string class_name = "";
			
			switch (g_map.units[i].class_of_unit)
			{
				case boats::SUBMARINES:		class_name = "Submarine";	break;
				case boats::AIR_CARRIERS:	class_name = "Air Carrier"; break;
				case boats::DESTROYER:		class_name = "Destroyer";	break;
				case boats::CRUISERS:		class_name = "Cruiser";		break;
			}

			ImVec2 textsize = ImVec2(0, 0);

			if (!class_name.empty())
			{
				textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17.f, FLT_MAX, -1.f, class_name.c_str());

				ImGui::GetForegroundDrawList()->AddLine(g_map.units[i].position, final_pos2, g_map.units[i].selected ? ImColor(255, 255, 255) : ImColor(255, 255, 255, 100));

				ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(g_map.units[i].position.x - 1 * animated_map_scale, g_map.units[i].position.y - 1 * animated_map_scale), ImVec2(g_map.units[i].position.x + textsize.x + 1 * animated_map_scale, g_map.units[i].position.y + textsize.y + 1 * animated_map_scale), ImColor(0, 0, 0, 200));
				ImGui::GetForegroundDrawList()->AddRect(ImVec2(g_map.units[i].position.x - 1 * animated_map_scale, g_map.units[i].position.y - 1 * animated_map_scale), ImVec2(g_map.units[i].position.x + textsize.x + 1 * animated_map_scale, g_map.units[i].position.y + textsize.y + 1 * animated_map_scale), ImColor(255, 255, 255, 60));

				ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(g_map.units[i].position), ImColor(255, 255, 255), class_name.c_str());

				g_map.process_unit_selections(&g_map.units[i], animated_map_scale);
			}

		}
	}
};

inline warships_processing g_warships;