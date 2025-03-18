#include "../../resources/imgui_resource/imgui.h"
#include <vector>
#include "../structs/rocket_struct.h"
#include "../structs/basic_object_struct.h"

class units_base
{
public:

	int health = 100;

	//owner informatiom
	int owner_country_id;
	int owner_building_id;

	//unit info
	bool warship  = false;
	bool airplane = false;

	int  class_of_unit = 0;

	int  unique_id;

	ImVec2 spawn_pos				= ImVec2();
	ImVec2 converted_spawn_pos		= ImVec2();
	ImVec2 position					= ImVec2();
	ImVec2 position_second_map	    = ImVec2();
	ImVec2 old_position				= ImVec2();
	ImVec2 move_offset				= ImVec2();
	ImVec2 interpolated_move_offset = ImVec2();
	ImVec2 target_pos				= ImVec2();
	ImVec2 stored_cursor			= ImVec2();

	std::vector<ImVec2> path = {};

	int old_tick;
	int reload_for_turrets_tick = 0;
	int reload_for_nuclear_tick = 0;
	std::vector < nuclear_strike_target > strike_queue;
	bool ready_to_nuclear;
	int last_path_update;
	int stuck_tick_timer;

	bool   pos_converted_to_map = true;
	bool   spawnpos_converted_to_map = false;

	bool hovered;
	selecting_type selected;
};