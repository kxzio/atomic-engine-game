#include "../../resources/imgui_resource/imgui.h"
#include <vector>

class units_base
{
public:

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
	ImVec2 move_offset				= ImVec2();
	ImVec2 interpolated_move_offset = ImVec2();
	ImVec2 target_pos				= ImVec2();
	ImVec2 stored_cursor			= ImVec2();

	std::vector<ImVec2> path = {};

	int old_tick;

	bool   pos_converted_to_map = true;
	bool   spawnpos_converted_to_map = false;

	bool hovered;
	bool selected;
};