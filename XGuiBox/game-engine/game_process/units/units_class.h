#include "../../resources/imgui_resource/imgui.h"

class units_base
{
public:

	bool warship  = false;
	bool airplane = false;

	int  owner_country_id;
	int  owner_building_id;

	int  unique_id;

	ImVec2 spawn_pos     = ImVec2();
	ImVec2 converted_spawn_pos = ImVec2();
	ImVec2 position      = ImVec2();
	ImVec2 move_offset   = ImVec2();
	ImVec2 target_pos    = ImVec2();
	ImVec2 stored_cursor = ImVec2();

	int old_tick;

	bool   pos_converted_to_map = false;
	bool   spawnpos_converted_to_map = false;

	bool hovered;
	bool selected;
};