#pragma once
#include "resources/window_profiling/window.h"

class player
{
public:

	std::string name;

	int id;

    int control_region;

	bool ready_to_play = false;
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

