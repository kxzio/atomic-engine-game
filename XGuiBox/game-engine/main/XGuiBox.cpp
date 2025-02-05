//XGUIBOX - v 0.01

//GUI RESOURCES
#include "../resources/imgui_resource/imgui.h"
#include "../resources/imgui_resource/imgui_impl_dx9.h"
#include "../resources/imgui_resource/imgui_impl_win32.h"
#include <dinput.h>
#include <tchar.h>
#include <d3d9.h>

#include "../resources/window_profiling/window.h"
#include "../xguibox/xgui.h"
#include "../resources/byte/OpenSans-Bold.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main()
{
	_CrtSetReportMode(_CRT_ASSERT, 0);

	/* Window title and creating main class, that creates window */
	g_window.set_name("XGUI Presentation");

	/* Window size customization. This will be not avaviable in future */
	g_window.set_window_pos (	ImVec2(0, 0)	);
	g_window.set_window_size(	ImVec2(400, 400)	);
	
	g_window.make_it_fullscreen();

	/* Font initialization ( Important to be before the "create window" function ) */
	g_xgui			  .add_font("verdana", 16);
	g_xgui            .add_font("cour", 16);
	g_xgui			  .add_font("framd", 25);
	g_xgui            .add_font("framd", 17);

	/* Main window is now appear on your screen with no (at this moment) interface */
	g_window.create_window();

};