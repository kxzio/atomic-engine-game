#pragma once

//GUI RESOURCES
#include "../imgui_resource/imgui.h"
#include "../imgui_resource/imgui_impl_dx9.h"
#include "../imgui_resource/imgui_impl_win32.h"

#include <dinput.h>
#include <tchar.h>
#include <d3d9.h>
#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;

class window_profiling
{
public:

	window_profiling(const char* string)
	{
		name_of_class = string;
	}

	enum types_of_window
	{
		POPUP			= 0x80000000L,
		CAPTION			= 0x00C00000L,
		BORDER			= 0x00800000L,
		NO_CONTROL		= 0x08000000L,
		DIALOGUE_FRAME	= 0x00400000L,
		SYSTEM_MENU		= 0x00080000L,
		TITLED			= 0x00000000L,
		TITLED_WINDOW	= WS_OVERLAPPEDWINDOW,
		ONLY_ICON		= 0x20000000L
	};

	void create_window();

	void set_window_size(ImVec2 par);

	void set_window_pos(ImVec2 par);

	void set_window_type(int type);

	void make_it_fullscreen();

	void load_textures();

	void unload_textures();

	void handle_device_lost();

	enum countries_name
	{
		USA,
		EC,
		Northeurope,
		Austrilia,
		Russia,
		China,
		Churki,
		EastEC,
		east_europe,
		Indo_China,
		Indostan,
		LatinUSA,
		North_WellWellWell,
		Samurai,
		SouthernUSA,
		MidWellWellWell,
		South_WellWellWell,
		Turk,
		Zakavkazie
	};



	IDirect3DTexture9* countries[20];

	IDirect3DTexture9* Logotype;

	IDirect3DTexture9* Noise;

	IDirect3DTexture9* Tv; 

	IDirect3DTexture9* RGB;


private:

	ImVec2 window_size, window_pos;

	int type_of_window;

	std::string name_of_class;

	HWND HWND_OF_PROCESS;
};
