#pragma once
#include "../resources/imgui_resource/imgui.h"

#include "../xguibox/xgui.h"
#include "../resources/imgui_resource/imgui_internal.h"
#include "../main/menu.h"
#include "../resources/window_profiling/window.h"
#include <cmath>
#include <limits>
#include <iostream> 
#include <algorithm> 
#include <fstream>

#include "structs/country_struct.h"
#include "units/units_class.h"


#define WIN32_LEAN_AND_MEAN
#ifndef YOUR_HEADER_FILE_H 
#define YOUR_HEADER_FILE_H
#endif  //YOUR_HEADER_FILE_H

class map_processing
{
public:


    //map
    std::vector < country_data > countries;
    float map_scale2 = 0.240010;


    //tech vars
    int screen_x, screen_y;

    POINT  cursor_pos;

    //selector
    ImRect selector_zone;

    //menus
    ImRect opened_menu_size;
    bool selection_for_nuclear_strike;
    int current_striking_building_id; // BUILDING THAT STRIKES
    std::vector < nuclear_strike_target > air_strike_targets;
    
    //units
    std::vector < units_base > units;

    //cycles
    void process_and_sync_game_cycle(std::vector <country_data>* countries, int player_id, float animated_map_scale, int hovered_country_id);

    void process_object_selections(bool city, int current_country, int player_id, std::vector <country_data>* countries, map_objects* object, float animated_map_scale, ImVec2 map_pos);

    void process_unit_selections(units_base* unit, float animated_map_scale);

    void process_map(window_profiling window, int screen_size_x, int screen_size_y, int player_id);

    void render_map_and_process_hitboxes(window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count);

    //tick and events
    bool tick_started;

    int global_tick;

    int game_events;

    enum game_events 
    {
        PREPARATION_EVENT,
        DOCKYARD_RELEASE,
        AIRCRAFR_RELEASE,
        NUCLEAR_DANGER,
        ANIHILATION,
        GAME_END
    };
};
inline map_processing g_map;

