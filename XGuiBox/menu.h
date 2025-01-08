#pragma once
#include "resources/window_profiling/window.h"
enum buildings
{
	AIRCRAFT_FACTORY = 1, SHIPYARD, MISSILE_DEFENSE,
	FIELD_AIR_STRIP, MISSILE_SILO,
	PERMANENT_AIRFIELD, RADAR
};

class econimics
{
public:

	float capital = 250000000;

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
};

class menu
{
public:
	void render(window_profiling window);

	//server settings
	int selected_game_mode;

	int change_res_x, change_res_y;

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

	//updating
	void client_send_player_class(int id);
	void server_send_player_class(int id);

	void client_send_city(int city_id, int country_id);
    void server_send_city(int city_id, int country_id);

	void client_send_building(int building_id, int country_id);
	void server_send_building(int building_id, int country_id);


	std::string game_cycle_messages;
};
inline socket_control g_socket_control;

