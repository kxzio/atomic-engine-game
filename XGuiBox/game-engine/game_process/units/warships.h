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
				float dx = final_pos2.x - g_map.units[i].position.x;
				float dy = final_pos2.y - g_map.units[i].position.y;

				// Вычисление длины вектора
				float length = sqrt(dx * dx + dy * dy);

				if (length > 0.0f) // Чтобы избежать деления на 0
				{
					// Нормализация вектора
					float nx = dx / length;
					float ny = dy / length;

					// Задаем скорость движения (0.5 * animated_map_scale)
					float speed = 0.5f * animated_map_scale;

					if (g_map.units[i].old_tick != g_map.global_tick)
					{
						// Перемещение в правильном направлении с учетом скорости
						g_map.units[i].move_offset.x += nx * speed;
						g_map.units[i].move_offset.y += ny * speed;

						g_map.units[i].old_tick = g_map.global_tick;
					}
				}
			}


			ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.units[i].position, 2 * animated_map_scale, ImColor(150, 200, 180), 15);

			ImGui::GetForegroundDrawList()->AddLine(g_map.units[i].position, final_pos2, ImColor(255, 255, 255));

			g_map.process_unit_selections(&g_map.units[i], animated_map_scale);


		}
	}
};

inline warships_processing g_warships;