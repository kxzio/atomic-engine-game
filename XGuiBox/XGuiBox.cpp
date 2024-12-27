//XGUIBOX - v 0.01

//GUI RESOURCES
#include "resources/imgui_resource/imgui.h"
#include "resources/imgui_resource/imgui_impl_dx9.h"
#include "resources/imgui_resource/imgui_impl_win32.h"
#include <dinput.h>
#include <tchar.h>
#include <d3d9.h>

#include "resources/window_profiling/window.h"
#include "xguibox/xgui.h"
#include "resources/OpenSans-Bold.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")


int main()
{
	/* Window title and creating main class, that creates window */
	window_profiling new_window_profile ("XGUI Presentation");

	/* Window size customization. This will be not avaviable in future */
	new_window_profile.set_window_pos (	ImVec2(0, 0)	);
	new_window_profile.set_window_size(	ImVec2(400, 400)	);
	
	new_window_profile.make_it_fullscreen();

	/* Font initialization ( Important to be before the "create window" function ) */
	g_xgui			  .add_font("verdana", 15);
	g_xgui            .add_font("cour", 16);
	g_xgui			  .add_font("framd", 45);
	g_xgui            .add_font("framd", 17);

	/* Main window is now appear on your screen with no (at this moment) interface */
	new_window_profile.create_window();


};