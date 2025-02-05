#pragma once
#include "../../resources/imgui_resource/imgui.h"

struct nuclear_strike_target
{
    int unique_id = 0;
    int GETTER_country_id;
    int GETTER_city_id;
    int GETTER_building_id;
    int GETTER_rocket;

    int SENDER_country_id;
    int SENDER_building_id;

    int last_global_tick = 0;
    int step_of_bomb = 0;
    ImVec2 bomb_pos;


    int segments;
};
