#pragma once
#include "../../resources/imgui_resource/imgui.h"

struct nuclear_strike_target
{
    int unique_id = 0;
    int GETTER_country_id;
    int GETTER_city_id = -1;
    int GETTER_building_id = -1;
    int GETTER_rocket = -1;
    int GETTER_unit = -1;

    int SENDER_country_id = -1;
    int SENDER_building_id = -1;
    int SENDER_unit = -1;

    int last_global_tick = 0;
    int step_of_bomb = 0;

    ImVec2 bomb_pos_map1;
    ImVec2 bomb_pos_map2;

    int segments;


    //for boats and airboats
    int damage = 0;

};
