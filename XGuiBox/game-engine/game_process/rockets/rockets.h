#pragma once
#include "../../resources/window_profiling/window.h"
#include <vector>
#include "../map_processing.h"
#include "../../tools/tools.h"

class rocket_processing
{
public:

	void process_rockets(int index, window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
	{
        bool is_going_to_update_targets = false;
        if (!g_map.air_strike_targets.empty())
        {
            //BIG TODO : REMAKE ALL IDs TO UNIQUE ID INSTEAD OF Static. because building can be destroyed for example.
            for (int targets = 0; targets < g_map.air_strike_targets.size(); targets++)
            {
                //ROCKET TO CITY
                if (g_map.air_strike_targets[targets].SENDER_unit == -1 && g_map.air_strike_targets[targets].GETTER_city_id != -1)
                {
                    if (true)
                    {
                        auto start_pos = countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos;

                        auto pos = ImVec2(countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                            countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);

                        ImVec2 final_pos = ImVec2(pos.x + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                        //targets
                        auto posx = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale;
                        auto posy = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale;

                        auto target = ImVec2(posx + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale);

                        //bomb sending and pos

                        if (function_count == 1)
                            g_map.air_strike_targets[targets].segments = g_tools.calculate_segments(final_pos, target, animated_map_scale);

                        auto point = g_tools.get_trajectory_one_point(final_pos, target, g_map.air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, g_map.air_strike_targets[targets].segments);

                        if (function_count == 1) g_map.air_strike_targets[targets].bomb_pos_map1 = point; else g_map.air_strike_targets[targets].bomb_pos_map2 = point;

                        //DRAWING
                        if (function_count == 1)
                        {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), final_pos, ImVec2(posx + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale),
                                g_map.air_strike_targets[targets].bomb_pos_map1, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map1, 3 * animated_map_scale, ImColor(255, 0, 0));
                        }
                        else {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), final_pos, ImVec2(posx + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale),
                                g_map.air_strike_targets[targets].bomb_pos_map2, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map2, 3 * animated_map_scale, ImColor(255, 0, 0));

                        }


                        //UPDATING
                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                        {
                            if (g_map.air_strike_targets[targets].step_of_bomb >= g_map.air_strike_targets[targets].segments) // segments
                            {
                                auto bomb_ptr = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                                    {
                                        return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                    });
                                g_map.air_strike_targets.erase(bomb_ptr);

                                is_going_to_update_targets = true;
                                continue;
                            }

                            if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick)
                            {
                                g_map.air_strike_targets[targets].step_of_bomb++;
                                g_map.air_strike_targets[targets].last_global_tick = g_map.global_tick;
                                is_going_to_update_targets = true;
                            }

                        }

                    }
                }

                //ROCKET TO BUILDING
                else if (g_map.air_strike_targets[targets].SENDER_unit == -1 && g_map.air_strike_targets[targets].GETTER_building_id != -1)
                {
                    if (true)
                    {
                        auto start_pos = countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos;

                        auto pos = ImVec2(countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                            countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);

                        ImVec2 final_pos = ImVec2(pos.x + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                        //bomb sending and pos

                        auto posx = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale;
                        auto posy = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale;

                        auto pos_of_building = ImVec2(posx + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).buildings[g_map.air_strike_targets[targets].GETTER_building_id].pos.x * animated_map_scale, posy + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).buildings[g_map.air_strike_targets[targets].GETTER_building_id].pos.y * animated_map_scale);

                        if (function_count == 1)
                            g_map.air_strike_targets[targets].segments = g_tools.calculate_segments(final_pos, pos_of_building, animated_map_scale);

                        auto point = g_tools.get_trajectory_one_point(final_pos, pos_of_building, g_map.air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, g_map.air_strike_targets[targets].segments);

                        if (function_count == 1) g_map.air_strike_targets[targets].bomb_pos_map1 = point; else g_map.air_strike_targets[targets].bomb_pos_map2 = point;

                        //DRAWING
                        if (function_count == 1)
                        {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), final_pos, pos_of_building, 
                                g_map.air_strike_targets[targets].bomb_pos_map1, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map1, 3 * animated_map_scale, ImColor(255, 0, 0));
                        }
                        else {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), final_pos, pos_of_building,
                                g_map.air_strike_targets[targets].bomb_pos_map2, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map2, 3 * animated_map_scale, ImColor(255, 0, 0));

                        }

                        //UPDATING
                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                        {
                            if (g_map.air_strike_targets[targets].step_of_bomb >= g_map.air_strike_targets[targets].segments) // segments
                            {
                                auto bomb_ptr = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                                    {
                                        return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                    });
                                g_map.air_strike_targets.erase(bomb_ptr);

                                is_going_to_update_targets = true;
                                continue;
                            }

                            if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick)
                            {
                                g_map.air_strike_targets[targets].step_of_bomb++;
                                is_going_to_update_targets = true;

                                if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick + 3)
                                    g_map.air_strike_targets[targets].last_global_tick = g_map.global_tick;

                            }

                        }


                    }
                }

                //ROCKET TO ROCKET
                else if (g_map.air_strike_targets[targets].GETTER_rocket != -1)
                {
                    auto pointed_bomb_ptr2 = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](const nuclear_strike_target& target)
                        {
                            return target.unique_id == g_map.air_strike_targets[targets].GETTER_rocket;
                        });

                    if (pointed_bomb_ptr2->GETTER_rocket == -1)
                    {
                        auto start_pos = countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos;

                        auto pos = ImVec2(countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                            countries->at(g_map.air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);

                        ImVec2 final_pos = ImVec2(pos.x + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(g_map.air_strike_targets[targets].SENDER_country_id).buildings[g_map.air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                        auto pointed_bomb_ptr3 = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                            {
                                return target.unique_id == g_map.air_strike_targets[targets].GETTER_rocket;
                            });


                        ImVec2 bomb_pos = ImVec2(0, 0);
                        if (function_count == 1)
                            bomb_pos = pointed_bomb_ptr3->bomb_pos_map1;
                        else
                            bomb_pos = pointed_bomb_ptr3->bomb_pos_map2;

                        if (function_count == 1)
                            g_map.air_strike_targets[targets].segments = g_tools.calculate_segments(final_pos, bomb_pos, animated_map_scale);

                        auto target_point = g_tools.get_trajectory_stright_one_point(final_pos, bomb_pos, g_map.air_strike_targets[targets].step_of_bomb, g_map.air_strike_targets[targets].segments);

                        if (function_count == 1) g_map.air_strike_targets[targets].bomb_pos_map1 = target_point; else g_map.air_strike_targets[targets].bomb_pos_map2 = target_point;

                        //DRAWING
                        if (function_count == 1)
                        {
                            g_tools.draw_trajectory_stright_arc(ImGui::GetForegroundDrawList(), final_pos, bomb_pos,
                                g_map.air_strike_targets[targets].bomb_pos_map1, ImColor(255, 255, 40, 150), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map1, 0.5 * animated_map_scale, ImColor(255, 190, 0));
                        }
                        else {
                            g_tools.draw_trajectory_stright_arc(ImGui::GetForegroundDrawList(), final_pos, bomb_pos,
                                g_map.air_strike_targets[targets].bomb_pos_map2, ImColor(255, 255, 40, 150), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map2, 0.5 * animated_map_scale, ImColor(255, 190, 0));

                        }

                        //UPDATING
                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                        {
                            if (g_map.air_strike_targets[targets].step_of_bomb >= g_map.air_strike_targets[targets].segments) // segments
                            {

                                if (true)
                                {
                                    std::srand(static_cast<unsigned int>(std::time(0)));
                                    int random_number = std::rand() % 11;

                                    if (random_number % 4 == 0)
                                    {

                                        auto pointed_bomb_ptr = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                                            {
                                                return target.unique_id == g_map.air_strike_targets[targets].GETTER_rocket;
                                            });
                                        g_map.air_strike_targets.erase(pointed_bomb_ptr);


                                        auto bomb_ptr = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                                            {
                                                return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                            });
                                        g_map.air_strike_targets.erase(bomb_ptr);

                                        //ROCKET DESTROYED
                                        is_going_to_update_targets = true;
                                        continue;
                                    }
                                    else
                                    {
                                        //ROCKET DESTROYED

                                        auto bomb_ptr = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](nuclear_strike_target target)
                                            {
                                                return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                            });
                                        g_map.air_strike_targets.erase(bomb_ptr);

                                        is_going_to_update_targets = true;
                                        continue;
                                    }
                                }
                            }

                            if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick)
                            {
                                g_map.air_strike_targets[targets].step_of_bomb++;
                                g_map.air_strike_targets[targets].last_global_tick = (g_map.global_tick - g_map.air_strike_targets[targets].last_global_tick) / 2;
                                is_going_to_update_targets = true;
                            }

                        }


                    }
                }

                //ROCKET TO FROM UNIT TO UNIT 
                else if (g_map.air_strike_targets[targets].SENDER_unit != -1 && g_map.air_strike_targets[targets].GETTER_unit != -1)
                {
                    auto sender_unit = std::find_if(g_map.units.begin(), g_map.units.end(), [&](const units_base& target)
                        {
                            return target.unique_id == g_map.air_strike_targets[targets].SENDER_unit;
                        });

                    auto getter_unit = std::find_if(g_map.units.begin(), g_map.units.end(), [&](const units_base& target)
                        {
                            return target.unique_id == g_map.air_strike_targets[targets].GETTER_unit;
                        });

                    if (sender_unit == g_map.units.end())
                        continue;

                    if (getter_unit == g_map.units.end())
                        continue;

                    if (function_count == 1)
                        g_map.air_strike_targets[targets].segments = g_tools.calculate_segments(sender_unit->position, getter_unit->position, animated_map_scale);

                    auto rocket_pos = g_tools.get_trajectory_stright_one_point(sender_unit->position, getter_unit->position, g_map.air_strike_targets[targets].step_of_bomb, g_map.air_strike_targets[targets].segments);

                    g_map.air_strike_targets[targets].bomb_pos_map1 = rocket_pos;

                    g_tools.draw_trajectory_stright_arc(ImGui::GetForegroundDrawList(), sender_unit->position, getter_unit->position,
                        g_map.air_strike_targets[targets].bomb_pos_map1, ImColor(255, 255, 40, 150), 200);

                    ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map1, 0.5 * animated_map_scale, ImColor(255, 190, 0));

                    //UPDATING
                    if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                    {
                        if (g_map.air_strike_targets[targets].step_of_bomb >= g_map.air_strike_targets[targets].segments) // segments
                        {

                            if (true)
                            {
                                std::srand(static_cast<unsigned int>(std::time(0)));
                                int random_number = std::rand() % 11;

                                if (random_number % 2 == 0)
                                {
                                    getter_unit->health -= g_map.air_strike_targets[targets].damage;

                                    g_socket_control.server_send_unit_health(getter_unit->unique_id);

                                    g_map.air_strike_targets.erase(g_map.air_strike_targets.begin() + targets);

                                    //HIT
                                    is_going_to_update_targets = true;
                                    continue;
                                }
                                else
                                {
                                    auto sender_unit_rocket = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](const nuclear_strike_target& target)
                                        {
                                            return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                        });

                                    g_map.air_strike_targets.erase(sender_unit_rocket);

                                    //NO HIT
                                    is_going_to_update_targets = true;
                                    continue;
                                }
                            }
                        }

                        if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick)
                        {
                            g_map.air_strike_targets[targets].step_of_bomb++;
                            g_map.air_strike_targets[targets].last_global_tick = (g_map.global_tick - g_map.air_strike_targets[targets].last_global_tick) / 2;
                            is_going_to_update_targets = true;
                        }

                    }

                }

                //ROCKET FROM SUBMARINE TO CITY
                else if (g_map.air_strike_targets[targets].SENDER_unit != -1 && g_map.air_strike_targets[targets].GETTER_city_id != -1)
                {
                    if (true)
                    {
                        auto sender_unit = std::find_if(g_map.units.begin(), g_map.units.end(), [&](const units_base& target)
                            {
                                return target.unique_id == g_map.air_strike_targets[targets].SENDER_unit;
                            });


                        if (sender_unit == g_map.units.end())
                            continue;


                        //targets
                        auto posx = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale;
                        auto posy = countries->at(g_map.air_strike_targets[targets].GETTER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[targets].GETTER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale;

                        auto target = ImVec2(posx + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(g_map.air_strike_targets[targets].GETTER_country_id).cities[g_map.air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale);

                        if (function_count == 1 && !g_map.air_strike_targets[targets].segments)
                            g_map.air_strike_targets[targets].segments = g_tools.calculate_segments(sender_unit->position, target, animated_map_scale);

                        ImVec2 rocket_pos = g_tools.get_trajectory_one_point(sender_unit->position, target, g_map.air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, g_map.air_strike_targets[targets].segments);
                        ImVec2 rocket_pos_map2 = g_tools.get_trajectory_one_point(sender_unit->position_map2, target, g_map.air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, g_map.air_strike_targets[targets].segments);

                        if (function_count == 1) g_map.air_strike_targets[targets].bomb_pos_map1 = rocket_pos; else g_map.air_strike_targets[targets].bomb_pos_map2 = rocket_pos_map2;

                        ImGui::GetForegroundDrawList()->AddText(ImVec2(100, 100), ImColor(255, 255, 255), std::to_string(g_map.air_strike_targets[targets].segments).c_str());

                        //DRAWING
                        if (function_count == 1)
                        {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), sender_unit->position, target,
                                g_map.air_strike_targets[targets].bomb_pos_map1, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map1, 3 * animated_map_scale, ImColor(255, 0, 0));
                        }
                        else {
                            g_tools.draw_trajectory_arc(ImGui::GetForegroundDrawList(), sender_unit->position_map2, target,
                                g_map.air_strike_targets[targets].bomb_pos_map2, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);
                            ImGui::GetForegroundDrawList()->AddCircleFilled(g_map.air_strike_targets[targets].bomb_pos_map2, 3 * animated_map_scale, ImColor(255, 0, 0));

                        }


                        //UPDATING
                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                        {
                            if (g_map.air_strike_targets[targets].step_of_bomb >= g_map.air_strike_targets[targets].segments) // segments
                            {

                                auto sender_unit_rocket = std::find_if(g_map.air_strike_targets.begin(), g_map.air_strike_targets.end(), [&](const nuclear_strike_target& target)
                                    {
                                        return target.unique_id == g_map.air_strike_targets[targets].unique_id;
                                    });

                                g_map.air_strike_targets.erase(sender_unit_rocket);
                                continue;
                            }

                            if (g_map.air_strike_targets[targets].last_global_tick < g_map.global_tick)
                            {
                                g_map.air_strike_targets[targets].step_of_bomb++;
                                g_map.air_strike_targets[targets].last_global_tick = g_map.global_tick;
                                is_going_to_update_targets = true;
                            }

                        }
                    }
                }
            }
        }

        if (is_going_to_update_targets && g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
        {
            g_socket_control.server_send_nuclear_targets();
        }
	}
};
inline rocket_processing g_rocket_processing;