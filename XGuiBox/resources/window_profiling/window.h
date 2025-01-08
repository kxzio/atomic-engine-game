#pragma once

//GUI RESOURCES
#include "../imgui_resource/imgui.h"
#include "../imgui_resource/imgui_impl_dx11.h"
#include "../imgui_resource/imgui_impl_win32.h"

#include <dinput.h>
#include <tchar.h>
#include <d3d11.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

class window_profiling
{
public:

	ID3D11Device* g_pd3dDevice = NULL;
	// Создание устройства DirectX 11 и контекста
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IDXGISwapChain* g_pSwapChain = nullptr; // Глобальная переменная для цепочки обмена
	ID3D11DeviceContext* g_pd3dDeviceContext = nullptr; // Контекст устройства Direct3D 11
	ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // Рендер-таргет

	window_profiling() = default;

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

	void set_name(std::string s)
	{
		name_of_class = s;
	}
	void make_it_fullscreen();

	void load_textures(ID3D11Device* device, ID3D11DeviceContext* context);

	void unload_textures();

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



	ID3D11ShaderResourceView* countries[20];  // Используйте Shader Resource View для текстур стран
	ID3D11ShaderResourceView* Logotype;      // Текстура для логотипа
	ID3D11ShaderResourceView* Noise;         // Текстура для шума
	ID3D11ShaderResourceView* Tv;            // Текстура для телевизионного эффекта
	ID3D11ShaderResourceView* RGB;           // Текстура для RGB линий



private:

	ImVec2 window_size, window_pos;

	int type_of_window;

	std::string name_of_class;

	HWND HWND_OF_PROCESS;
};

inline window_profiling g_window;