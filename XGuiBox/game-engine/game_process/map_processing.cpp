#include "map_processing.h"
#include <sstream>
#include <chrono>
#include <random>

#include "buildings/buildings.h"
#include "buildings/../country-cities/city.h"
#include "rockets/rockets.h"
#include "map_helper.h"
#include "units/warships.h"
#include <boost/functional/hash.hpp>

void map_processing::process_object_selections(int function_count, bool city, int current_country, int player_id, std::vector <country_data>* countries, map_objects* object, float animated_map_scale, ImVec2 map_pos)
{
    auto data = countries->at(current_country);

    auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
    auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;

    ImVec2 point_pos = ImVec2(posx + object->pos.x * animated_map_scale - 1.5 * animated_map_scale, posy + object->pos.y * animated_map_scale - 1.5 * animated_map_scale);
    ImVec2 point_size = ImVec2(posx + object->pos.x * animated_map_scale + 1.5 * animated_map_scale, posy + object->pos.y * animated_map_scale + 1.5 * animated_map_scale);

    bool single_select = false;

    if (g_map.cursor_pos.x > g_map.opened_menu_size.Min.x && g_map.cursor_pos.x < g_map.opened_menu_size.Max.x)
    {
        if (g_map.cursor_pos.y > g_map.opened_menu_size.Min.y && g_map.cursor_pos.y < g_map.opened_menu_size.Max.y)
        {
            return;
        }
    }

    if (current_country == g_menu.players[player_id].control_region || g_map.selection_for_nuclear_strike)
    {
        if (g_map.selector_zone.GetSize().x == 0)
        {
            if (g_tools.is_point_in_rect(point_pos, ImRect(ImVec2(g_map.cursor_pos.x - 5 * animated_map_scale, g_map.cursor_pos.y - 5 * animated_map_scale), ImVec2(g_map.cursor_pos.x + 5 * animated_map_scale, g_map.cursor_pos.y + 5 * animated_map_scale))))
            {
                object->hovered = true;
                ImGui::GetForegroundDrawList()->AddRect(ImVec2(point_pos.x - 3 * animated_map_scale, point_pos.y - 3 * animated_map_scale), ImVec2(point_size.x + 3 * animated_map_scale, point_size.y + 3 * animated_map_scale), g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));

                if (ImGui::IsMouseReleased(0))
                {
                    single_select = true;
                    object->selected = SOLO_SELECTED;
                    g_map.last_iteration_was_with_function_number = function_count;
                    ImGui::GetForegroundDrawList()->AddRect(point_pos, point_size, g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));

                    //nuclear target selection
                    if (current_country != g_menu.players[player_id].control_region)
                    {
                        if (g_map.selection_for_nuclear_strike)
                        {
                            int id = 0;
                            if (city)
                            {
                                for (int cityi = 0; cityi < countries->at(current_country).cities.size(); cityi++)
                                {
                                    if (countries->at(current_country).cities.at(cityi).selected == SOLO_SELECTED)
                                    {
                                        id = cityi;

                                        nuclear_strike_target new_target;
                                        new_target.unique_id = g_tools.generate_unique_int();
                                        new_target.GETTER_country_id  = current_country;
                                        new_target.GETTER_city_id     = id;
                                        new_target.GETTER_building_id = -1;
                                        new_target.GETTER_rocket      = -1;
                                        if (g_map.current_striking_boat_id == -1)
                                        {
                                            new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                            new_target.SENDER_building_id = g_map.current_striking_building_id;
                                        }
                                        else
                                        {
                                            new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                            new_target.SENDER_building_id = -1;

                                            new_target.SENDER_unit = g_map.current_striking_boat_id;
                                        }

                                        if (g_map.current_striking_boat_id == -1)
                                        {
                                            countries->at(g_menu.players[player_id].control_region).buildings[g_map.current_striking_building_id].missile_silo_heart.strike_queue.push_back(new_target);
                                        }
                                        else
                                        {
                                            auto sender_unit = std::find_if(g_map.units.begin(), g_map.units.end(), [&](const units_base& target)
                                                {
                                                    return target.unique_id == g_map.current_striking_boat_id;
                                                });


                                            if (sender_unit == g_map.units.end())
                                                continue;

                                            sender_unit->strike_queue.push_back(new_target);
                                        }
                                        g_map.selection_for_nuclear_strike = false;

                                    }
                                }
                            }
                            else
                            {
                                for (int build = 0; build < countries->at(current_country).buildings.size(); build++)
                                {
                                    if (countries->at(current_country).buildings.at(build).selected == SOLO_SELECTED)
                                    {
                                        id = build;

                                        nuclear_strike_target new_target;
                                        new_target.unique_id = g_tools.generate_unique_int();
                                        new_target.GETTER_country_id = current_country;
                                        new_target.GETTER_city_id = -1;
                                        new_target.GETTER_building_id = id;
                                        new_target.GETTER_rocket = -1;
                                        if (g_map.current_striking_boat_id == -1)
                                        {
                                            new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                            new_target.SENDER_building_id = g_map.current_striking_building_id;
                                        }
                                        else
                                        {
                                            new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                            new_target.SENDER_building_id = -1;

                                            new_target.SENDER_unit = g_map.current_striking_boat_id;
                                        }
                                        if (g_map.current_striking_boat_id == -1)
                                        {
                                            countries->at(g_menu.players[player_id].control_region).buildings[g_map.current_striking_building_id].missile_silo_heart.strike_queue.push_back(new_target);
                                        }
                                        else
                                        {
                                            auto sender_unit = std::find_if(g_map.units.begin(), g_map.units.end(), [&](const units_base& target)
                                                {
                                                    return target.unique_id == g_map.current_striking_boat_id;
                                                });


                                            if (sender_unit == g_map.units.end())
                                                continue;

                                            sender_unit->strike_queue.push_back(new_target);
                                        }
                                        g_map.selection_for_nuclear_strike = false;

                                    }
                                }
                            }
                        }
                    }
                }

            }
            else object->hovered = false;
        }
        else
        {
            if (g_tools.is_point_in_rect(point_pos, g_map.selector_zone))
            {
                object->hovered = true;
                ImGui::GetForegroundDrawList()->AddRect(ImVec2(point_pos.x - 3 * animated_map_scale, point_pos.y - 3 * animated_map_scale), ImVec2(point_size.x + 3 * animated_map_scale, point_size.y + 3 * animated_map_scale), g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));
            }
            else if (ImGui::IsMouseDown(0))
            {
                object->hovered = false;
            }
            else if (object->hovered || object->selected == MULTIPLE_SELECTED)
            {
                object->hovered = false;
                object->selected = MULTIPLE_SELECTED;
                g_map.last_iteration_was_with_function_number = function_count;
            }
        }
    }
    if (ImGui::IsMouseClicked(0) && !single_select)
    {
        object->hovered = false;
        object->selected = NOT_SELECTED;
    }


    if (ImGui::IsMouseClicked(0))
    {
        g_map.opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(0, 0));
    }

    if (object->selected)
    {
        ImGui::GetForegroundDrawList()->AddRect(point_pos, point_size, g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));
    }
}

void map_processing::process_unit_selections(units_base* unit, ImVec2 pos, float animated_map_scale)
{

    ImVec2 unit_pos;

    bool single_select = false;

    if (g_map.cursor_pos.x > g_map.opened_menu_size.Min.x && g_map.cursor_pos.x < g_map.opened_menu_size.Max.x)
    {
        if (g_map.cursor_pos.y > g_map.opened_menu_size.Min.y && g_map.cursor_pos.y < g_map.opened_menu_size.Max.y)
        {
            return;
        }
    }

    if (g_map.selector_zone.GetSize().x == 0)
    {
        if (g_tools.is_point_in_rect(pos, ImRect(ImVec2(g_map.cursor_pos.x - 5 * animated_map_scale, g_map.cursor_pos.y - 5 * animated_map_scale), ImVec2(g_map.cursor_pos.x + 5 * animated_map_scale, g_map.cursor_pos.y + 5 * animated_map_scale))))
        {
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(pos.x - 5 * animated_map_scale, pos.y - 5 * animated_map_scale), ImVec2(pos.x + 5 * animated_map_scale, pos.y + 5 * animated_map_scale), ImColor(255, 255, 255));

            if (ImGui::IsMouseReleased(0))
            {
                single_select = true;
                unit->selected = SOLO_SELECTED;
            }
        }
        else unit->hovered = false;
    }
    else
    {
        if (g_tools.is_point_in_rect(pos, g_map.selector_zone))
        {
            unit->hovered = true;
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(pos.x - 5 * animated_map_scale, pos.y - 5 * animated_map_scale), ImVec2(pos.x + 5 * animated_map_scale, pos.y + 5 * animated_map_scale), ImColor(255, 255, 255));
        }
        else
            unit->hovered = false;
    }

    if ((!ImGui::IsMouseDown(0)) && (unit->hovered || unit->selected == MULTIPLE_SELECTED))
    {
        if (!single_select)
        {
            unit->hovered = false;
            unit->selected = MULTIPLE_SELECTED;
        }
    }

    if (ImGui::IsMouseClicked(0) && !single_select)
    {
        unit->hovered = false;
        unit->selected = NOT_SELECTED;
    }

    if (ImGui::IsMouseClicked(0))
    {
        g_map.opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(0, 0));
    }

    if (unit->selected)
    {
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(pos.x - 5 * animated_map_scale, pos.y - 5 * animated_map_scale), ImVec2(pos.x + 5 * animated_map_scale, pos.y + 5 * animated_map_scale), ImColor(255, 255, 255));
    }
}


void map_processing::render_map_and_process_hitboxes(window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
{

    //grid
    ImGui::GetBackgroundDrawList()->AddImage(
        (ImTextureID)window.Grid,

        ImVec2((map_pos.x + 200) * animated_map_scale,
            map_pos.y * animated_map_scale - 200 * animated_map_scale),

        ImVec2(map_pos.x * animated_map_scale + 1450 * animated_map_scale,
            map_pos.y * animated_map_scale + 550 * animated_map_scale),
        ImVec2(0, 0),
        ImVec2(1, 1),
        ImColor(100, 100, 150, 90)
    );

    {
        static NavigableArea navigable_area;
        static bool navigate_area_init;
        if (!navigate_area_init)
        {
            if (!navigable_area.LoadFromFile("navigable_area.txt")) {
                std::cout << "Не удалось загрузить данные." << std::endl;
            }
            navigate_area_init = true;
        }


        //navigable_area.Draw(map_pos, animated_map_scale, ImVec2(g_map.cursor_pos.x, g_map.cursor_pos.y));

        g_city_processing.update_hitboxes(countries);

        //main cycle
        for (int i = 0; i < countries->size(); i++)
        {
            //hitbox processing
            g_city_processing.     process_country_hitboxes   (i, window, countries, animated_map_scale, hovered_id, cursor_pos, map_pos, player_id, function_count);
        }

        for (int i = 0; i < countries->size(); i++)
        {
            //rendering citys and country image
            g_city_processing.     process_country_and_cities (i, window, countries, animated_map_scale, hovered_id, cursor_pos, map_pos, player_id, function_count);

            //processing buildings
            g_building_processing. process_buildings          (navigable_area, i, window, countries, animated_map_scale, hovered_id, cursor_pos, map_pos, player_id, function_count);
            
        }
        g_rocket_processing.       process_rockets            (0, window, countries, animated_map_scale, hovered_id, cursor_pos, map_pos, player_id, function_count);

        //process warships
        g_warships.process_warships(navigable_area, countries, animated_map_scale, cursor_pos, map_pos, player_id, function_count);


    }
}

void map_processing::process_map(window_profiling window, int screen_size_x, int screen_size_y, int player_id)
{

    //global vars [ Pos ]
    static ImVec2 final_map_pos;
    static ImVec2 moved_pos = ImVec2(-190, 241.5);

    //global vars [ Scale ]
    static float  map_scale = 2;
    static float  animated_map_scale;
    

    //updating cursor and screen size
    g_tools.update_cursor_and_screen_size(screen_size_x, screen_size_y);

    //mouse scrolling
    g_tools.process_mouse3_scroll(&moved_pos, &map_scale, &animated_map_scale);


    //map moving
    g_tools.move_map               (&moved_pos, map_scale);

    //mouse3 map movement
    g_tools.process_mouse3_movement(&moved_pos, map_scale);


    //animating map moving
    static float animated_map_pos_x; animated_map_pos_x = ImGui::LerpAnimate("Map_Move", "PositionX", true, moved_pos.x, 14, ImGui::INTERP);
    static float animated_map_pos_y; animated_map_pos_y = ImGui::LerpAnimate("Map_Move", "PositionY", true, moved_pos.y, 14, ImGui::INTERP);
    final_map_pos = ImVec2(animated_map_pos_x, animated_map_pos_y);


    //creating city
    static bool cities_created;
    static std::vector <city>* city_data; 
    if (!cities_created)
    {
        city_data = g_map_helper.create_cities();
        cities_created = true;
    }


    //offsets in map
    static const ImVec2 offset = ImVec2(500, 1);
    static const int color_offset = 15;

    if (countries.empty())
    countries =
    {
        { "North-America",  window.countries[window.countries_name::USA],                   ImVec2(-48.6336 + offset.x, 53.9273 + offset.y)   ,  ImVec2(1683, 2111), ImVec2(0.195, 0.123),       ImColor(96, 151, 223),   city_data[window.countries_name::USA],              },
        { "EU",             window.countries[window.countries_name::EC],                    ImVec2(241.1947 + offset.x, 22.4489 + offset.y)   ,  ImVec2(1320, 1969), ImVec2(0.18, 0.23),         ImColor(133, 205, 212),   city_data[window.countries_name::EC]                },
        { "North-Europe",   window.countries[window.countries_name::Northeurope],           ImVec2(377.7431 + offset.x, 0.8408 + offset.y)   ,   ImVec2(620, 1131),  ImVec2(0.1818, 0.1306),     ImColor(133, 205, 212),   city_data[window.countries_name::Northeurope]       },
        { "Australia",      window.countries[window.countries_name::Austrilia],             ImVec2(849.2101 + offset.x, 488.0281 + offset.y)   , ImVec2(1028, 805),  ImVec2(0.1206, 0.188),      ImColor(57, 57, 57),   city_data[window.countries_name::Austrilia]         },
        { "Russia",         window.countries[window.countries_name::Russia],                ImVec2(698.6149 + offset.x, 17.3418 + offset.y)   ,  ImVec2(2467, 1538), ImVec2(0.1442, 0.1806),     ImColor(130, 213, 87),   city_data[window.countries_name::Russia]            },
        { "China",          window.countries[window.countries_name::China],                 ImVec2(696.0981 + offset.x, 215.8585 + offset.y)   , ImVec2(886, 651),   ImVec2(0.2062, 0.153),      ImColor(221, 197, 86),   city_data[window.countries_name::China]             },
        { "Central Asia",   window.countries[window.countries_name::Churki],                ImVec2(567.2284 + offset.x, 177.5464 + offset.y)   , ImVec2(591, 423),   ImVec2(0.136, 0.1976),      ImColor(57, 57, 57),   city_data[window.countries_name::Churki]            },
        { "East EU",        window.countries[window.countries_name::EastEC],                ImVec2(416.0617 + offset.x, 180.4763 + offset.y)   , ImVec2(330, 420),   ImVec2(0.206, 0.2302),      ImColor(57, 57, 57),   city_data[window.countries_name::EastEC]            },
        { "East Europe",    window.countries[window.countries_name::east_europe],           ImVec2(443.122 + offset.x, 154.4864 + offset.y)   ,  ImVec2(263, 269),   ImVec2(0.1234, 0.126201),   ImColor(57, 57, 57),   city_data[window.countries_name::east_europe]       },
        { "IndoChina",      window.countries[window.countries_name::Indo_China],            ImVec2(693.4327 + offset.x, 306.3944 + offset.y)   , ImVec2(178, 267),   ImVec2(0.1648, 0.1238),     ImColor(57, 57, 57),   city_data[window.countries_name::Indo_China]        },
        { "Indostan",       window.countries[window.countries_name::Indostan],              ImVec2(586.6926 + offset.x, 272.443 + offset.y)   ,  ImVec2(828, 540),   ImVec2(0.1954, 0.1272),     ImColor(57, 57, 57),   city_data[window.countries_name::Indostan]          },
        { "Latin-USA",      window.countries[window.countries_name::LatinUSA],              ImVec2(70.8135 + offset.x, 296.821 + offset.y)   ,   ImVec2(461, 302),   ImVec2(0.216, 0.141),       ImColor(57, 57, 57),   city_data[window.countries_name::LatinUSA]          },
        { "North-Africa",   window.countries[window.countries_name::North_WellWellWell],    ImVec2(401.2982 + offset.x, 280.6873 + offset.y)   , ImVec2(1060, 520),  ImVec2(0.124, 0.120),       ImColor(163, 169, 96),   city_data[window.countries_name::North_WellWellWell]},
        { "East-Asia",      window.countries[window.countries_name::Samurai],               ImVec2(794.3515 + offset.x, 304.7909 + offset.y)   , ImVec2(1081, 1035), ImVec2(0.126, 0.1211),      ImColor(128, 111, 62),   city_data[window.countries_name::Samurai]           },
        { "South-America",  window.countries[window.countries_name::SouthernUSA],           ImVec2(100.3651 + offset.x, 460.8606 + offset.y)   , ImVec2(1203, 1225), ImVec2(0.14, 0.142),        ImColor(57, 57, 57),   city_data[window.countries_name::SouthernUSA]       },
        { "Mid-Africa",     window.countries[window.countries_name::MidWellWellWell],       ImVec2(438.9074 + offset.x, 340.2161 + offset.y)   , ImVec2(623, 291),   ImVec2(0.146, 0.1374),      ImColor(57, 57, 57),   city_data[window.countries_name::MidWellWellWell]   },
        { "South-Africa",   window.countries[window.countries_name::South_WellWellWell],    ImVec2(443.0285 + offset.x, 439.819 + offset.y)   ,  ImVec2(563, 849),   ImVec2(0.1316, 0.1986),     ImColor(57, 57, 57),   city_data[window.countries_name::South_WellWellWell]},
        { "Turkey",         window.countries[window.countries_name::Turk],                  ImVec2(463.8263 + offset.x, 226.1221 + offset.y)   , ImVec2(335, 243),   ImVec2(0.1596, 0.2266),     ImColor(57, 57, 57),   city_data[window.countries_name::Turk]              },
        { "Zakavkazie",     window.countries[window.countries_name::Zakavkazie],            ImVec2(491.7546 + offset.x, 202.2194 + offset.y)   , ImVec2(154, 101),   ImVec2(0.1432, 0.1956),     ImColor(57, 57, 57),   city_data[window.countries_name::Zakavkazie]        }
    };

    int hovered_country_id = -1;

    if (final_map_pos.x > 1025)
    {
        ImGui::ChangeAnimateValue("Map_Move", "PositionX", final_map_pos.x - 1250);
        animated_map_pos_x = final_map_pos.x - 1250;
        moved_pos.x = moved_pos.x - 1250;
        final_map_pos.x = final_map_pos.x - 1250;
    }
    if (final_map_pos.x < -1494)
    {
        ImGui::ChangeAnimateValue("Map_Move", "PositionX", final_map_pos.x + 1250);
        animated_map_pos_x = final_map_pos.x + 1250;
        moved_pos.x = moved_pos.x + 1250;
        final_map_pos.x = final_map_pos.x + 1250;
    }


    //map processing

    this->render_map_and_process_hitboxes(window, &countries, animated_map_scale, &hovered_country_id, ImVec2(cursor_pos.x, cursor_pos.y), final_map_pos, player_id, 1);
    //first map

    ImVec2 final_pos_secondary = final_map_pos;
    if (final_map_pos.x < -225)
    {
    
        final_pos_secondary.x += 1250;
    }
    else {
        final_pos_secondary.x -= 1250;
    }

    //second map
    this->render_map_and_process_hitboxes(window, &countries, animated_map_scale, &hovered_country_id, ImVec2(cursor_pos.x, cursor_pos.y), final_pos_secondary, player_id, 2);

    process_and_sync_game_cycle(&countries, player_id, animated_map_scale, hovered_country_id);

}

//game tick and event processing
void process_events(int tick_duration_ms  = 15)
{
    static int game_length;

    if (g_menu.selected_game_mode == 0)
    {
        game_length = 30;
    }
    else if (g_menu.selected_game_mode == 1)
    {
        game_length = 45;
    }

    int total_game_time_sec = ((game_length / 5) * 60);
    int elapsed_time_sec = (g_map.global_tick * tick_duration_ms) / 1000;

    // Events switching
    if (elapsed_time_sec > total_game_time_sec * 0) { g_map.game_events = g_map.game_events::PREPARATION_EVENT; }
    if (elapsed_time_sec > total_game_time_sec * 1) { g_map.game_events = g_map.game_events::DOCKYARD_RELEASE; }
    if (elapsed_time_sec > total_game_time_sec * 2) { g_map.game_events = g_map.game_events::AIRCRAFR_RELEASE; }
    if (elapsed_time_sec > total_game_time_sec * 3) { g_map.game_events = g_map.game_events::NUCLEAR_DANGER; }
    if (elapsed_time_sec > total_game_time_sec * 4) { g_map.game_events = g_map.game_events::ANIHILATION; }
    if (elapsed_time_sec > total_game_time_sec * 5) { g_map.game_events = g_map.game_events::GAME_END; }
}

void game_event_timer()
{
    while (true)
    {
        process_events();
        std::this_thread::sleep_for(std::chrono::milliseconds(15)); //TODO : change it to 1 second in release version
        g_map.global_tick++;
    }
}

//game processing
void map_processing::process_and_sync_game_cycle(std::vector <country_data>* countries, int player_id, float animated_map_scale, int hovered_country_id)
{
    if (!tick_started)
    {
        std::thread(game_event_timer).detach();
        tick_started = true;
    }

    if (g_socket_control.player_role        ==      g_socket_control.player_role_enum::SERVER)
    {
        //server side 

        //tick and event update
        static int old_global_tick = 0;
        static int old_game_event  = 0;

        //sending global game tick to client
        if (old_global_tick != global_tick)         { g_socket_control.server_send_tick ();        old_global_tick = global_tick;        }

        //sending global game event to client
        if (old_game_event  != g_map.game_events)   { g_socket_control.server_send_event();        old_game_event = g_map.game_events;   }

        g_socket_control.server_process_client_sync();
    }
    else if (g_socket_control.player_role   ==      g_socket_control.player_role_enum::CLIENT)
    {
        //client side
        g_socket_control.client_process_client_sync();
    }

    //ecomics cycle
    static int old_global_tick_for_economy;
    static float capital_goal_for_animation = g_menu.players[player_id].economics.capital;
    if (global_tick % 5 == 0 && old_global_tick_for_economy != global_tick)
    {
        old_global_tick_for_economy = global_tick;
        capital_goal_for_animation += g_menu.players[player_id].economics.capital_inflow * (g_menu.players[player_id].economics.capital_inflow_ratio / 100);
    }

    float progress = (capital_goal_for_animation - g_menu.players[player_id].economics.capital) / 10;


    if (progress > 1)
        g_menu.players[player_id].economics.capital += progress;
    else
        g_menu.players[player_id].economics.capital = capital_goal_for_animation;

    bool is_server = g_socket_control.player_role == g_socket_control.player_role_enum::SERVER;
    
    //drag n drop
    {
        int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(g_window.window_size);
        ImGui::Begin("Drag n Drop For Ships And Air Ships", (bool*)1, flags);
        {


            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 0, 0, 0)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(0, 0, 0, 0)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(0, 0, 0, 0)));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 0, 0, 0)));

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Button("DNDSS", g_window.window_size);
            if (ImGui::BeginDragDropTarget())
            {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM2"))
                    {
                        int payload_idx = *(const int*)payload->Data;
                        {

                            if (!countries->at(g_menu.players[player_id].control_region).cancel_dragging)
                            {
                                int player_region = g_menu.players[player_id].control_region;
                                int buildings_id = countries->at(player_region).what_building_is_dragging;

                                switch (countries->at(player_region).what_type_of_boat_are_we_dragging)
                                {
                                case boats::SUBMARINES:
                                {
                                    countries->at(player_region).buildings[buildings_id].shipyard_heart.submarines.erase(countries->at(player_region).buildings[buildings_id].shipyard_heart.submarines.begin() + payload_idx);
                                    g_menu.players[player_id].war_property.submarine_count--;
                                }
                                break;

                                case boats::AIR_CARRIERS:
                                {
                                    countries->at(player_region).buildings[buildings_id].shipyard_heart.carriers.erase(countries->at(player_region).buildings[buildings_id].shipyard_heart.carriers.begin() + payload_idx);
                                    g_menu.players[player_id].war_property.carrier_count--;
                                }
                                break;

                                case boats::DESTROYER:
                                {
                                    countries->at(player_region).buildings[buildings_id].shipyard_heart.destroyers.erase(countries->at(player_region).buildings[buildings_id].shipyard_heart.destroyers.begin() + payload_idx);
                                    g_menu.players[player_id].war_property.destroyer_count--;
                                }
                                break;

                                case boats::CRUISERS:
                                {
                                    countries->at(player_region).buildings[buildings_id].shipyard_heart.cruisers.erase(countries->at(player_region).buildings[buildings_id].shipyard_heart.cruisers.begin() + payload_idx);
                                    g_menu.players[player_id].war_property.cruiser_count--;
                                }
                                break;

                                }

                                countries->at(player_region).what_building_is_dragging = 0;

                                //spawn boat
                                units_base new_unit;

                                new_unit.class_of_unit = countries->at(player_region).what_type_of_boat_are_we_dragging;
                                new_unit.unique_id = g_tools.generate_unique_int();
                                new_unit.warship = true;
                                new_unit.airplane = false;
                                new_unit.owner_country_id = player_region;
                                new_unit.owner_building_id = buildings_id;
                                new_unit.spawn_pos = ImVec2(cursor_pos.x, cursor_pos.y);

                                g_map.units.push_back(new_unit);
                            }
                            g_map.drag_n_drop = false;
                        }
                    }
                
                ImGui::EndDragDropTarget();
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();

        }
        ImGui::End();
    }
    //timer visual
    {
        auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, g_tools.convert_tick_to_timer().c_str());
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x / 2 - 30, 30 - 5), ImVec2(screen_x / 2 + 30, 30 + 23), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x / 2 - 30, 30 - 5), ImVec2(screen_x / 2 + 30, 30 + 23), ImColor(79, 255, 69, 130));
        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x / 2 - textsize.x / 2, 30), ImColor(255, 255, 255), g_tools.convert_tick_to_timer().c_str());
    }

    //timer visual
    {
        static const char* stage_names[] =
        {
            "Preparation time",
            "Mobilisation",
            "Mobilisation : 2",
            "Nuclear Danger",
            "Anihiliation",
            "The End",
        };

        auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, stage_names[game_events]);
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x / 2 - textsize.x, 75 - textsize.y / 2), ImVec2(screen_x / 2 + textsize.x, 75 + textsize.y), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x / 2 - textsize.x, 75 - textsize.y / 2), ImVec2(screen_x / 2 + textsize.x, 75 + textsize.y), ImColor(79, 255, 69, 130));
        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x / 2 - textsize.x / 2, 70), ImColor(255, 255, 255), stage_names[game_events]);
    }

    //playerlist
    {
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x - 210, 19), ImVec2(screen_x - 25, 25 + (25 * g_menu.players.size())), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x - 210, 19), ImVec2(screen_x - 25, 25 + (25 * g_menu.players.size())), ImColor(79, 255, 69, 130));

        for (int i = 0; i < g_menu.players.size(); i++)
        {
            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x - 200, 25 + (25 * i)), ImColor(255, 255, 255), g_menu.players[i].name.c_str());
        }
    }

    static int last_playerlist_pos_y = 50 + (25 * g_menu.players.size());

    //building ui
    {
        static int opened_tab_right_ui;
        ImGui::SetNextWindowPos(ImVec2(screen_x - 220, last_playerlist_pos_y));
        ImGui::SetNextWindowSize(ImVec2(screen_x - 25, last_playerlist_pos_y + 500));
        ImGui::Begin("Building Window", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove 
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDecoration);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
            {
                    {
                        ImGui::PushFont(g_xgui.fonts[2].font_addr);
                        {
                            ImGui::ButtonEx(std::string(g_tools.format_currency(int(g_menu.players[player_id].economics.capital)) + std::string("$")).c_str(), ImVec2(160, 40), ImGuiButtonFlags_NoInput);
                        }
                        ImGui::PopFont();

                        ImGui::PushFont(g_xgui.fonts[3].font_addr);
                        {
                            ImGui::NewLine();

                            if (ImGui::Button("Building mode", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 1)
                                    opened_tab_right_ui = 0;
                                else
                                opened_tab_right_ui = 1;
                            }
                            ImGui::NewLine();

                            if (ImGui::Button("Diplomacy", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 2)
                                    opened_tab_right_ui = 0;
                                else
                                    opened_tab_right_ui = 2;
                            }
                            ImGui::NewLine();

                            if (ImGui::Button("Economics", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 3)
                                    opened_tab_right_ui = 0;
                                else
                                    opened_tab_right_ui = 3;
                            }
                            ImGui::NewLine();
                        }
                }
                ImGui::PopFont();
            }
            ImGui::PopStyleVar();
        }

        ImGui::End();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            opened_tab_right_ui = 0;
        }

        static int should_build_building_id = 0;

        switch (opened_tab_right_ui)
        {
        case 1:
        {
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 - 400, screen_y - 270));
            ImGui::SetNextWindowSize(ImVec2(800, 400));
            ImGui::Begin("Building", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDecoration);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
                {
                    ImGui::PushFont(g_xgui.fonts[3].font_addr);
                    {
                        if (ImGui::Button("AIR", ImVec2(250, 25))) {}                                                          ImGui::SameLine(); if (ImGui::Button("WATER", ImVec2(250, 25))) {}                                                          ImGui::SameLine(); if (ImGui::Button("MISSILE", ImVec2(250, 25))) {}
                        if (ImGui::Button("Aircraft Factory [ 25.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::AIRCRAFT_FACTORY; } ImGui::SameLine(); if (ImGui::Button("Shipyard [ 20.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::SHIPYARD; }         ImGui::SameLine(); if (ImGui::Button("Missile Defense [ 9.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::MISSILE_DEFENSE; }
                        if (ImGui::Button("Field Airstrip [ 10.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::FIELD_AIR_STRIP; }  ImGui::SameLine(); if (ImGui::Button(" ", ImVec2(250, 50))) {}                                                          ImGui::SameLine(); if (ImGui::Button("Missile Silo [ 13.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::MISSILE_SILO; }
                        if (ImGui::Button("Permanent Airfield [ 20.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::PERMANENT_AIRFIELD; }ImGui::SameLine(); if (ImGui::Button("   ", ImVec2(250, 50))) {}                                                          ImGui::SameLine(); if (ImGui::Button("Radar [ 7 000 000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::RADAR; }
                    }
                    ImGui::PopFont();
                }
                ImGui::PopStyleVar();
            }
            ImGui::End();
        }
        break;

        }

        if (should_build_building_id != 0)
        {
            static int price;
            static int animaton_for_no_money_text;

            switch (should_build_building_id)
            {
                case AIRCRAFT_FACTORY:      { price = 25000000; } break;
                case FIELD_AIR_STRIP:       { price = 10000000; } break;
                case PERMANENT_AIRFIELD:    { price = 20000000; } break;
                case SHIPYARD:              { price = 20000000; } break;
                case MISSILE_DEFENSE:       { price = 9000000;  } break;
                case MISSILE_SILO:          { price = 13000000; } break;
                case RADAR:                 { price = 7000000;  } break;
            }

            if (true)
            {

                if (hovered_country_id == g_menu.players[player_id].control_region)
                {
                    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), 5 * animated_map_scale, ImColor(255, 0, 0, 250));

                    if (should_build_building_id == MISSILE_DEFENSE)
                    {
                        ImGui::GetForegroundDrawList()->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), 50.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                    }

                    if (ImGui::IsMouseDown(0))
                    {
                        ImVec4 pixel_color;
                        g_convex_math.GetScreenPixelWithGDI(cursor_pos.x, cursor_pos.y, pixel_color);
                        float absolute_brightness = (pixel_color.x * 0.2126) + (pixel_color.y * 0.7152) + (pixel_color.z * 0.0722);

                        bool color_check;
                        if (should_build_building_id == SHIPYARD)
                            color_check = absolute_brightness < 0.11;
                        else
                            color_check = absolute_brightness > 0.11;

                        if (g_menu.players[player_id].economics.capital >= price && color_check && cursor_pos.x > 100 && cursor_pos.y > 100 && cursor_pos.x < screen_x - 100 && cursor_pos.y < screen_y - 100)
                        {
                            opened_tab_right_ui = 0;
                            capital_goal_for_animation = g_menu.players[player_id].economics.capital - price;
                            building new_build;
                            new_build.building_type = should_build_building_id;
                            new_build.pos = ImVec2(cursor_pos.x, cursor_pos.y);
                            new_build.progress_of_building = 0;
                            new_build.endurance = 0;
                            new_build.hovered = false;
                            new_build.selected = NOT_SELECTED;

                            switch (new_build.building_type)
                            {
                                case AIRCRAFT_FACTORY:      { new_build.name = "Aircraft factory"; } break;
                                case FIELD_AIR_STRIP:       { new_build.name = "Field Airstrip"; } break;
                                case PERMANENT_AIRFIELD:    { new_build.name = "Permanent Airfield"; } break;
                                case SHIPYARD:              { new_build.name = "Shipyard"; } break;
                                case MISSILE_DEFENSE:       { new_build.name = "Missile Defense"; } break;
                                case MISSILE_SILO:          { new_build.name = "Missile Silo"; } break;
                                case RADAR:                 { new_build.name = "Radar"; } break;
                            }

                            countries->at(g_menu.players[player_id].control_region).buildings.push_back(new_build);

                            should_build_building_id = 0;
                        }
                        else
                        {
                            opened_tab_right_ui = 0;
                            should_build_building_id = 0;
                        }
                    }
                }
            }

        }
    }

    //selector
    {
        bool should_selector = true;
        if (g_map.cursor_pos.x > g_map.opened_menu_size.Min.x && g_map.cursor_pos.x < g_map.opened_menu_size.Max.x)
        {
            if (g_map.cursor_pos.y > g_map.opened_menu_size.Min.y && g_map.cursor_pos.y < g_map.opened_menu_size.Max.y)
            {
                should_selector = false;
            }
        }
        if (should_selector)
        {
            static int old_cursor_pos_x, old_cursor_pos_y;
            if (ImGui::IsMouseDown(0))
            {
                if (old_cursor_pos_x == 0 && old_cursor_pos_y == 0)
                {
                    old_cursor_pos_x = cursor_pos.x;
                    old_cursor_pos_y = cursor_pos.y;
                }

                if (old_cursor_pos_x != cursor_pos.x && old_cursor_pos_y != cursor_pos.y)
                {
                    selector_zone = ImRect(ImVec2(old_cursor_pos_x, old_cursor_pos_y), ImVec2(cursor_pos.x, cursor_pos.y));
                    ImGui::GetForegroundDrawList()->AddRect(selector_zone.Min, selector_zone.Max, ImColor(79, 255, 69, 130));
                }
            }
            else if (ImGui::IsMouseReleased(0))
            {
                selector_zone = ImRect(ImVec2(0, 0), ImVec2(0, 0));
                old_cursor_pos_x = 0;
                old_cursor_pos_y = 0;
            }
        }
    }
}
