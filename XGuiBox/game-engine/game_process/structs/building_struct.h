#pragma once
#include <vector>
#include "../../resources/imgui_resource/imgui.h"
#include "basic_object_struct.h"
#include "rocket_struct.h"

struct AIR_FACTORY_SYSTEM_HEART
{
    int old_tick_for_jets = 0;
    int goal_amount_of_jets = 0;
    int old_tick_for_bombers = 0;
    int goal_amount_of_bombers = 0;
};
struct SHIPYARD_SYSTEM_HEART
{
    int old_tick_for_boat1 = 0;
    int goal_amount_of_boat1 = 0;
    int old_tick_for_boat2 = 0;
    int goal_amount_of_boat2 = 0;
    int old_tick_for_boat3 = 0;
    int goal_amount_of_boat3 = 0;
    int old_tick_for_boat4 = 0;
    int goal_amount_of_boat4 = 0;


    //listbox of boats
    std::vector< std::string > carriers;
    std::vector< std::string > cruisers;
    std::vector< std::string > destroyers;
    std::vector< std::string > submarines;

};
struct MISSILE_SILO_HEART
{
    int old_tick_for_reload = 0;
    bool ready_to_shot;
    std::vector < nuclear_strike_target > strike_queue;
};
struct MISSILE_DEFENSE_HEART
{
    int old_tick_for_reload = 0;
    bool ready_to_shot;

    bool rocket_targeted;
    ImVec2 targeted_tocket_pos;
};


class building : public map_objects
{
public:

    int building_type;

    int progress_of_building;

    int endurance;

    bool size_converted_to_map = false;

    //HEARTS
    AIR_FACTORY_SYSTEM_HEART air_factory_heart;
    SHIPYARD_SYSTEM_HEART    shipyard_heart;
    MISSILE_SILO_HEART       missile_silo_heart;
    MISSILE_DEFENSE_HEART    missile_defense_heart;


};
