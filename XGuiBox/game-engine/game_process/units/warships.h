#pragma once
#include "../map_processing.h"
#include "../../tools/tools.h"

class warships_processing
{
public:

	int old_tick_for_update = 0;

	bool doLinesIntersect(float x1, float y1, float x2, float y2,
		float x3, float y3, float x4, float y4)
	{
		auto crossProduct = [](float ax, float ay, float bx, float by) {
			return ax * by - ay * bx;
			};

		float d1 = crossProduct(x2 - x1, y2 - y1, x3 - x1, y3 - y1);
		float d2 = crossProduct(x2 - x1, y2 - y1, x4 - x1, y4 - y1);
		float d3 = crossProduct(x4 - x3, y4 - y3, x1 - x3, y1 - y3);
		float d4 = crossProduct(x4 - x3, y4 - y3, x2 - x3, y2 - y3);

		return (d1 * d2 < 0) && (d3 * d4 < 0);
	}

	bool isPathBlockedByLand(float startX, float startY, float endX, float endY)
	{
		for (const auto& country : g_map.countries)
		{
			const auto& hull = country.convex_hull; // Контур страны

			for (size_t i = 0; i < hull.size(); i++)
			{
				float x1 = hull[i].x;
				float y1 = hull[i].y;
				float x2 = hull[(i + 1) % hull.size()].x;
				float y2 = hull[(i + 1) % hull.size()].y;

				if (doLinesIntersect(startX, startY, endX, endY, x1, y1, x2, y2))
					return true; // Лодка пересекает сушу
			}
		}
		return false; // Вода чистая
	}


	void process_warships(NavigableArea nav_area, std::vector <country_data>* countries, float animated_map_scale, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
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

				if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
				{
					g_socket_control.server_send_unit(g_map.units[i].unique_id);
				}
				else if (g_socket_control.player_role == g_socket_control.player_role_enum::CLIENT)
				{
					g_socket_control.client_send_unit(g_map.units[i].unique_id);
				}
			}
			 
			g_map.units[i].interpolated_move_offset.x = ImGui::LerpAnimate(std::to_string(i).c_str(),"InterpX", true, g_map.units[i].move_offset.x, 100, ImGui::INTERP);
			g_map.units[i].interpolated_move_offset.y = ImGui::LerpAnimate(std::to_string(i).c_str(), "InterpY", true, g_map.units[i].move_offset.y, 100, ImGui::INTERP);

			ImVec2 final_pos = ImVec2(pos.x + g_map.units[i].converted_spawn_pos.x * animated_map_scale + g_map.units[i].interpolated_move_offset.x * animated_map_scale, pos.y + g_map.units[i].converted_spawn_pos.y * animated_map_scale + g_map.units[i].interpolated_move_offset.y * animated_map_scale);

			g_map.units[i].position = final_pos;

			if (ImGui::IsMouseClicked(1) && g_map.units[i].selected)
			{
				if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
				{
					g_map.units[i].path.clear();
				}

				if (g_map.units[i].owner_country_id == g_menu.players[player_id].control_region)
				{
					g_map.units[i].stored_cursor = cursor_pos;
					g_map.units[i].pos_converted_to_map = false;
				}
			}

			ImVec2 mapped_pos = ImVec2((g_map.units[i].stored_cursor.x - pos.x) / animated_map_scale, (g_map.units[i].stored_cursor.y - pos.y) / animated_map_scale);

			bool target_is_empty = g_map.units[i].stored_cursor.x == 0 && g_map.units[i].stored_cursor.y == 0;

			if (!g_map.units[i].pos_converted_to_map)
			{
				if (g_map.units[i].stored_cursor.x != 0 && g_map.units[i].stored_cursor.y != 0)
				{
					//g_map.units[i].target_pos = mapped_pos;

					g_map.units[i].pos_converted_to_map = true;

					ImVec2 mapped_path_pos = ImVec2(pos.x + mapped_pos.x * animated_map_scale, pos.y + mapped_pos.y * animated_map_scale);

					std::vector<ImVec2> path_validity;

					if (true) {
						if (g_map.units[i].path.size() == 0) {
							path_validity.push_back(final_pos);
						}
						else {
							const ImVec2& last_point = g_map.units[i].path.back();  // Последняя точка пути
							ImVec2 mapped_path_pos2 = ImVec2(
								pos.x + last_point.x * animated_map_scale,
								pos.y + last_point.y * animated_map_scale
							);
							path_validity.push_back(mapped_path_pos2);
						}
					}


					path_validity.push_back(mapped_path_pos);

					if (nav_area.IsPathValid(path_validity, map_pos, animated_map_scale))
					{
						g_map.units[i].path.push_back(mapped_pos);
					}

					
				}
			}

			std::vector<ImVec2> mapped_path;
			mapped_path.push_back(final_pos);

			if (!g_map.units[i].path.empty())
			{
				for (int pathid = 0; pathid < g_map.units[i].path.size(); pathid++)
				{
					ImVec2 mapped_path_pos = ImVec2(pos.x + g_map.units[i].path[pathid].x * animated_map_scale, pos.y + g_map.units[i].path[pathid].y * animated_map_scale);
					mapped_path.push_back(mapped_path_pos);
				}
			}

			if (!g_map.units[i].path.empty())
				g_map.units[i].target_pos = g_map.units[i].path[0];

			ImVec2 final_pos2;

			if (!target_is_empty)
				final_pos2 = ImVec2(pos.x + g_map.units[i].target_pos.x * animated_map_scale, pos.y + g_map.units[i].target_pos.y * animated_map_scale);
			else
				final_pos2 = final_pos;

			if (g_map.units[i].owner_country_id == g_menu.players[player_id].control_region)
			{

				float dx = (final_pos2.x - g_map.units[i].position.x) / animated_map_scale;
				float dy = (final_pos2.y - g_map.units[i].position.y) / animated_map_scale;

				float length = sqrt(dx * dx + dy * dy);

				if (length > 0.0f)
				{
					float nx = dx / length;
					float ny = dy / length;
					float speed = 0.045f;

					bool blockLeft = false, blockRight = false, blockUp = false, blockDown = false;
					int left_count = 0, right_count = 0, up_count = 0, down_count = 0;

					float sensor_offset = 9.0f * animated_map_scale;

					for (int boat2 = 0; boat2 < g_map.units.size(); boat2++)
					{
						if (!g_map.units[i].warship && g_map.units[boat2].warship) continue;
						if (i == boat2) continue;

						float left1 = g_map.units[i].position.x - (6.5 * animated_map_scale);
						float right1 = g_map.units[i].position.x + (6.5 * animated_map_scale);
						float top1 = g_map.units[i].position.y + (6.5 * animated_map_scale);
						float bottom1 = g_map.units[i].position.y - (6.5 * animated_map_scale);

						float left2 = g_map.units[boat2].position.x - (6.5 * animated_map_scale);
						float right2 = g_map.units[boat2].position.x + (6.5 * animated_map_scale);
						float top2 = g_map.units[boat2].position.y + (6.5 * animated_map_scale);
						float bottom2 = g_map.units[boat2].position.y - (6.5 * animated_map_scale);

						float sensor_x_forward = g_map.units[i].position.x + nx * sensor_offset;
						float sensor_y_forward = g_map.units[i].position.y + ny * sensor_offset;

						float sensor_x_back = g_map.units[i].position.x - nx * sensor_offset;
						float sensor_y_back = g_map.units[i].position.y - ny * sensor_offset;

						float sensor_x_left = g_map.units[i].position.x - ny * sensor_offset;
						float sensor_y_left = g_map.units[i].position.y + nx * sensor_offset;

						float sensor_x_right = g_map.units[i].position.x + ny * sensor_offset;
						float sensor_y_right = g_map.units[i].position.y - nx * sensor_offset;

						bool forward_collision = (sensor_x_forward >= left2 && sensor_x_forward <= right2 &&
							sensor_y_forward <= top2 && sensor_y_forward >= bottom2);

						bool back_collision = (sensor_x_back >= left2 && sensor_x_back <= right2 &&
							sensor_y_back <= top2 && sensor_y_back >= bottom2);

						bool left_collision = (sensor_x_left >= left2 && sensor_x_left <= right2 &&
							sensor_y_left <= top2 && sensor_y_left >= bottom2);

						bool right_collision = (sensor_x_right >= left2 && sensor_x_right <= right2 &&
							sensor_y_right <= top2 && sensor_y_right >= bottom2);

						if (forward_collision)
						{
							blockRight |= (sensor_x_forward > left2);
							blockLeft |= (sensor_x_forward < right2);
							blockUp |= (sensor_y_forward > bottom2);
							blockDown |= (sensor_y_forward < top2);
						}

						if (left_collision) left_count++;
						if (right_collision) right_count++;
						if (forward_collision) up_count++;
						if (back_collision) down_count++;

						float dist_x = g_map.units[boat2].position.x - g_map.units[i].position.x;
						float dist_y = g_map.units[boat2].position.y - g_map.units[i].position.y;
						float dist = sqrt(dist_x * dist_x + dist_y * dist_y);
						float min_dist = 12.0f * animated_map_scale;

						float distance_5_ticks_before = sqrt(pow(g_map.units[i].move_offset.x - g_map.units[i].old_position.x, 2) + pow(g_map.units[i].move_offset.y - g_map.units[i].old_position.y, 2)) * 150;

						if (distance_5_ticks_before > 5)
						{
							if (g_map.global_tick % 7 == 0 && g_map.units[i].stuck_tick_timer < 200)
								g_map.units[i].stuck_tick_timer++;
						}
						else
						{
							if (g_map.global_tick % 7 == 0 && g_map.units[i].stuck_tick_timer > 1)
								g_map.units[i].stuck_tick_timer--;
						}

						//ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(g_map.units[i].position), ImColor(255, 255, 255), std::to_string(g_map.units[i].stuck_tick_timer).c_str());

						if (i < boat2 && dist < min_dist && g_map.units[i].stuck_tick_timer == 1) 
						{
							float repel_force = 0.2f * (1.0f - (dist / min_dist));

							if (g_map.units[i].last_path_update + 100 < g_map.global_tick)
							{
								float enemy_move_x = g_map.units[boat2].move_offset.x - g_map.units[boat2].old_position.x;
								float enemy_move_y = g_map.units[boat2].move_offset.y - g_map.units[boat2].old_position.y;

								bool prefer_left = false;
								bool prefer_up = false;

								if (fabs(enemy_move_x) > fabs(enemy_move_y)) 
								{
									if (enemy_move_x > 0) 
										prefer_left = true;
									else 
										prefer_left = false;
								}
								else 
								{
									if (enemy_move_y > 0) 
										prefer_up = true;
									else 
										prefer_up = false;
								}

								ImVec2 detour_point;

								if (fabs(dist_x) > fabs(dist_y))
								{
									if (prefer_left)
										detour_point = ImVec2((g_map.units[i].position.x - pos.x) / animated_map_scale - 7,
											(g_map.units[i].position.y - pos.y) / animated_map_scale);
									else
										detour_point = ImVec2((g_map.units[i].position.x - pos.x) / animated_map_scale + 7,
											(g_map.units[i].position.y - pos.y) / animated_map_scale);
								}
								else 
								{
									if (prefer_up)
										detour_point = ImVec2((g_map.units[i].position.x - pos.x) / animated_map_scale,
											(g_map.units[i].position.y - pos.y) / animated_map_scale + 7);
									else
										detour_point = ImVec2((g_map.units[i].position.x - pos.x) / animated_map_scale,
											(g_map.units[i].position.y - pos.y) / animated_map_scale - 7);
								}

								g_map.units[i].path.insert(g_map.units[i].path.begin(), detour_point);
								g_map.units[i].last_path_update = g_map.global_tick;
							}
						}
					}

					if (blockUp && ny < 0)
					{
						ny = 0.1f;
						nx *= 0.6f;
					}
					if (blockDown && ny > 0)
					{
						ny = -0.1f;
						nx *= 0.6f;
					}
					if (blockLeft && nx < 0)
					{
						nx = 0.1f;
						ny *= 0.6f;
					}
					if (blockRight && nx > 0)
					{
						nx = -0.1f;
						ny *= 0.6f;
					}

					if (right_count > left_count + 1) ny += 0.7f;
					if (left_count > right_count + 1) ny -= 0.7f;
					if (up_count > down_count + 1) nx += 0.7f;
					if (down_count > up_count + 1) nx -= 0.7f;

					float random_factor = ((rand() % 100) / 4000.0f) - 0.01f;
					nx += random_factor;
					ny += random_factor;

					if (g_map.units[i].old_tick != g_map.global_tick)
					{
						g_map.units[i].move_offset.x += nx * speed;
						g_map.units[i].move_offset.y += ny * speed;
						g_map.units[i].old_tick = g_map.global_tick;
					}
				}

				if (length < 2.0f && !g_map.units[i].path.empty())
					g_map.units[i].path.erase(g_map.units[i].path.begin());

				if (g_map.global_tick % 5 == 0)
				{
					g_map.units[i].old_position = g_map.units[i].move_offset;
				}

			}



			ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.units[i].position, 2 * animated_map_scale, g_menu.players[player_id].control_region == g_map.units[i].owner_country_id ? ImColor(150, 200, 180) : ImColor(255, 40, 10), 15);

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

				ImGui::GetForegroundDrawList()->AddPolyline(mapped_path.data(), mapped_path.size(), g_map.units[i].selected ? ImColor(255, 255, 255) : ImColor(255, 255, 255, 100), 0, 1.f);

				ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(g_map.units[i].position.x - 1 * animated_map_scale, g_map.units[i].position.y - 1 * animated_map_scale), ImVec2(g_map.units[i].position.x + textsize.x + 1 * animated_map_scale, g_map.units[i].position.y + textsize.y + 1 * animated_map_scale), ImColor(0, 0, 0, 200));
				ImGui::GetForegroundDrawList()->AddRect(ImVec2(g_map.units[i].position.x - 1 * animated_map_scale, g_map.units[i].position.y - 1 * animated_map_scale), ImVec2(g_map.units[i].position.x + textsize.x + 1 * animated_map_scale, g_map.units[i].position.y + textsize.y + 1 * animated_map_scale), ImColor(255, 255, 255, 60));

				ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(g_map.units[i].position), ImColor(255, 255, 255), class_name.c_str());


				if (g_map.units[i].owner_country_id == g_menu.players[player_id].control_region)
					g_map.process_unit_selections(&g_map.units[i], animated_map_scale);
			}

		}

		if (old_tick_for_update != g_map.global_tick)
		{
			if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
			{
				g_socket_control.server_send_unit_pos(g_menu.players[player_id].control_region);
			}
			else if (g_socket_control.player_role == g_socket_control.player_role_enum::CLIENT)
			{
				g_socket_control.client_send_unit_pos(g_menu.players[player_id].control_region);
			}
			old_tick_for_update = g_map.global_tick;
		}
	}
};

inline warships_processing g_warships;