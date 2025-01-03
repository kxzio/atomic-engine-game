#pragma once
#include "resources/window_profiling/window.h"

class building
{
public:
	int building_type;

	int progress_of_building;

	int endurance;

	ImVec2 pos;
	bool size_converted_to_map;
};
class econimics
{
public:

	float capital = 25000000;

	float capital_inflow = 15000;

	float capital_inflow_ratio = 100.f;
};

class player
{
public:

	std::string name;

	int id;

    int control_region;

	bool ready_to_play = false;

	econimics economics;

	std::vector < building > region_buildings;
};

class menu
{
public:
	void render(window_profiling window);

	//server settings
	int selected_game_mode;

	std::vector <std::string> player_names;

	std::vector < player > players;

};
inline menu g_menu;

class socket_control
{
public:

	enum player_role_enum
	{
		SERVER,
		CLIENT
	};

	int player_role;

	void server_send_message(std::string message);

	void client_send_message(std::string message);

	std::string game_cycle_messages;
};
inline socket_control g_socket_control;

