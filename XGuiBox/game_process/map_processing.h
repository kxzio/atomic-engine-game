#pragma once
#include "../resources/imgui_resource/imgui.h"

#include "../xguibox/xgui.h"
#include "../resources/imgui_resource/imgui_internal.h"
#include "../menu.h"
#include "../resources/window_profiling/window.h"
#include <cmath>
#include <limits>
#include <iostream> 
#include <algorithm> 
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#ifndef YOUR_HEADER_FILE_H 
#define YOUR_HEADER_FILE_H
#endif  //YOUR_HEADER_FILE_H

struct Point {
    int x, y;

    Point(int x, int y) : x(x), y(y) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

struct ComparePoints {
    bool operator()(const Point& p1, const Point& p2) const {
        return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
    }
};

struct ConvexMathOperations
{
    int Cross(const Point& o, const Point& a, const Point& b) {
        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
    }

    int orientation(const Point& p, const Point& q, const Point& r) {

        int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
        if (val == 0) return 0;  // ����� �����������
        return (val > 0) ? 1 : 2;  // 1 - �� ������� �������, 2 - ������
    }
    
    bool GetScreenPixelWithGDI(int x, int y, ImVec4& outColor) 
    {
        HDC hdcScreen = GetDC(nullptr); // ���������� ������
        if (!hdcScreen) return false;

        COLORREF color = GetPixel(hdcScreen, x, y); // ������ �������
        ReleaseDC(nullptr, hdcScreen);             // ����������� ����������

        if (color == CLR_INVALID) return false;

        // ����������� COLORREF � RGBA
        BYTE r = GetRValue(color);
        BYTE g = GetGValue(color);
        BYTE b = GetBValue(color);
        outColor = ImColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);

        return true;
    }

    bool IsPointInsidePolygon2(const Point& p, const std::vector<Point>& polygon) {
        int n = polygon.size();
        bool inside = false;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            const Point& pi = polygon[i];
            const Point& pj = polygon[j];
            if ((pi.y > p.y) != (pj.y > p.y) &&
                p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x) {
                inside = !inside;
            }
        }
        return inside;
    }

    bool IsPointInsidePolygon(const Point& point, const std::vector<Point>& polygon) {
        int intersections = 0;
        int n = polygon.size();

        for (int i = 0; i < n; i++) {
            const Point& p1 = polygon[i];
            const Point& p2 = polygon[(i + 1) % n];

            if ((p1.y > point.y) != (p2.y > point.y)) {
                float x_intersect = p1.x + (point.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                if (point.x < x_intersect) {
                    intersections++;
                }
            }
        }

        return (intersections % 2) != 0;
    }
};
inline ConvexMathOperations g_convex_math;

enum selecting_type
{
    NOT_SELECTED,
    MULTIPLE_SELECTED,
    SOLO_SELECTED
};
struct map_objects
{
    std::string name;

    ImVec2 pos;

    selecting_type selected = NOT_SELECTED;

    bool hovered = false;

    bool highlighted = false;
};
struct nuclear_strike_target
{
    int GETTER_country_id;
    int GETTER_city_id;
    int GETTER_building_id;

    int SENDER_country_id;
    int SENDER_building_id;

    int last_global_tick = 0;
    int step_of_bomb;
    ImVec2 bomb_pos;
};

struct AIR_FACTORY_SYSTEM_HEART
{
    int old_tick_for_jets = 0;
    int goal_amount_of_jets = 0;
    int old_tick_for_bombers = 0;
    int goal_amount_of_bombers = 0;
};
struct SHIPYARD_SYSTEM_HEART
{
    int old_tick_for_boat1 = 0;
    int goal_amount_of_boat1 = 0;
    int old_tick_for_boat2 = 0;
    int goal_amount_of_boat2 = 0;
    int old_tick_for_boat3 = 0;
    int goal_amount_of_boat3 = 0;
    int old_tick_for_boat4 = 0;
    int goal_amount_of_boat4 = 0;

};
struct MISSILE_SILO_HEART
{
    int old_tick_for_reload = 0;
    bool ready_to_shot;
    std::vector < nuclear_strike_target > strike_queue;
};

class building : public map_objects
{
public:

    int building_type;

    int progress_of_building;

    int endurance;

    bool size_converted_to_map;


    //HEARTS
    AIR_FACTORY_SYSTEM_HEART air_factory_heart;
    SHIPYARD_SYSTEM_HEART    shipyard_heart;
    MISSILE_SILO_HEART       missile_silo_heart;
};

struct city : public map_objects
{
    city() = default;

    int population;
    city(std::string iname, ImVec2 ipos, int ipopulation = 0)
    {
        name = iname;
        pos = ipos;
        population = ipopulation;
    }
};

struct country_data
{
    //name and texture of country
    std::string name;
    ID3D11ShaderResourceView* texture;

    //poses and sizes
    ImVec2 position;
    ImVec2 size;
    ImVec2 hitbox_size;

    ImColor color;

    //objects
    std::vector < city >     cities;
    std::vector < building > buildings;

    //hitboxes
    bool hitbox_get = false;
    std::vector<Point>  convex_hull = {};
    std::vector<ImVec2> convex_hull_screen_coords;


};

class map_processing
{
public:

    std::vector < country_data > countries;

    //tech vars
    int screen_x, screen_y;

    POINT  cursor_pos;

    //selector
    ImRect selector_zone;

    //menus
    ImRect opened_menu_size;
    bool selection_for_nuclear_strike;
    int current_striking_building_id; // BUILDING THAT STRIKES
    std::vector < nuclear_strike_target > air_strike_targets;

    //cycles
    void process_and_sync_game_cycle(std::vector <country_data>* countries, int player_id, float animated_map_scale, int hovered_country_id);

    void process_map(window_profiling window, int screen_size_x, int screen_size_y, int player_id);

    void render_map_and_process_hitboxes(window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count);

    //tick and events
    bool tick_started;

    int global_tick;

    int game_events;

    enum game_events 
    {
        PREPARATION_EVENT,
        DOCKYARD_RELEASE,
        AIRCRAFR_RELEASE,
        NUCLEAR_DANGER,
        ANIHILATION,
        GAME_END
    };
};
inline map_processing g_map;

