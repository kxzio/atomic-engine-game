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
					if (function_count == 1)
						g_socket_control.server_send_unit(g_map.units[i].unique_id);
				}
				else if (g_socket_control.player_role == g_socket_control.player_role_enum::CLIENT)
				{
					if (function_count == 1)
						g_socket_control.client_send_unit(g_map.units[i].unique_id);
				}
			}
			 
			if (function_count == 1)
			{
				g_map.units[i].interpolated_move_offset.x = ImGui::LerpAnimate(std::to_string(g_map.units[i].unique_id).c_str(), "InterpX", true, g_map.units[i].move_offset.x, 100, ImGui::INTERP);
				g_map.units[i].interpolated_move_offset.y = ImGui::LerpAnimate(std::to_string(g_map.units[i].unique_id).c_str(), "InterpY", true, g_map.units[i].move_offset.y, 100, ImGui::INTERP);
			}
			ImVec2 final_pos = ImVec2(pos.x + g_map.units[i].converted_spawn_pos.x * animated_map_scale + g_map.units[i].interpolated_move_offset.x * animated_map_scale, pos.y + g_map.units[i].converted_spawn_pos.y * animated_map_scale + g_map.units[i].interpolated_move_offset.y * animated_map_scale);

			if (function_count == 1)
				g_map.units[i].position		= final_pos;
			else
				g_map.units[i].position_map2 = final_pos;

			if (true)
			{
				//THRERS SOMETHING HAPPENING IN THERE DUE TO FUNCTION COUNTS I FUCKEN HATE THIS CODE BRO :skull_emoji:
				if (ImGui::IsMouseClicked(1) && g_map.units[i].selected != NOT_SELECTED && function_count == 1)
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
			}

			ImVec2 mapped_pos = ImVec2((g_map.units[i].stored_cursor.x - pos.x) / animated_map_scale, (g_map.units[i].stored_cursor.y - pos.y) / animated_map_scale);

			
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

			bool target_is_empty = mapped_path.size() == 1;

			if (!target_is_empty)
				final_pos2 = ImVec2(pos.x + g_map.units[i].target_pos.x * animated_map_scale, pos.y + g_map.units[i].target_pos.y * animated_map_scale);
			else
				final_pos2 = final_pos;

			//colission
			if (function_count == 1)
			{
				if (g_map.units[i].owner_country_id == g_menu.players[player_id].control_region)
				{

					float dx = (final_pos2.x - g_map.units[i].position.x) / animated_map_scale;
					float dy = (final_pos2.y - g_map.units[i].position.y) / animated_map_scale;

					float length = sqrt(dx * dx + dy * dy);

					if (length > 0.0f) {
						float nx = dx / length; // Нормализованный вектор движения
						float ny = dy / length;
						float speed = 0.045f;

						bool blockLeft = false, blockRight = false, blockUp = false, blockDown = false;
						float sensor_offset = 9.0f * animated_map_scale;

						for (int boat2 = 0; boat2 < g_map.units.size(); boat2++)
						{
							if (!g_map.units[i].warship && g_map.units[boat2].warship) continue;
							if (i == boat2) continue;

							// Координаты текущего корабля
							float left1 = g_map.units[i].position.x - (4.5f * animated_map_scale);
							float right1 = g_map.units[i].position.x + (4.5f * animated_map_scale);
							float top1 = g_map.units[i].position.y + (4.5f * animated_map_scale);
							float bottom1 = g_map.units[i].position.y - (4.5f * animated_map_scale);

							// Координаты другого корабля
							float left2 = g_map.units[boat2].position.x - (4.5f * animated_map_scale);
							float right2 = g_map.units[boat2].position.x + (4.5f * animated_map_scale);
							float top2 = g_map.units[boat2].position.y + (4.5f * animated_map_scale);
							float bottom2 = g_map.units[boat2].position.y - (4.5f * animated_map_scale);

							float dist_x = g_map.units[boat2].position.x - g_map.units[i].position.x;
							float dist_y = g_map.units[boat2].position.y - g_map.units[i].position.y;

							// Пропуск, если другой корабль слишком далеко
							float max_range = 15.0f * animated_map_scale;
							if (fabs(dist_x) > max_range || fabs(dist_y) > max_range) continue;

							// Проверка на пересечение (столкновение)
							bool collision = (right1 > left2 && left1 < right2 && top1 > bottom2 && bottom1 < top2);

							// Отдельная проверка блокировки по каждой оси
							if (collision) {
								if (g_map.units[i].position.x < g_map.units[boat2].position.x) blockRight = true; // Блокировка справа
								if (g_map.units[i].position.x > g_map.units[boat2].position.x) blockLeft = true;  // Блокировка слева
								if (g_map.units[i].position.y < g_map.units[boat2].position.y) blockDown = true;  // Блокировка снизу
								if (g_map.units[i].position.y > g_map.units[boat2].position.y) blockUp = true;    // Блокировка сверху
							}
						}

						// Движение, если нет блокировки в этом направлении
						if (g_map.units[i].old_tick != g_map.global_tick) {
							if ((nx > 0 && !blockRight) || (nx < 0 && !blockLeft)) {
								g_map.units[i].move_offset.x += nx * speed; // Двигаемся по X
							}

							if ((ny > 0 && !blockDown) || (ny < 0 && !blockUp)) {
								g_map.units[i].move_offset.y += ny * speed; // Двигаемся по Y
							}

							g_map.units[i].old_tick = g_map.global_tick; // Обновляем тик
						}
					}



					if (length < 2.0f && !g_map.units[i].path.empty())
						g_map.units[i].path.erase(g_map.units[i].path.begin());

					if (g_map.global_tick % 5 == 0)
					{
						g_map.units[i].old_position = g_map.units[i].move_offset;
					}

				}
			}

			if (function_count == 1)
			{
				//attack
				for (int boat2 = 0; boat2 < g_map.units.size(); boat2++)
				{
					if (g_socket_control.player_role != g_socket_control.player_role_enum::SERVER)
						continue;

					if (i == boat2)
						continue;

					if (g_map.units[boat2].owner_country_id == g_map.units[i].owner_country_id)
						continue;

					float dist_x = g_map.units[boat2].position.x - g_map.units[i].position.x;
					float dist_y = g_map.units[boat2].position.y - g_map.units[i].position.y;

					float distance_between_boats = g_tools.calculate_distance(g_map.units[i].position, g_map.units[boat2].position) / animated_map_scale;

					if (distance_between_boats < 60 && g_map.units[i].reload_for_turrets_tick + 160 < g_map.global_tick)
					{
						g_map.units[i].reload_for_turrets_tick = g_map.global_tick;

						int dmg = 0;
						switch (g_map.units[i].class_of_unit)
						{
						case boats::AIR_CARRIERS: {
							continue;
						}
												break;

						case boats::SUBMARINES: {
							continue;
						}
											  break;

						case boats::CRUISERS: {
							dmg = 25;
						}
											break;

						case boats::DESTROYER: {
							dmg = 15;
						}
											 break;
						}

						//start firing
						nuclear_strike_target new_target;
						new_target.unique_id = g_tools.generate_unique_int();

						new_target.GETTER_country_id = -1;
						new_target.GETTER_city_id = -1;
						new_target.GETTER_building_id = -1;
						new_target.GETTER_rocket = -1;
						new_target.SENDER_country_id = -1;
						new_target.SENDER_building_id = -1;
						new_target.damage = dmg;
						new_target.SENDER_unit = g_map.units[i].unique_id;
						new_target.GETTER_unit = g_map.units[boat2].unique_id;

						g_map.air_strike_targets.push_back(new_target);
					}

				}
			}

			if (function_count == 1)
			{
				//nuclear strike
				{
					if (g_map.units[i].ready_to_nuclear && !g_map.units[i].strike_queue.empty())
					{
						if (g_map.units[i].ready_to_nuclear)
						{
							g_map.air_strike_targets.push_back(g_map.units[i].strike_queue[0]);
							g_map.units[i].reload_for_nuclear_tick = g_map.global_tick;
							g_map.units[i].strike_queue.erase(g_map.units[i].strike_queue.begin());
							g_map.units[i].ready_to_nuclear = false;

							if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
							{
								g_socket_control.server_send_nuclear_targets();
							}
							else
								g_socket_control.client_send_nuclear_targets();
						}

					}

					if (g_map.global_tick > g_map.units[i].reload_for_nuclear_tick + 650)
					{
						g_map.units[i].ready_to_nuclear = true;
					}
					else
						g_map.units[i].ready_to_nuclear = false;

					if (g_map.selection_for_nuclear_strike)
					{
						ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 25, ImVec2(g_map.screen_x / 2, 150), ImColor(255, 255, 255), "Choose Target!");

					}
				}
			}

			std::string class_name = "";
			
			switch (g_map.units[i].class_of_unit)
			{
				case boats::SUBMARINES:		class_name = "Submarine";	break;
				case boats::AIR_CARRIERS:	class_name = "Air Carrier"; break;
				case boats::DESTROYER:		class_name = "Destroyer";	break;
				case boats::CRUISERS:		class_name = "Cruiser";		break;
			}

			ImVec2 textsize = ImVec2(0, 0);

			ImVec2 drawing_pos;

			if (function_count == 1)
				drawing_pos = g_map.units[i].position;
			else
				drawing_pos = g_map.units[i].position_map2;

			if (!class_name.empty())
			{
				ImGui::GetForegroundDrawList()->AddCircleFilled(drawing_pos, 2 * animated_map_scale, g_menu.players[player_id].control_region == g_map.units[i].owner_country_id ? ImColor(150, 200, 180) : ImColor(255, 40, 10), 15);

				textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17.f, FLT_MAX, -1.f, class_name.c_str());

				ImGui::GetForegroundDrawList()->AddPolyline(mapped_path.data(), mapped_path.size(), g_map.units[i].selected ? ImColor(255, 255, 255) : ImColor(255, 255, 255, 100), 0, 1.f);

				ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(drawing_pos.x - 1 * animated_map_scale, drawing_pos.y - 1 * animated_map_scale), ImVec2(drawing_pos.x + textsize.x + 1 * animated_map_scale, drawing_pos.y + textsize.y + 1 * animated_map_scale), ImColor(0, 0, 0, 200));
				ImGui::GetForegroundDrawList()->AddRect(ImVec2(drawing_pos.x - 1 * animated_map_scale, drawing_pos.y - 1 * animated_map_scale), ImVec2(drawing_pos.x + textsize.x + 1 * animated_map_scale, drawing_pos.y + textsize.y + 1 * animated_map_scale), ImColor(255, 255, 255, 60));

				//healthbar
				ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(drawing_pos.x - 5 * animated_map_scale, drawing_pos.y + 5 * animated_map_scale), ImVec2(drawing_pos.x + (((g_map.units[i].health / 100.f) * 10.f)) * animated_map_scale, drawing_pos.y + 6 * animated_map_scale), ImColor(0, 255, 0, 160));
				ImGui::GetForegroundDrawList()->AddRect(ImVec2(drawing_pos.x - 5 * animated_map_scale, drawing_pos.y + 5 * animated_map_scale), ImVec2(drawing_pos.x + 10 * animated_map_scale, drawing_pos.y + 6 * animated_map_scale), ImColor(255, 255, 255, 160));

				ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(drawing_pos), ImColor(255, 255, 255), class_name.c_str());


				if (g_map.units[i].owner_country_id == g_menu.players[player_id].control_region)
				{
					if (function_count == 1)
						g_map.process_unit_selections(&g_map.units[i], g_map.units[i].position, animated_map_scale);
					else
						g_map.process_unit_selections(&g_map.units[i], g_map.units[i].position_map2, animated_map_scale);
				}
			}

			if (function_count == 1)
			{
				if (g_map.units[i].selected == SOLO_SELECTED && g_map.units[i].class_of_unit == boats::SUBMARINES)
				{
					g_map.opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(350, g_map.screen_y));
					ImGui::SetNextWindowPos(ImVec2(0, 0));
					ImGui::SetNextWindowSize(ImVec2(350, g_map.screen_y));
					ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 210);
					ImGui::Begin("Submarine");
					{
						{
							float min = 0; float max = 1;
							float progress = (g_map.global_tick - g_map.units[i].reload_for_nuclear_tick) / 650.f;
							ImGui::SliderScalarForProgress("Launch progress [1]", ImGuiDataType_Float, &progress, &min, &max);
						}

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
						ImGui::PushStyleColor(ImGuiCol_Text, g_map.units[i].ready_to_nuclear ? ImVec4(ImColor(30, 255, 47)) : ImVec4(ImColor(255, 30, 30)));

						ImGui::NewLine();
						ImGui::CenteredText(g_map.units[i].ready_to_nuclear ? "[ READY ]" : "[ RELOADING ]");
						ImGui::NewLine();
						ImGui::PopStyleColor();

						if (ImGui::Button("LAUNCH MISSILE", ImVec2(330, 45)))
						{
							//if (g_map.units[i].ready_to_nuclear)
							{
								g_map.units[i].selected = NOT_SELECTED;
								g_map.selection_for_nuclear_strike = true;
								g_map.current_striking_boat_id = g_map.units[i].unique_id;
							}

						}

						ImGui::NewLine();

						auto* vector = &g_map.units[i].strike_queue;
						{
							ImGui::BeginListBox("Queue", ImVec2(330, 120));
							{
								for (int i2 = 0; i2 < vector->size(); i2++)
								{
									if (vector->at(i).SENDER_unit == g_map.units[i].unique_id)
									{
										std::stringstream ss;

										if (vector->at(i).GETTER_city_id != -1)
											ss << i + 1 << ". " << countries->at(vector->at(i).GETTER_country_id).name << " - " << countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].name;
										else
											ss << i + 1 << ". " << countries->at(vector->at(i).GETTER_country_id).name << " - " << countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].name;

										if (ImGui::BeginDragDropTarget())
										{
											if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
											{
												int payload_idx = *(const int*)payload->Data;
												if (payload_idx != i)
												{
													// Перемещаем элемент в списке
													auto temp = vector->at(payload_idx);
													vector->erase(vector->begin() + payload_idx);
													vector->insert(vector->begin() + i, temp);
												}
											}
											ImGui::EndDragDropTarget();
										}

										ImGui::PushStyleColor(ImGuiCol_Text, i == 0 ? ImVec4(ImColor(79, 255, 69)) : ImVec4(ImColor(255, 255, 255, 110)));

										ImGui::Selectable(ss.str().c_str());

										ImGui::PopStyleColor();
										if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
											ImGui::SetDragDropPayload("DND_ITEM", &i, sizeof(int));
											ImGui::Text("%s", ss.str().c_str());
											ImGui::EndDragDropSource();
										}


										if (ImGui::BeginDragDropTarget())
										{
											if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
											{
												int payload_idx = *(const int*)payload->Data;
												if (payload_idx != i)
												{
													// Перемещаем элемент в списке
													auto temp = vector->at(payload_idx);
													vector->erase(vector->begin() + payload_idx);
													vector->insert(vector->begin() + i, temp);
												}
											}
											ImGui::EndDragDropTarget();
										}
									}
								}
							}
							ImGui::EndListBox();

							ImGui::Button("Delete target", ImVec2(330, 25));
							if (ImGui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
								{
									int payload_idx = *(const int*)payload->Data;
									{
										vector->erase(vector->begin() + payload_idx);
									}
								}
								ImGui::EndDragDropTarget();
							}
						}

						ImGui::PopStyleVar();
					}
					ImGui::End();
				}

				if (g_map.units[i].health < 1)
					g_map.units.erase(g_map.units.begin() + i);
			}

		}




		std::vector<Circle> circles;

		for (int i = 0; i < g_map.units.size(); i++) 
		{

			if (!g_map.units[i].warship)
				continue;

			if (!g_map.units[i].selected)
				continue;

			Circle c;
			c.center = g_map.units[i].position;
			c.radius = 30 * animated_map_scale;
			circles.push_back(c);

			Circle c2;
			c2.center = g_map.units[i].position_map2;
			c2.radius = 30 * animated_map_scale;
			circles.push_back(c2);
		}

		// Для каждого круга отрисовываем его внешнюю границу (без внутренних перекрытий)
		for (const auto& circle : circles) {
			g_tools.DrawCircleVisibleArcs(circle, circles, IM_COL32(255, 0, 0, 255), 2.0f);
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