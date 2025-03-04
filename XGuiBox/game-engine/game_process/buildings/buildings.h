#pragma once
#include "../../resources/window_profiling/window.h"
#include <vector>
#include "../map_processing.h"
#include "../../tools/tools.h"


class building_processing
{
public:

	void process_buildings(int index, window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
	{
        int i = index;

        for (int buildings_id = 0; buildings_id < countries->at(i).buildings.size(); buildings_id++)
        {
            if (!countries->at(i).buildings.at(buildings_id).name.empty())
            {
                auto pos = ImVec2(countries->at(i).position.x * animated_map_scale - (countries->at(i).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                    countries->at(i).position.y * animated_map_scale - (countries->at(i).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);

                ImVec2 mapped_pos = ImVec2((countries->at(i).buildings[buildings_id].pos.x - pos.x) / animated_map_scale, (countries->at(i).buildings[buildings_id].pos.y - pos.y) / animated_map_scale);

                if (countries->at(i).buildings[buildings_id].size_converted_to_map == false)
                {
                    countries->at(i).buildings[buildings_id].pos = mapped_pos;
                    countries->at(i).buildings[buildings_id].size_converted_to_map = true;
                }

                ImVec2 final_pos = ImVec2(pos.x + countries->at(i).buildings[buildings_id].pos.x * animated_map_scale, pos.y + countries->at(i).buildings[buildings_id].pos.y * animated_map_scale);

                ImGui::GetForegroundDrawList()->AddCircle(final_pos, 5 * animated_map_scale, ImColor(255, 255, 255, 250));

                auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, countries->at(i).buildings[buildings_id].name.c_str());
                ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(final_pos.x - textsize.x / 2, final_pos.y - 8 * animated_map_scale), ImColor(255, 255, 255), countries->at(i).buildings[buildings_id].name.c_str());

                g_map.process_object_selections(false, i, player_id, countries, &countries->at(i).buildings[buildings_id], animated_map_scale, map_pos);

                if ((countries->at(i).buildings[buildings_id].selected != NOT_SELECTED && countries->at(i).buildings[buildings_id].building_type == MISSILE_DEFENSE && countries->at(i).buildings[buildings_id].progress_of_building == 105))
                {
                    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(final_pos), 70.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                }

                if (countries->at(i).buildings[buildings_id].highlighted)
                {
                    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(final_pos), 15.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                }

                if (i != g_menu.players[player_id].control_region)
                    continue;

                //NEXT PART OF CODE SHOULD BE PROCESSED ONLY BY ONE FUNCTION "render_map_and_process. secondary function for double map should ignore this code"

                //first function called
                if (function_count == 1)
                {
                    //building process
                    if (countries->at(i).buildings[buildings_id].progress_of_building != 105)
                    {
                        static int old_game_tick;

                        if (old_game_tick != g_map.global_tick)
                        {
                            countries->at(i).buildings[buildings_id].progress_of_building += 1;
                            old_game_tick = g_map.global_tick;
                        }

                        ImGui::GetForegroundDrawList()->AddRect(ImVec2(final_pos.x - 7 * animated_map_scale, final_pos.y - 15 * animated_map_scale), ImVec2(final_pos.x + 7 * animated_map_scale, final_pos.y - 17 * animated_map_scale), ImColor(255, 255, 255, 250));

                        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(final_pos.x - 7 * animated_map_scale, final_pos.y - 15 * animated_map_scale), ImVec2(final_pos.x - 7 * animated_map_scale + ((countries->at(i).buildings[buildings_id].progress_of_building / 100.f) * 14 * animated_map_scale), final_pos.y - 17 * animated_map_scale), ImColor(255, 255, 255, 250));

                    }

                    if (countries->at(i).buildings[buildings_id].progress_of_building == 100)
                    {
                        countries->at(i).buildings[buildings_id].progress_of_building = 105;

                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                        {
                            g_socket_control.server_send_building(buildings_id, i);
                        }
                        else
                            g_socket_control.client_send_building(buildings_id, i);
                    }

                    //BUILDING FINISHED - the main cycle of building
                    if (countries->at(i).buildings[buildings_id].progress_of_building == 105)
                    {
                        if (countries->at(i).buildings[buildings_id].selected == SOLO_SELECTED)
                        {
                            g_map.opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(350, g_map.screen_y));
                            ImGui::SetNextWindowPos(ImVec2(0, 0));
                            ImGui::SetNextWindowSize(ImVec2(350, g_map.screen_y));
                            ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 210);
                            ImGui::Begin(countries->at(i).buildings[buildings_id].name.c_str());
                            {
                                switch (countries->at(i).buildings[buildings_id].building_type)
                                {
                                    case AIRCRAFT_FACTORY:
                                    {
                                        ImGui::Text(std::string(std::string("Current amount of jets : ") + std::to_string(g_menu.players[player_id].war_property.amount_of_jets)).c_str());

                                        ImGui::SliderInt("Fighter Jets", &countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_jets, 0, 30);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets) / 100.f;
                                            ImGui::SliderScalarForProgress("Building progress [Jets]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                        ImGui::Text(std::string(std::string("Current amount of bombers : ") + std::to_string(g_menu.players[player_id].war_property.amount_of_bombers)).c_str());

                                        ImGui::SliderInt("Bombers", &countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_bombers, 0, 15);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers) / 200.f;
                                            ImGui::SliderScalarForProgress("Building progress [Bombers]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                    }
                                    break;

                                    case SHIPYARD:
                                    {
                                        ImGui::Text(std::string(std::string("Current amount of Submarines : ") + std::to_string(g_menu.players[player_id].war_property.submarine_count)).c_str());

                                        ImGui::SliderInt("Submarines", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat1, 0, 15);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1) / 200.f;
                                            ImGui::SliderScalarForProgress("Building progress [1]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                        ImGui::Text(std::string(std::string("Current amount of Air Carriers : ") + std::to_string(g_menu.players[player_id].war_property.carrier_count)).c_str());

                                        ImGui::SliderInt("Air Carriers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat2, 0, 15);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2) / 200.f;
                                            ImGui::SliderScalarForProgress("Building progress [2]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                        ImGui::Text(std::string(std::string("Current amount of Destroyers : ") + std::to_string(g_menu.players[player_id].war_property.destroyer_count)).c_str());

                                        ImGui::SliderInt("Destroyers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat3, 0, 15);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3) / 200.f;
                                            ImGui::SliderScalarForProgress("Building progress [3]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                        ImGui::Text(std::string(std::string("Current amount of Cruisers : ") + std::to_string(g_menu.players[player_id].war_property.cruiser_count)).c_str());

                                        ImGui::SliderInt("Cruisers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat4, 0, 15);

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4) / 200.f;
                                            ImGui::SliderScalarForProgress("Building progress [4]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::NewLine();

                                        ImGui::NewLine();

                                    }
                                    break;

                                    case MISSILE_SILO:
                                    {

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload) / 350.f;
                                            ImGui::SliderScalarForProgress("Launch progress [1]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
                                        ImGui::PushStyleColor(ImGuiCol_Text, countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot ? ImVec4(ImColor(30, 255, 47)) : ImVec4(ImColor(255, 30, 30)));

                                        ImGui::NewLine();
                                        ImGui::CenteredText(countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot ? "[ READY ]" : "[ RELOADING ]");
                                        ImGui::NewLine();
                                        ImGui::PopStyleColor();

                                        if (ImGui::Button("LAUNCH MISSILE", ImVec2(330, 45)))
                                        {
                                            //if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot)
                                            {
                                                countries->at(i).buildings[buildings_id].selected = NOT_SELECTED;
                                                g_map.selection_for_nuclear_strike = true;
                                                g_map.current_striking_building_id = buildings_id;
                                            }

                                        }

                                        ImGui::NewLine();

                                        auto* vector = &countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue;
                                        {
                                            ImGui::BeginListBox("Queue", ImVec2(330, 120));
                                            {
                                                for (int i = 0; i < vector->size(); i++)
                                                {
                                                    if (vector->at(i).SENDER_building_id == buildings_id)
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

                                                        if (vector->at(i).GETTER_city_id != -1)
                                                        {
                                                            if (ImGui::IsItemHovered())
                                                            {
                                                                countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].highlighted = true;
                                                            }
                                                            else
                                                                countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].highlighted = false;

                                                        }
                                                        else
                                                        {
                                                            if (ImGui::IsItemHovered())
                                                            {
                                                                countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].highlighted = true;
                                                            }
                                                            else
                                                                countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].highlighted = false;
                                                        }

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
                                    break;

                                    case MISSILE_DEFENSE:
                                    {

                                        {
                                            float min = 0; float max = 1;
                                            float progress = (g_map.global_tick - countries->at(i).buildings[buildings_id].missile_defense_heart.old_tick_for_reload) / 50.f;
                                            ImGui::SliderScalarForProgress("Launch progress [1]", ImGuiDataType_Float, &progress, &min, &max);
                                        }

                                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
                                        ImGui::PushStyleColor(ImGuiCol_Text, countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot ? ImVec4(ImColor(30, 255, 47)) : ImVec4(ImColor(255, 30, 30)));

                                        ImGui::NewLine();
                                        ImGui::CenteredText(countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot ? "[ READY ]" : "[ RELOADING ]");
                                        ImGui::NewLine();
                                        ImGui::PopStyleColor();
                                        ImGui::PopStyleVar();
                                    }
                                    break;

                                }
                                ImGui::End();
                            }

                            if (countries->at(i).buildings[buildings_id].building_type == SHIPYARD)
                            {
                                if (ImGui::IsMouseClicked(1))
                                {
                                    units_base new_unit;

                                    new_unit.unique_id = g_tools.generate_unique_int();
                                    new_unit.warship = true;
                                    new_unit.airplane = false;
                                    new_unit.owner_country_id = i;
                                    new_unit.owner_building_id = buildings_id;

                                    g_map.units.push_back(new_unit);
                                }
                            }
                        }

                        switch (countries->at(i).buildings[buildings_id].building_type)
                        {
                            case AIRCRAFT_FACTORY:
                            {
                                if (countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_jets > g_menu.players[player_id].war_property.amount_of_jets)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets + 100)
                                    {
                                        countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets = g_map.global_tick;
                                        g_menu.players[player_id].war_property.amount_of_jets++;
                                    }
                                }

                                if (countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_bombers > g_menu.players[player_id].war_property.amount_of_bombers)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers + 200)
                                    {
                                        countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers = g_map.global_tick;
                                        g_menu.players[player_id].war_property.amount_of_bombers++;
                                    }
                                }

                            }
                            break;

                            case SHIPYARD:
                            {
                                if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat1 > g_menu.players[player_id].war_property.submarine_count)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1 + 200)
                                    {
                                        countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1 = g_map.global_tick;
                                        g_menu.players[player_id].war_property.submarine_count++;
                                    }
                                }

                                if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat2 > g_menu.players[player_id].war_property.carrier_count)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2 + 200)
                                    {
                                        countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2 = g_map.global_tick;
                                        g_menu.players[player_id].war_property.carrier_count++;
                                    }
                                }

                                if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat3 > g_menu.players[player_id].war_property.destroyer_count)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3 + 200)
                                    {
                                        countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3 = g_map.global_tick;
                                        g_menu.players[player_id].war_property.destroyer_count++;
                                    }
                                }

                                if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat4 > g_menu.players[player_id].war_property.cruiser_count)
                                {
                                    if (g_map.global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4 + 200)
                                    {
                                        countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4 = g_map.global_tick;
                                        g_menu.players[player_id].war_property.cruiser_count++;
                                    }
                                }

                            }
                            break;

                            case MISSILE_SILO:
                            {

                                if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot && !countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.empty())
                                {
                                    if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot)
                                    {
                                        g_map.air_strike_targets.push_back(countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue[0]);
                                        countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload = g_map.global_tick;
                                        countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.erase(countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.begin());
                                        countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = false;

                                        if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                                        {
                                            g_socket_control.server_send_nuclear_targets();
                                        }
                                        else
                                            g_socket_control.client_send_nuclear_targets();
                                    }

                                }

                                if (g_map.global_tick > countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload + 350)
                                {
                                    countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = true;
                                }
                                else
                                    countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = false;

                                if (g_map.selection_for_nuclear_strike)
                                {
                                    ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 25, ImVec2(g_map.screen_x / 2, 150), ImColor(255, 255, 255), "Choose Target!");

                                }
                            }
                            break;

                            case MISSILE_DEFENSE:
                            {

                                if (g_map.global_tick > countries->at(i).buildings[buildings_id].missile_defense_heart.old_tick_for_reload + 50)
                                {
                                    countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = true;
                                }
                                else
                                    countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = false;


                                if (countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot)
                                {
                                    for (int rockets_id = 0; rockets_id < g_map.air_strike_targets.size(); rockets_id++)
                                    {
                                        if (g_map.air_strike_targets[rockets_id].GETTER_rocket != -1)
                                            continue;

                                        auto pos2 = ImVec2(countries->at(g_map.air_strike_targets[rockets_id].GETTER_country_id).position.x * animated_map_scale - (countries->at(g_map.air_strike_targets[rockets_id].GETTER_country_id).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                                            countries->at(g_map.air_strike_targets[rockets_id].GETTER_country_id).position.y * animated_map_scale - (countries->at(g_map.air_strike_targets[rockets_id].GETTER_country_id).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);

                                        ImVec2 bombpos = ImVec2(g_map.air_strike_targets[rockets_id].bomb_pos_map1.x, g_map.air_strike_targets[rockets_id].bomb_pos_map1.y);

                                        auto buildpos = ImVec2(final_pos.x, final_pos.y);

                                        float distance = g_tools.calculate_distance(bombpos, buildpos) / animated_map_scale;

                                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(15, 40), ImColor(255, 255, 255), std::to_string(distance).c_str());


                                        if (distance < 70 && g_map.air_strike_targets[rockets_id].SENDER_country_id != g_menu.players[player_id].control_region)
                                        {
                                            if (countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot)
                                            {
                                                nuclear_strike_target new_target;
                                                new_target.unique_id = g_tools.generate_unique_int();
                                                new_target.GETTER_country_id = g_map.air_strike_targets[rockets_id].SENDER_country_id;
                                                new_target.GETTER_city_id = -1;
                                                new_target.GETTER_building_id = -1;
                                                new_target.GETTER_rocket = g_map.air_strike_targets[rockets_id].unique_id;
                                                new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                                new_target.SENDER_building_id = buildings_id;

                                                g_map.air_strike_targets.push_back(new_target);
                                                countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = false;
                                                countries->at(i).buildings[buildings_id].missile_defense_heart.old_tick_for_reload = g_map.global_tick;

                                                if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                                                {
                                                    g_socket_control.server_send_nuclear_targets();
                                                }
                                                else
                                                    g_socket_control.client_send_nuclear_targets();
                                            }
                                        }
                                    }
                                }
                            }
                            break;

                        }
                    }
                }
            }

        }
	}
};
inline building_processing g_building_processing;