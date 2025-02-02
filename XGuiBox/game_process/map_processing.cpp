#include "map_processing.h"
#include <sstream>
#include <chrono>
#include <random>

int generateUniqueInt() 
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    static std::mt19937_64 rng(millis);
    std::uniform_int_distribution<int> dist(0, 10000);

    int random_part = dist(rng);
    int unique_id = static_cast<int>(millis % 100000000) + random_part;

    return unique_id;
}

static float map_scale2 = 0.240010;

float CalculateSegments(ImVec2 start, ImVec2 end, float animated_map_scale)
{
    float distance = (std::sqrt((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y))) / animated_map_scale;

    return distance;
}

bool IsPointInImRect(const ImVec2& point, const ImRect& rect) {
    ImVec2 min = rect.Min;
    ImVec2 max = rect.Max;

    // ������������ ��� � ���� � ������ ������������� ��������
    if (min.x > max.x) std::swap(min.x, max.x);
    if (min.y > max.y) std::swap(min.y, max.y);

    return (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y);
}
std::vector<ImVec2> GetTrajectoryPoints(ImVec2 start, ImVec2 end, float height, int segments = 20)
{
    std::vector<ImVec2> points;

    // ���������� ��������� ��� ��������� ����
    const int num_segments = segments;

    // ���������� ����� ����
    for (int i = 0; i <= num_segments; ++i) {
        float t = (float)i / (float)num_segments;
        float x = start.x + t * (end.x - start.x);
        float y = start.y + t * (end.y - start.y) - height * sin(t * IM_PI);
        points.push_back(ImVec2(x, y));
    }

    return points;
}

ImVec2 GetTrajectoryOnePoint(ImVec2 start, ImVec2 end, int id, float height, int segments)
{

    int num_segments = segments;

    // ��������� ������� ����� �� ������������ ����������
    float t = (float)id / (float)num_segments;
    float x = start.x + t * (end.x - start.x);
    float y = start.y + t * (end.y - start.y) - height * sin(t * IM_PI); // ���� ����� �����


    return ImVec2(x, y);
}

std::vector<ImVec2> GetTrajectoryStrightPoints(ImVec2 start, ImVec2 end, int s) {

    int num_segments = 200;
    std::vector<ImVec2> points;

    // ���������� ����� �� ������ �����     
    for (int i = 0; i <= num_segments; ++i) {
        float t = (float)i / (float)num_segments;
        float x = start.x + t * (end.x - start.x);
        float y = start.y + t * (end.y - start.y);
        points.push_back(ImVec2(x, y));
    }

    return points;
}

ImVec2 GetTrajectoryStrightOnePoint(ImVec2 start, ImVec2 end, int id, int segments)
{
    int num_segments = segments;

    float t = (float)id / (float)num_segments;
    float x = start.x + t * (end.x - start.x);
    float y = start.y + t * (end.y - start.y);

    return ImVec2(x, y);
}


float CalculateDistance(ImVec2 point, ImVec2 circle_center) {
    
    return sqrt((pow(circle_center.x - point.x, 2) + pow(circle_center.y - point.y, 2)));
}


void DrawTrajectoryArc(ImDrawList* draw_list, ImVec2 start, ImVec2 end, ImVec2 bombpos, float height, ImU32 color, float segments) {

    auto points = GetTrajectoryPoints(start, end, height, segments);
    // ��������� ����

    bool direction_right;

    if (start.x < end.x)
        direction_right = true;
    else
        direction_right = false;

    // ������� �����, �� ������� ����� ����
    int num_segments = points.size();
    int target_segment = 0;
    for (int i = 0; i < num_segments; ++i) 
    {
        if (direction_right)
        {
            if (points[i].x >= bombpos.x) {
                target_segment = i;
                break;
            }
        }
        else
        {
            if (points[i].x <= bombpos.x) {
                target_segment = i;
                break;
            }

        }
    }

    draw_list->AddPolyline(points.data() + target_segment, num_segments - target_segment, color, false, 1.f);
}

void DrawTrajectoryStrightArc(ImDrawList* draw_list, ImVec2 start, ImVec2 end, ImVec2 bombpos, ImU32 color, float segments) {

    auto points = GetTrajectoryStrightPoints(start, end, segments);
    // ��������� ����

    bool direction_right;

    if (start.x < end.x)
        direction_right = true;
    else
        direction_right = false;

    // ������� �����, �� ������� ����� ����
    int num_segments = points.size();
    int target_segment = 0;
    for (int i = 0; i < num_segments; ++i)
    {
        if (direction_right)
        {
            if (points[i].x >= bombpos.x) {
                target_segment = i;
                break;
            }
        }
        else
        {
            if (points[i].x <= bombpos.x) {
                target_segment = i;
                break;
            }

        }
    }

    draw_list->AddPolyline(points.data() + target_segment, num_segments - target_segment, color, false, 1.f);
}
void process_object_selections(bool city, int current_country, int player_id, std::vector <country_data>* countries, map_objects* object, float animated_map_scale, ImVec2 map_pos)
{
    auto data = countries->at(current_country);

    auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
    auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;

    ImVec2 point_pos = ImVec2(posx + object->pos.x * animated_map_scale - 1.5 * animated_map_scale, posy + object->pos.y * animated_map_scale - 1.5 * animated_map_scale);
    ImVec2 point_size = ImVec2(posx + object->pos.x * animated_map_scale + 1.5 * animated_map_scale, posy + object->pos.y * animated_map_scale + 1.5 * animated_map_scale);

    bool single_select = false;

    if (g_map.cursor_pos.x > g_map.opened_menu_size.Min.x && g_map.cursor_pos.x < g_map.opened_menu_size.Max.x)
    {
        if (g_map.cursor_pos.y > g_map.opened_menu_size.Min.y && g_map.cursor_pos.y < g_map.opened_menu_size.Max.y)
        {
            return;
        }
    }

    if (current_country == g_menu.players[player_id].control_region || g_map.selection_for_nuclear_strike)
    {
        if (g_map.selector_zone.GetSize().x == 0)
        {
            if (IsPointInImRect(point_pos, ImRect(ImVec2(g_map.cursor_pos.x - 5 * animated_map_scale, g_map.cursor_pos.y - 5 * animated_map_scale), ImVec2(g_map.cursor_pos.x + 5 * animated_map_scale, g_map.cursor_pos.y + 5 * animated_map_scale))))
            {
                object->hovered = true;
                ImGui::GetForegroundDrawList()->AddRect(ImVec2(point_pos.x - 3 * animated_map_scale, point_pos.y - 3 * animated_map_scale), ImVec2(point_size.x + 3 * animated_map_scale, point_size.y + 3 * animated_map_scale), g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));

                if (ImGui::IsMouseReleased(0))
                {
                    single_select = true;
                    object->selected = SOLO_SELECTED;
                    ImGui::GetForegroundDrawList()->AddRect(point_pos, point_size, g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));

                    //nuclear target selection
                    if (current_country != g_menu.players[player_id].control_region)
                    {
                        if (g_map.selection_for_nuclear_strike)
                        {
                            int id = 0;
                            if (city)
                            {
                                for (int cityi = 0; cityi < countries->at(current_country).cities.size(); cityi++)
                                {
                                    if (countries->at(current_country).cities.at(cityi).selected == SOLO_SELECTED)
                                    {
                                        id = cityi;

                                        nuclear_strike_target new_target;
                                        new_target.unique_id = generateUniqueInt();
                                        new_target.GETTER_country_id  = current_country;
                                        new_target.GETTER_city_id     = id;
                                        new_target.GETTER_building_id = -1;
                                        new_target.GETTER_rocket      = -1;
                                        new_target.SENDER_country_id  = g_menu.players[player_id].control_region;
                                        new_target.SENDER_building_id = g_map.current_striking_building_id;

                                        countries->at(g_menu.players[player_id].control_region).buildings[g_map.current_striking_building_id].missile_silo_heart.strike_queue.push_back(new_target);
                                        g_map.selection_for_nuclear_strike = false;

                                    }
                                }
                            }
                            else
                            {
                                for (int build = 0; build < countries->at(current_country).buildings.size(); build++)
                                {
                                    if (countries->at(current_country).buildings.at(build).selected == SOLO_SELECTED)
                                    {
                                        id = build;

                                        nuclear_strike_target new_target;
                                        new_target.unique_id = generateUniqueInt();
                                        new_target.GETTER_country_id = current_country;
                                        new_target.GETTER_city_id = -1;
                                        new_target.GETTER_building_id = id;
                                        new_target.GETTER_rocket = -1;
                                        new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                        new_target.SENDER_building_id = g_map.current_striking_building_id;

                                        countries->at(g_menu.players[player_id].control_region).buildings[g_map.current_striking_building_id].missile_silo_heart.strike_queue.push_back(new_target);
                                        g_map.selection_for_nuclear_strike = false;

                                    }
                                }
                            }
                        }
                    }
                }

            }
            else object->hovered = false;
        }
        else
        {
            if (IsPointInImRect(point_pos, g_map.selector_zone))
            {
                object->hovered = true;
                ImGui::GetForegroundDrawList()->AddRect(ImVec2(point_pos.x - 3 * animated_map_scale, point_pos.y - 3 * animated_map_scale), ImVec2(point_size.x + 3 * animated_map_scale, point_size.y + 3 * animated_map_scale), g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));
            }
            else if (ImGui::IsMouseDown(0))
            {
                object->hovered = false;
            }
            else if (object->hovered || object->selected == MULTIPLE_SELECTED)
            {
                object->hovered = false;
                object->selected = MULTIPLE_SELECTED;
            }
        }
    }
    if (ImGui::IsMouseClicked(0) && !single_select)
    {
        object->hovered = false;
        object->selected = NOT_SELECTED;
        g_map.opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(0, 0));
    }

    if (object->selected)
    {
        ImGui::GetForegroundDrawList()->AddRect(point_pos, point_size, g_map.selection_for_nuclear_strike ? ImColor(255, 0, 0) : ImColor(255, 255, 255));
    }
}

void process_convex_file(const std::string& input_filename, std::vector<country_data>& countries) 
{
    std::ifstream file(input_filename);
    if (!file.is_open()) {
        std::cerr << "�� ������� ������� ����." << std::endl;
        return;
    }

    std::string line;
    country_data current_country;
    while (std::getline(file, line)) {
        // ���������� ������ ������
        if (line.empty()) {
            continue;
        }

        // ���� ��� ��� ������, ������ ����� ������ country_data
        if (line.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
            if (!current_country.name.empty()) {
                // ��������� ���������� ������
                countries.push_back(current_country);
            }
            current_country = country_data();  // ������ ����� ������ ��� ��������� ������
            current_country.name = line;  // ��� ������
        }
        else {
            // ������ ���������
            std::istringstream ss(line);
            float x, y;
            char comma;
            if (ss >> x >> comma >> y) {
                current_country.convex_hull.push_back(Point(x, y));  // ��������� �����
            }
        }
    }

    // ��������� ��������� ������
    if (!current_country.name.empty()) {
        countries.push_back(current_country);
    }

    file.close();
}


void map_processing::render_map_and_process_hitboxes(window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
{

    //grid
    {
        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)window.Grid,

            ImVec2((map_pos.x + 200) * animated_map_scale,
                map_pos.y * animated_map_scale - 200 * animated_map_scale),

            ImVec2(map_pos.x * animated_map_scale + 1450 *animated_map_scale,
                map_pos.y * animated_map_scale + 550 * animated_map_scale),
            ImVec2(0, 0),
            ImVec2(1, 1),
            ImColor(100, 100, 100, 90)
        );

    }

    static bool read;

    if (!read)
    {
        static std::vector<country_data> temp;
        process_convex_file("convex_hull.data", temp);
        for (size_t i = 0; i < temp.size(); ++i)
        {
            for (size_t i2 = 0; i2 < temp[i].convex_hull.size(); ++i2) 
            {
                const Point& pt = temp[i].convex_hull[i2];

                countries->at(i).convex_hull.push_back(Point(pt.x, pt.y));
            }
        }

        read = true;
    }

    /*
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

    */

    {
        //processing hitbox
        for (int i = 0; i < countries->size(); i++)
        {
            auto data = countries->at(i);
            static float map_scale2 = 0.240010;
            auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
            auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;
            auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) + map_pos.x * animated_map_scale;
            auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) + map_pos.y * animated_map_scale;

            auto sizex = secondposx - posx;
            auto sizey = secondposy - posy;

            int texture_size_x = data.size.x;  int texture_size_y = data.size.y;

            if (!countries->at(i).hitbox_get)
            {

                if (i == window.countries_name::Indostan)
                {
                    int start_index = 19;
                    int end_index = 36;

                    if (end_index < countries->at(i).convex_hull.size())
                    {
                        countries->at(i).convex_hull.erase(countries->at(i).convex_hull.begin() + start_index, countries->at(i).convex_hull.begin() + end_index + 1);
                    }
                }
                Point mouse_point = { static_cast<int>(cursor_pos.x), static_cast<int>(cursor_pos.y) };
                countries->at(i).hitbox_get = true;
            }


            float scale = animated_map_scale;

            std::vector <Point> converted_to_screen_convex;

            for (int i2 = 0; i2 < countries->at(i).convex_hull.size(); i2++)
            {
                Point pt = countries->at(i).convex_hull[i2];
                ImVec2 screen_pos = ImVec2(posx + pt.x * data.hitbox_size.x * scale, posy + pt.y * data.hitbox_size.y * scale);
                converted_to_screen_convex.push_back(Point(screen_pos.x, screen_pos.y));

                if (i == window.countries_name::Russia)
                {
                    if (i2 == 0)
                    {
                       countries->at(i).convex_hull[i2].x = 199;
                       countries->at(i).convex_hull[i2].y = 1589;
                    }

                }
                if (i == window.countries_name::EastEC)
                {
                    if (i2 == 0)
                    {
                       countries->at(i).convex_hull[i2].x = 622;countries->at(i).convex_hull[i2].y = 2033;
                    }
                    if (i2 == 1)
                    {
                       countries->at(i).convex_hull[i2].x = 34;countries->at(i).convex_hull[i2].y = 78;
                    }
                }
                if (i == window.countries_name::Austrilia)
                {
                    if (i2 == 0)
                    {
                       countries->at(i).convex_hull[i2].x = 202;
                       countries->at(i).convex_hull[i2].y = 174;
                    }
                }
                if (i == window.countries_name::Samurai)
                {
                    if (i2 == 0)
                    {
                       countries->at(i).convex_hull[i2].x = 178;
                       countries->at(i).convex_hull[i2].y = 1316;
                    }
                    if (i2 == 1)
                    {
                       countries->at(i).convex_hull[i2].x = 812;
                       countries->at(i).convex_hull[i2].y = 684;
                    }
                }
                if (i == window.countries_name::EC)
                {
                    if (i2 == 0)
                    {
                       countries->at(i).convex_hull[i2].x = 254; countries->at(i).convex_hull[i2].y = 1030;
                    }
                    if (i2 == 1)
                    {
                       countries->at(i).convex_hull[i2].x = 60; countries->at(i).convex_hull[i2].y = 388;
                    }
                    if (i2 == 2)
                    {
                       countries->at(i).convex_hull[i2].x = 158; countries->at(i).convex_hull[i2].y = 268;
                    }
                    if (i2 == 3)
                    {
                       countries->at(i).convex_hull[i2].x = 238; countries->at(i).convex_hull[i2].y = 180;
                    }
                    if (i2 == 4)
                    {
                       countries->at(i).convex_hull[i2].x = 304; countries->at(i).convex_hull[i2].y = 148;
                    }
                    if (i2 == 5)
                    {
                       countries->at(i).convex_hull[i2].x = 384; countries->at(i).convex_hull[i2].y = 128;
                    }
                    if (i2 == 6)
                    {
                       countries->at(i).convex_hull[i2].x = 436; countries->at(i).convex_hull[i2].y = 90;
                    }
                    if (i2 == 7)
                    {
                       countries->at(i).convex_hull[i2].x = 538; countries->at(i).convex_hull[i2].y = 42;
                    }
                    if (i2 == 8)
                    {
                       countries->at(i).convex_hull[i2].x = 634; countries->at(i).convex_hull[i2].y = 16;
                    }


                }
                if (i == window.countries_name::Northeurope)
                {
                    if (i2 == 20)
                    {
                       countries->at(i).convex_hull[i2].x = 532;
                       countries->at(i).convex_hull[i2].y = 1960;
                    }
                    if (i2 == 21)
                    {
                       countries->at(i).convex_hull[i2].x = 436;
                       countries->at(i).convex_hull[i2].y = 1948;
                    }
                    if (i2 == 22)
                    {
                       countries->at(i).convex_hull[i2].x = 364;
                       countries->at(i).convex_hull[i2].y = 1884;
                    }
                    if (i2 == 23)
                    {
                       countries->at(i).convex_hull[i2].x = 295;
                       countries->at(i).convex_hull[i2].y = 1831;
                    }
                    if (i2 == 24)
                    {
                       countries->at(i).convex_hull[i2].x = 364;
                       countries->at(i).convex_hull[i2].y = 1820;
                    }
                    if (i2 == 2)
                    {
                       countries->at(i).convex_hull[i2].x = 418;
                       countries->at(i).convex_hull[i2].y = 82;
                    }
                }

                
                //ImGui::GetForegroundDrawList()->AddRect(
                 //   screen_pos,
                //    ImVec2(screen_pos.x + 2 * animated_map_scale, screen_pos.y + 2 * animated_map_scale),
                //    ImColor(255, 255, 255)
                //);
                //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[0].font_addr, 15, screen_pos, ImColor(255, 255, 255), std::to_string(i2).c_str());
                
            }

            posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
            posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;
            secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
            secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;
            sizex = secondposx - posx;
            sizey = secondposy - posy;


            if (g_convex_math.IsPointInsidePolygon2(Point(cursor_pos.x, cursor_pos.y), converted_to_screen_convex))
            {
                *hovered_id = i;
            }

        }

        //rendering citys and country image
        for (int i = 0; i < countries->size(); i++)
        {
            auto data = countries->at(i);

            auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
            auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;
            auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
            auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;

            auto sizex = secondposx - posx;
            auto sizey = secondposy - posy;

            auto fontsize = 17 + animated_map_scale * 3;
            auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(fontsize, FLT_MAX, -1.f, data.name.c_str());

            

            if (*hovered_id != -1)
            {
                if (*hovered_id == i)
                {
                    data.color = ImColor(int(data.color.Value.x * 255) + 60, int(data.color.Value.y * 255) + 60, int(data.color.Value.z * 255) + 60);

                    auto pos = ImVec2(countries->at(i).position.x * animated_map_scale - (countries->at(i).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                        countries->at(i).position.y * animated_map_scale - (countries->at(i).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);
                }

            }
            

            data.color.Value.w = 255.f;

            ImGui::GetBackgroundDrawList()->AddImage(
                (ImTextureID)data.texture,

                ImVec2(data.position.x* animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                    data.position.y* animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale),

                ImVec2(data.position.x* animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                    data.position.y* animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale),
                ImVec2(0, 0),
                ImVec2(1, 1),
                data.color
            );


            //cities
            for (int city_id = 0; city_id < data.cities.size(); city_id++)
            {
                int alpha_for_city_text = 255 - ((5.25 - animated_map_scale) * 100);

                data.color.Value.w = float(alpha_for_city_text / 255.f);

                //ImGui::GetForegroundDrawList()->AddRect(ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale - 2 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale - 2 * animated_map_scale), ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale + 2 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale + 2 * animated_map_scale), ImColor(255, 255, 255, alpha_for_city_text));
                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - 0.5 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale - 0.5 * animated_map_scale), ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale + 0.5 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale + 0.5 * animated_map_scale), data.color);

                auto textsize_for_city = g_xgui.fonts[3].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, data.cities[city_id].name.c_str());

                ImVec2 text_pos = ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25);

                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(0.f ,0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 , posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 , posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());


                if (!countries->at(i).cities[city_id].selected)
                    ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(data.color.Value.x, data.color.Value.y, data.color.Value.z, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

                if (countries->at(i).cities[city_id].highlighted)
                {
                    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale ), 15.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                }

                process_object_selections(true, i, player_id, countries, &countries->at(i).cities[city_id], animated_map_scale, map_pos);
        
                if (countries->at(i).cities[city_id].selected != NOT_SELECTED)
                {
                    ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(1.f, 1.f, 1.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
                }

            }

        }
        
        //nuclear rockets
        {
            bool is_going_to_update_targets = false;
            if (!air_strike_targets.empty())
            {
                for (int targets = 0; targets < air_strike_targets.size(); targets++)
                {
                    if (air_strike_targets[targets].GETTER_city_id != -1)
                    {
                        if (true)
                        {
                            auto start_pos = countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos;

                            auto pos = ImVec2(countries->at(air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                                countries->at(air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);

                            ImVec2 final_pos = ImVec2(pos.x + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                            //targets
                            auto posx = countries->at(air_strike_targets[targets].GETTER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[targets].GETTER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
                            auto posy = countries->at(air_strike_targets[targets].GETTER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[targets].GETTER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;

                            auto target = ImVec2(posx + countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale);

                            //auto city_pos = ImVec2(countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale + pos.x, countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale + pos.y);
                            //auto striking_building_pos = ImVec2(final_pos.x, final_pos.y);

                            //if (function_count == 1)
                            //{
                            //    air_strike_targets[targets].distance_between_targets = CalculateDistance(city_pos, striking_building_pos) / animated_map_scale;  
                            //}
                            //int segments_num = air_strike_targets[targets].distance_between_targets;

                            //bomb sending and pos

                            if (function_count == 1)
                                air_strike_targets[targets].segments = CalculateSegments(final_pos, target, animated_map_scale);
                            
                            auto point = GetTrajectoryOnePoint(final_pos, target, air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, air_strike_targets[targets].segments);

                            air_strike_targets[targets].bomb_pos = point;

                            if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                            {
                                if (air_strike_targets[targets].step_of_bomb > air_strike_targets[targets].segments) // segments
                                {
                                    auto bomb_ptr = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                        {
                                            return target.unique_id == air_strike_targets[targets].unique_id;
                                        });
                                    air_strike_targets.erase(bomb_ptr);

                                    is_going_to_update_targets = true;
                                    continue;
                                }

                                if (air_strike_targets[targets].last_global_tick < global_tick)
                                {
                                    air_strike_targets[targets].step_of_bomb++;
                                    air_strike_targets[targets].last_global_tick = global_tick;
                                    is_going_to_update_targets = true;
                                }

                            }


                            //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(20, 20), ImColor(255, 255, 255), std::to_string(city_pos.x).c_str());
                            //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(20, 50), ImColor(255, 255, 255), std::to_string(city_pos.y).c_str());
                            //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(20, 80), ImColor(255, 255, 255), std::to_string(striking_building_pos.x).c_str());
                            //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(20, 110), ImColor(255, 255, 255), std::to_string(striking_building_pos.y).c_str());
                            //ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(20, 270), ImColor(255, 255, 255), std::to_string(air_strike_targets[targets].distance_between_targets).c_str());

                            DrawTrajectoryArc(ImGui::GetForegroundDrawList(), final_pos, ImVec2(posx + countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.x * animated_map_scale, posy + countries->at(air_strike_targets[targets].GETTER_country_id).cities[air_strike_targets[targets].GETTER_city_id].pos.y * animated_map_scale), air_strike_targets[targets].bomb_pos, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);

                            ImGui::GetForegroundDrawList()->AddCircleFilled(air_strike_targets[targets].bomb_pos, 3 * animated_map_scale, ImColor(255, 0, 0));

                        }
                    }

                    else if (air_strike_targets[targets].GETTER_building_id != -1)
                    {
                        if (true)
                        {
                            auto start_pos = countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos;

                            auto pos = ImVec2(countries->at(air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                                countries->at(air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);

                            ImVec2 final_pos = ImVec2(pos.x + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                            //bomb sending and pos

                            auto posx = countries->at(air_strike_targets[targets].GETTER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[targets].GETTER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
                            auto posy = countries->at(air_strike_targets[targets].GETTER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[targets].GETTER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;

                            auto pos_of_building = ImVec2(posx + countries->at(air_strike_targets[targets].GETTER_country_id).buildings[air_strike_targets[targets].GETTER_building_id].pos.x * animated_map_scale, posy + countries->at(air_strike_targets[targets].GETTER_country_id).buildings[air_strike_targets[targets].GETTER_building_id].pos.y * animated_map_scale);

                            if (function_count == 1)
                                air_strike_targets[targets].segments = CalculateSegments(final_pos, pos_of_building, animated_map_scale);

                            auto point = GetTrajectoryOnePoint(final_pos, pos_of_building, air_strike_targets[targets].step_of_bomb, 140.f * animated_map_scale, air_strike_targets[targets].segments);

                            air_strike_targets[targets].bomb_pos = point;


                            if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                            {
                                if (air_strike_targets[targets].step_of_bomb > air_strike_targets[targets].segments) // segments
                                {
                                    auto bomb_ptr = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                        {
                                            return target.unique_id == air_strike_targets[targets].unique_id;
                                        });
                                    air_strike_targets.erase(bomb_ptr);

                                    is_going_to_update_targets = true;
                                    continue;
                                }

                                if (air_strike_targets[targets].last_global_tick < global_tick)
                                {
                                   air_strike_targets[targets].step_of_bomb++;
                                   is_going_to_update_targets = true;

                                   if (air_strike_targets[targets].last_global_tick < global_tick + 3)
                                       air_strike_targets[targets].last_global_tick = global_tick;
                                    
                                }

                            }

                            DrawTrajectoryArc(ImGui::GetForegroundDrawList(), final_pos, pos_of_building, air_strike_targets[targets].bomb_pos, 140.f * animated_map_scale, ImColor(255, 255, 255, 210), 200);

                            ImGui::GetForegroundDrawList()->AddCircleFilled(air_strike_targets[targets].bomb_pos, 3 * animated_map_scale, ImColor(255, 0, 0));


                        }
                    }

                    else if (air_strike_targets[targets].GETTER_rocket != -1)
                    {
                        auto pointed_bomb_ptr2 = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](const nuclear_strike_target& target)
                            {
                                return target.unique_id == air_strike_targets[targets].GETTER_rocket;
                            });

                        if (pointed_bomb_ptr2->GETTER_rocket == -1)
                        {                     
                            auto start_pos = countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos;

                            auto pos = ImVec2(countries->at(air_strike_targets[targets].SENDER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                                countries->at(air_strike_targets[targets].SENDER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[targets].SENDER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);

                            ImVec2 final_pos = ImVec2(pos.x + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.x * animated_map_scale, pos.y + countries->at(air_strike_targets[targets].SENDER_country_id).buildings[air_strike_targets[targets].SENDER_building_id].pos.y * animated_map_scale);

                            auto pointed_bomb_ptr3 = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                {
                                    return target.unique_id == air_strike_targets[targets].GETTER_rocket;
                                });

                            auto bomb_pos = pointed_bomb_ptr3->bomb_pos;

                            if (function_count == 1)
                                air_strike_targets[targets].segments = CalculateSegments(final_pos, bomb_pos, animated_map_scale);

                            auto target_point = GetTrajectoryStrightOnePoint(final_pos, bomb_pos, air_strike_targets[targets].step_of_bomb, air_strike_targets[targets].segments);

                            air_strike_targets[targets].bomb_pos = ImVec2(target_point);

                            //bomb sending and pos
                            if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
                            {
                                if (air_strike_targets[targets].step_of_bomb > air_strike_targets[targets].segments) // segments
                                {

                                    if (true)
                                    {
                                        std::srand(static_cast<unsigned int>(std::time(0)));
                                        int random_number = std::rand() % 11;

                                        if (random_number % 4 == 0)
                                        {

                                            auto pointed_bomb_ptr = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                                {
                                                    return target.unique_id == air_strike_targets[targets].GETTER_rocket;
                                                });
                                            air_strike_targets.erase(pointed_bomb_ptr);


                                            auto bomb_ptr = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                                {
                                                    return target.unique_id == air_strike_targets[targets].unique_id;
                                                });
                                            air_strike_targets.erase(bomb_ptr);

                                            //ROCKET DESTROYED
                                            is_going_to_update_targets = true;
                                            continue;
                                        }
                                        else
                                        {
                                            //ROCKET DESTROYED

                                            auto bomb_ptr = std::find_if(air_strike_targets.begin(), air_strike_targets.end(), [&](nuclear_strike_target target)
                                                {
                                                    return target.unique_id == air_strike_targets[targets].unique_id;
                                                });
                                            air_strike_targets.erase(bomb_ptr);

                                            is_going_to_update_targets = true;
                                            continue;
                                        }
                                    }
                                }

                                if (air_strike_targets[targets].last_global_tick < global_tick)
                                {
                                    air_strike_targets[targets].step_of_bomb++;
                                    air_strike_targets[targets].last_global_tick = (global_tick - air_strike_targets[targets].last_global_tick) / 2;
                                    is_going_to_update_targets = true;
                                }

                            }


                             //PVO
                            {

                                DrawTrajectoryStrightArc(ImGui::GetForegroundDrawList(), final_pos, bomb_pos, air_strike_targets[targets].bomb_pos, ImColor(255, 255, 40, 150), 200);

                                ImGui::GetForegroundDrawList()->AddCircleFilled(air_strike_targets[targets].bomb_pos, 1 * animated_map_scale, ImColor(255, 190, 0));
                            }

                        }
                    }
                }
            }

            if (is_going_to_update_targets && g_socket_control.player_role == g_socket_control.player_role_enum::SERVER && function_count == 1)
            {
                g_socket_control.server_send_nuclear_targets();
            }


        }

        //buildings
        for (int i = 0; i < countries->size(); i++)
        {  

            for (int buildings_id = 0; buildings_id < countries->at(i).buildings.size(); buildings_id++)
            {
                if (!countries->at(i).buildings.at(buildings_id).name.empty())
                {
                    auto pos = ImVec2(countries->at(i).position.x * animated_map_scale - (countries->at(i).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                        countries->at(i).position.y * animated_map_scale - (countries->at(i).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);

                    ImVec2 mapped_pos = ImVec2((countries->at(i).buildings[buildings_id].pos.x - pos.x) / animated_map_scale, (countries->at(i).buildings[buildings_id].pos.y - pos.y) / animated_map_scale);

                    if (countries->at(i).buildings[buildings_id].size_converted_to_map == false)
                    {
                        countries->at(i).buildings[buildings_id].pos = mapped_pos;
                        countries->at(i).buildings[buildings_id].size_converted_to_map = true;
                    }

                    ImVec2 final_pos = ImVec2(pos.x + countries->at(i).buildings[buildings_id].pos.x * animated_map_scale, pos.y + countries->at(i).buildings[buildings_id].pos.y * animated_map_scale);

                    ImGui::GetForegroundDrawList()->AddCircle(final_pos, 5 * animated_map_scale, ImColor(255, 255, 255, 250));

                    auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, countries->at(i).buildings[buildings_id].name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(final_pos.x - textsize.x / 2, final_pos.y - 8 * animated_map_scale), ImColor(255, 255, 255), countries->at(i).buildings[buildings_id].name.c_str());

                    process_object_selections(false, i, player_id, countries, &countries->at(i).buildings[buildings_id], animated_map_scale, map_pos);
                    
                    if ((countries->at(i).buildings[buildings_id].selected != NOT_SELECTED && countries->at(i).buildings[buildings_id].building_type == MISSILE_DEFENSE && countries->at(i).buildings[buildings_id].progress_of_building == 105))
                    {
                        ImGui::GetForegroundDrawList()->AddCircle(ImVec2(final_pos), 50.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                    }

                    if (countries->at(i).buildings[buildings_id].highlighted)
                    {
                        ImGui::GetForegroundDrawList()->AddCircle(ImVec2(final_pos), 15.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                    }

                    if (i != g_menu.players[player_id].control_region)
                        continue;

                    //NEXT PART OF CODE SHOULD BE PROCESSED ONLY BY ONE FUNCTION "render_map_and_process. secondary function for double map should ignore this code"

                    //first function called
                    if (function_count == 1)
                    { 
                        //building process
                        if (countries->at(i).buildings[buildings_id].progress_of_building != 105)
                        {
                            static int old_game_tick;

                            if (old_game_tick != global_tick)
                            {
                                countries->at(i).buildings[buildings_id].progress_of_building += 10;
                                old_game_tick = global_tick;
                            }

                            ImGui::GetForegroundDrawList()->AddRect(ImVec2(final_pos.x - 7 * animated_map_scale, final_pos.y - 15 * animated_map_scale), ImVec2(final_pos.x + 7 * animated_map_scale, final_pos.y - 17 * animated_map_scale), ImColor(255, 255, 255, 250));

                            ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(final_pos.x - 7 * animated_map_scale, final_pos.y - 15 * animated_map_scale), ImVec2(final_pos.x - 7 * animated_map_scale + ((countries->at(i).buildings[buildings_id].progress_of_building / 100.f) * 14 * animated_map_scale), final_pos.y - 17 * animated_map_scale), ImColor(255, 255, 255, 250));

                        }

                        if (countries->at(i).buildings[buildings_id].progress_of_building == 100)
                        {
                            countries->at(i).buildings[buildings_id].progress_of_building = 105;

                            if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                            {
                                g_socket_control.server_send_building(buildings_id, i);
                            }
                            else
                                g_socket_control.client_send_building(buildings_id, i);
                        }

                        //BUILDING FINISHED - the main cycle of building
                        if (countries->at(i).buildings[buildings_id].progress_of_building == 105)
                        {
                            if (countries->at(i).buildings[buildings_id].selected == SOLO_SELECTED)
                            {
                                opened_menu_size = ImRect(ImVec2(0, 0), ImVec2(350, screen_y));
                                ImGui::SetNextWindowPos(ImVec2(0, 0));
                                ImGui::SetNextWindowSize(ImVec2(350, screen_y));
                                ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 210);
                                ImGui::Begin(countries->at(i).buildings[buildings_id].name.c_str());
                                {
                                    switch (countries->at(i).buildings[buildings_id].building_type)
                                    {
                                        case AIRCRAFT_FACTORY:
                                        {
                                            ImGui::Text(std::string(std::string("Current amount of jets : ") + std::to_string(g_menu.players[player_id].war_property.amount_of_jets)).c_str());

                                            ImGui::SliderInt("Fighter Jets", &countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_jets, 0, 30);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets) / 10.f;
                                                ImGui::SliderScalarForProgress("Building progress [Jets]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                            ImGui::Text(std::string(std::string("Current amount of bombers : ") + std::to_string(g_menu.players[player_id].war_property.amount_of_bombers)).c_str());

                                            ImGui::SliderInt("Bombers", &countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_bombers, 0, 15);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers) / 20.f;
                                                ImGui::SliderScalarForProgress("Building progress [Bombers]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                        }
                                        break;

                                        case SHIPYARD:
                                        {
                                            ImGui::Text(std::string(std::string("Current amount of Submarines : ") + std::to_string(g_menu.players[player_id].war_property.submarine_count)).c_str());

                                            ImGui::SliderInt("Submarines", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat1, 0, 15);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1) / 20.f;
                                                ImGui::SliderScalarForProgress("Building progress [1]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                            ImGui::Text(std::string(std::string("Current amount of Air Carriers : ") + std::to_string(g_menu.players[player_id].war_property.carrier_count)).c_str());

                                            ImGui::SliderInt("Air Carriers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat2, 0, 15);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2) / 20.f;
                                                ImGui::SliderScalarForProgress("Building progress [2]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                            ImGui::Text(std::string(std::string("Current amount of Destroyers : ") + std::to_string(g_menu.players[player_id].war_property.destroyer_count)).c_str());

                                            ImGui::SliderInt("Destroyers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat3, 0, 15);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3) / 20.f;
                                                ImGui::SliderScalarForProgress("Building progress [3]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                            ImGui::Text(std::string(std::string("Current amount of Cruisers : ") + std::to_string(g_menu.players[player_id].war_property.cruiser_count)).c_str());

                                            ImGui::SliderInt("Cruisers", &countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat4, 0, 15);

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4) / 20.f;
                                                ImGui::SliderScalarForProgress("Building progress [4]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::NewLine();

                                            ImGui::NewLine();

                                        }
                                        break;

                                        case MISSILE_SILO:
                                        {

                                            {
                                                float min = 0; float max = 1;
                                                float progress = (global_tick - countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload) / 35.f;
                                                ImGui::SliderScalarForProgress("Launch progress [1]", ImGuiDataType_Float, &progress, &min, &max);
                                            }

                                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
                                            ImGui::PushStyleColor(ImGuiCol_Text, countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot ? ImVec4(ImColor(30, 255, 47)) : ImVec4(ImColor(255, 30, 30)));

                                            ImGui::NewLine();
                                            ImGui::CenteredText(countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot ? "[ READY ]" : "[ RELOADING ]");
                                            ImGui::NewLine();
                                            ImGui::PopStyleColor();

                                            if (ImGui::Button("LAUNCH MISSILE", ImVec2(330, 45)))
                                            {
                                                //if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot)
                                                {
                                                    countries->at(i).buildings[buildings_id].selected = NOT_SELECTED;
                                                    selection_for_nuclear_strike = true;
                                                    current_striking_building_id = buildings_id;
                                                }

                                            }

                                            ImGui::NewLine();

                                            auto* vector = &countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue;
                                            {
                                                ImGui::BeginListBox("Queue", ImVec2(330, 120));
                                                {
                                                    for (int i = 0; i < vector->size(); i++)
                                                    {
                                                        if (vector->at(i).SENDER_building_id == buildings_id)
                                                        {
                                                            std::stringstream ss;

                                                            if (vector->at(i).GETTER_city_id != -1)
                                                                ss << i + 1 << ". " << countries->at(vector->at(i).GETTER_country_id).name << " - " << countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].name;
                                                            else
                                                                ss << i + 1 << ". " << countries->at(vector->at(i).GETTER_country_id).name << " - " << countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].name;

                                                            if (ImGui::BeginDragDropTarget())
                                                            {
                                                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
                                                                {
                                                                    int payload_idx = *(const int*)payload->Data;
                                                                    if (payload_idx != i)
                                                                    {
                                                                        // ���������� ������� � ������
                                                                        auto temp = vector->at(payload_idx);
                                                                        vector->erase(vector->begin() + payload_idx);
                                                                        vector->insert(vector->begin() + i, temp);
                                                                    }
                                                                }
                                                                ImGui::EndDragDropTarget();
                                                            }

                                                            ImGui::PushStyleColor(ImGuiCol_Text, i == 0 ? ImVec4(ImColor(79, 255, 69)) : ImVec4(ImColor(255, 255, 255, 110)));

                                                            ImGui::Selectable(ss.str().c_str());

                                                            if (vector->at(i).GETTER_city_id != -1)
                                                            {
                                                                if (ImGui::IsItemHovered())
                                                                {
                                                                    countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].highlighted = true;
                                                                }
                                                                else
                                                                    countries->at(vector->at(i).GETTER_country_id).cities[vector->at(i).GETTER_city_id].highlighted = false;

                                                            }
                                                            else
                                                            {
                                                                if (ImGui::IsItemHovered())
                                                                {
                                                                    countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].highlighted = true;
                                                                }
                                                                else
                                                                    countries->at(vector->at(i).GETTER_country_id).buildings[vector->at(i).GETTER_building_id].highlighted = false;
                                                            }

                                                            ImGui::PopStyleColor();
                                                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                                                ImGui::SetDragDropPayload("DND_ITEM", &i, sizeof(int));
                                                                ImGui::Text("%s", ss.str().c_str());
                                                                ImGui::EndDragDropSource();
                                                            }


   

                                                            if (ImGui::BeginDragDropTarget()) 
                                                            {
                                                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM")) 
                                                                {
                                                                    int payload_idx = *(const int*)payload->Data;
                                                                    if (payload_idx != i) 
                                                                    {
                                                                        // ���������� ������� � ������
                                                                        auto temp = vector->at(payload_idx);
                                                                        vector->erase(vector->begin() + payload_idx);
                                                                        vector->insert(vector->begin() + i, temp);
                                                                    }
                                                                }
                                                                ImGui::EndDragDropTarget();
                                                            }
                                                        }
                                                    }
                                                }
                                                ImGui::EndListBox();

                                                ImGui::Button("Delete target", ImVec2(330, 25));
                                                if (ImGui::BeginDragDropTarget())
                                                {
                                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
                                                    {
                                                        int payload_idx = *(const int*)payload->Data;
                                                        {
                                                            vector->erase(vector->begin() + payload_idx);
                                                        }
                                                    }
                                                    ImGui::EndDragDropTarget();
                                                }
                                            }

                                            ImGui::PopStyleVar();
                                        }
                                        break;

                                    }
                                    ImGui::End();
                                }
                            }

                            switch (countries->at(i).buildings[buildings_id].building_type)
                            {
                                case AIRCRAFT_FACTORY:
                                {
                                    if (countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_jets > g_menu.players[player_id].war_property.amount_of_jets)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets + 10)
                                        {
                                            countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_jets = global_tick;
                                            g_menu.players[player_id].war_property.amount_of_jets++;
                                        }
                                    }

                                    if (countries->at(i).buildings[buildings_id].air_factory_heart.goal_amount_of_bombers > g_menu.players[player_id].war_property.amount_of_bombers)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers + 20)
                                        {
                                            countries->at(i).buildings[buildings_id].air_factory_heart.old_tick_for_bombers = global_tick;
                                            g_menu.players[player_id].war_property.amount_of_bombers++;
                                        }
                                    }

                                }
                                break;

                                case SHIPYARD:
                                {
                                    if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat1 > g_menu.players[player_id].war_property.submarine_count)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1 + 20)
                                        {
                                            countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat1 = global_tick;
                                            g_menu.players[player_id].war_property.submarine_count++;
                                        }
                                    }

                                    if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat2 > g_menu.players[player_id].war_property.carrier_count)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2 + 20)
                                        {
                                            countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat2 = global_tick;
                                            g_menu.players[player_id].war_property.carrier_count++;
                                        }
                                    }

                                    if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat3 > g_menu.players[player_id].war_property.destroyer_count)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3 + 20)
                                        {
                                            countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat3 = global_tick;
                                            g_menu.players[player_id].war_property.destroyer_count++;
                                        }
                                    }

                                    if (countries->at(i).buildings[buildings_id].shipyard_heart.goal_amount_of_boat4 > g_menu.players[player_id].war_property.cruiser_count)
                                    {
                                        if (global_tick > countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4 + 20)
                                        {
                                            countries->at(i).buildings[buildings_id].shipyard_heart.old_tick_for_boat4 = global_tick;
                                            g_menu.players[player_id].war_property.cruiser_count++;
                                        }
                                    }

                                }
                                break;

                                case MISSILE_SILO :
                                {

                                    if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot && !countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.empty())
                                    {
                                            if (countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot)
                                            {
                                                air_strike_targets.push_back(countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue[0]);
                                                countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload = global_tick;
                                                countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.erase(countries->at(i).buildings[buildings_id].missile_silo_heart.strike_queue.begin());
                                                countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = false;

                                                if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                                                {
                                                    g_socket_control.server_send_nuclear_targets();
                                                }
                                                else
                                                    g_socket_control.client_send_nuclear_targets();
                                            }
                                        
                                    }

                                    if (global_tick > countries->at(i).buildings[buildings_id].missile_silo_heart.old_tick_for_reload + 35)
                                    {
                                        countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = true;
                                    }
                                    else
                                        countries->at(i).buildings[buildings_id].missile_silo_heart.ready_to_shot = false;

                                    if (selection_for_nuclear_strike)
                                    {
                                        ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 25, ImVec2(screen_x / 2, 150), ImColor(255, 255, 255), "Choose Target!");
 
                                    }
                                }
                                break;

                                case MISSILE_DEFENSE : 
                                {

                                    if (global_tick > countries->at(i).buildings[buildings_id].missile_defense_heart.old_tick_for_reload + 5)
                                    {
                                        countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = true;
                                    }
                                    else
                                        countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = false;


                                    if (countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot)
                                    {
                                        for (int rockets_id = 0; rockets_id < air_strike_targets.size(); rockets_id++)
                                        {
                                            if (air_strike_targets[rockets_id].GETTER_rocket != -1)
                                                continue;

                                            auto pos2 = ImVec2(countries->at(air_strike_targets[rockets_id].GETTER_country_id).position.x * animated_map_scale - (countries->at(air_strike_targets[rockets_id].GETTER_country_id).size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale,
                                                countries->at(air_strike_targets[rockets_id].GETTER_country_id).position.y * animated_map_scale - (countries->at(air_strike_targets[rockets_id].GETTER_country_id).size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale);

                                            auto bombpos = ImVec2(air_strike_targets[rockets_id].bomb_pos.x, air_strike_targets[rockets_id].bomb_pos.y);

                                            auto buildpos = ImVec2(final_pos.x, final_pos.y);

                                            float distance = CalculateDistance(bombpos, buildpos) / animated_map_scale;
                                            
                                            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(15, 15), ImColor(255, 255, 255), std::to_string(distance).c_str());


                                            if (distance < 50 && air_strike_targets[rockets_id].SENDER_country_id != g_menu.players[player_id].control_region)
                                            {
                                                if (countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot)
                                                {
                                                    nuclear_strike_target new_target;
                                                    new_target.unique_id = generateUniqueInt();
                                                    new_target.GETTER_country_id = air_strike_targets[rockets_id].SENDER_country_id;
                                                    new_target.GETTER_city_id = -1;
                                                    new_target.GETTER_building_id = -1;
                                                    new_target.GETTER_rocket = air_strike_targets[rockets_id].unique_id;
                                                    new_target.SENDER_country_id = g_menu.players[player_id].control_region;
                                                    new_target.SENDER_building_id = buildings_id;

                                                    air_strike_targets.push_back(new_target);
                                                    countries->at(i).buildings[buildings_id].missile_defense_heart.ready_to_shot = false;
                                                    countries->at(i).buildings[buildings_id].missile_defense_heart.old_tick_for_reload = global_tick;

                                                    if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
                                                    {
                                                        g_socket_control.server_send_nuclear_targets();
                                                    }
                                                    else
                                                        g_socket_control.client_send_nuclear_targets();
                                                }
                                            }
                                        }
                                    }
                                }
                                break;


                                
                            }
                        }
                    }
                }

            }
        }

    }
}
void map_processing::process_map(window_profiling window, int screen_size_x, int screen_size_y, int player_id)
{
    static ImVec2 final_map_pos;
    static ImVec2 moved_pos = ImVec2(-190, 241.5);

    static float  map_scale = 2;
    static float  animated_map_scale;
    
    GetCursorPos  (&cursor_pos);
    ScreenToClient(g_xgui.hwnd_of_process, &cursor_pos);

    screen_x = screen_size_x;
    screen_y = screen_size_y;

    //cursor processing
    static ImVec2 cursor_pos_screened_value;
    cursor_pos_screened_value.x = (((screen_size_x * map_scale) / 2 - (cursor_pos.x * map_scale)) / (map_scale * 2));
    cursor_pos_screened_value.y = (((screen_size_y * map_scale) / 2 - (cursor_pos.y * map_scale)) / (map_scale * 2));
    ImVec2 cursor_rel_to_map = ImVec2(
        (cursor_pos.x - moved_pos.x - screen_size_x / 2) / map_scale,
        (cursor_pos.y - moved_pos.y - screen_size_y / 2) / map_scale
    );

    //scroll +- and limting scrolling
    if (ImGui::GetIO().MouseWheel > 0)
    {
        float old_scale = map_scale;

        if (map_scale < 5.25)
        {
            map_scale += 0.25f;

            moved_pos.x -= 70 / map_scale;
            moved_pos.y -= 20 / map_scale;

        }
        else
            map_scale = 5.25;

    }
    if (ImGui::GetIO().MouseWheel < 0)
    {
        if (map_scale > 2)
        {
            map_scale -= 0.25f;

        }
        else
            map_scale = 2;
    }

    //animation of scroll
    animated_map_scale = ImGui::LerpAnimate("Map_Move", "Scale", true, map_scale, 15, ImGui::INTERP);

    //limiting map movement engine
    static float limits_x[2]; static float limits_y[2];
    if (map_scale == 1.25)
    {
        limits_x[0] = -50;   limits_x[1] = -218;
        limits_y[0] = 241;   limits_y[1] = 241;
    }
    else
    {
        limits_x[0] = -138 + 17 * map_scale;   limits_x[1] = -370 - 175 * map_scale;
        limits_y[0] = 326 + 17 * map_scale;   limits_y[1] = -18 - 160 * map_scale;
    }

    //moving map
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        moved_pos.y += 11.5 / (map_scale / 2);
    }

    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        moved_pos.x += 11.5 / (map_scale / 2);
    }

    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        moved_pos.y -= 11.5 / (map_scale / 2);
    }

    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        moved_pos.x -= 11.5 / (map_scale / 2);
    }


    //mouse3 map movement
    static POINT old_cursor_pos;
    static bool MiddleMouseMove;

    static bool store_cursor_pos;
    store_cursor_pos = !store_cursor_pos;

    if (store_cursor_pos)
        old_cursor_pos = cursor_pos;

    if (ImGui::IsMouseDown(2)) {

        {
            MiddleMouseMove = true;
        }
    }

    if (MiddleMouseMove)
    {
        moved_pos.x -= (old_cursor_pos.x - cursor_pos.x) / (map_scale / 2) * 2;
        moved_pos.y -= (old_cursor_pos.y - cursor_pos.y) / (map_scale / 2) * 2;
    }
    if (ImGui::IsMouseReleased(2)) {
        MiddleMouseMove = false;
        old_cursor_pos.x = 0;
        old_cursor_pos.y = 0;
    }


    //limiting moving map
    if (moved_pos.y > limits_y[0])
        moved_pos.y = limits_y[0];

    if (moved_pos.y < limits_y[1])
        moved_pos.y = limits_y[1];



    //animating map moving
    static float animated_map_pos_x; animated_map_pos_x = ImGui::LerpAnimate("Map_Move", "PositionX", true, moved_pos.x, 14, ImGui::INTERP);
    static float animated_map_pos_y; animated_map_pos_y = ImGui::LerpAnimate("Map_Move", "PositionY", true, moved_pos.y, 14, ImGui::INTERP);
    final_map_pos = ImVec2(animated_map_pos_x, animated_map_pos_y);


    //city data
    static const ImVec2 offset = ImVec2(500, 1);
    static std::vector< city > city_data[19];

    city_data[window.countries_name::USA] =
    {
        { "Barrow", ImVec2(42, 201) },
        { "Nome", ImVec2(15, 261) },
        { "Fairbanks", ImVec2(72, 257) },
        { "Bethel", ImVec2(24, 291) },
        { "Anchorage", ImVec2(65, 286) },
        { "Whitehorse", ImVec2(113, 286) },
        { "Juneau", ImVec2(119, 312) },
        { "Yellowknife", ImVec2(191.031555, 277.857788) },
        { "Churchill", ImVec2(259.339233, 305.550110) },
        { "Nain", ImVec2(369.185455, 312.627045) },
        { "Vancouver", ImVec2(148.569977, 356.319336) },
        { "Calgary", ImVec2(181.185364, 349.550110) },
        { "Edmonton", ImVec2(195.031525, 329.550110) },
        { "Saskatoon", ImVec2(219.954605, 341.242401) },
        { "Regina", ImVec2(223.193100, 353.199554) },
        { "Winnipeg", ImVec2(251.339233, 355.703949) },//320.866608, 377.498627
        { "Ottawa", ImVec2(320.866608, 377.498627) },
        { "Quebec", ImVec2(332.558929, 367.960144) },
        { "Toronto", ImVec2(306.712738, 388.883240) },
        { "Charlottetown", ImVec2(360.251251, 375.344757) },
        { "Seattle", ImVec2(156.335510, 368.888214) },
        { "Helena", ImVec2(186.049805, 373.173950) },
        { "San Jose", ImVec2(162.621231, 417.459686) },
        { "Los Angeles", ImVec2(174.049805, 429.745392) },
        { "Las Vegas", ImVec2(183.764084, 419.745392) },
        { "Bismarck", ImVec2(238.621246, 374.316803) },
        { "Oklahoma City", ImVec2(252.049820, 420.888245) },
        { "Dallas", ImVec2(257.520111, 436.963287) },
        { "Houston", ImVec2(258.696564, 448.727997) },
        { "New York", ImVec2(324.587738, 397.249176) },
        { "Washington D.C", ImVec2(319.921051, 408.915833) },
        { "Atlanta", ImVec2(292.587708, 432.249176) },
        { "Mexico City", ImVec2(241.254333, 483.915894) },
        { "Monterrey", ImVec2(231.254318, 460.915863) },
        { "Merida", ImVec2(274.751282, 482.769775) },
        { "Santa Fe", ImVec2(215.197632, 413.709595) },
    };

    city_data[window.countries_name::LatinUSA] =
    {
        { "Managua", ImVec2(22.737667, 54.650902) },
        { "Tegucigalpa", ImVec2(18.407806, 46.787113) },

    };

    city_data[window.countries_name::SouthernUSA] =
    {
        { "Caracas", ImVec2(150.090164, 8.688351) },
        { "Bogota", ImVec2(123.423492, 30.288361) },
        { "Trujillo", ImVec2(105.290154, 69.755043) },
        { "Lima", ImVec2(115.423492, 88.688385) },
        { "Antofagasta", ImVec2(135.690170, 127.088402) },
        { "Puerto Montt", ImVec2(126.090126, 199.678741) },
        { "Sucre", ImVec2(152.756805, 107.945358) },
        { "Rio Gallegos", ImVec2(138.356796, 248.478760) },
        { "Santiago", ImVec2(132.756790, 168.478714) },
        { "Buenos Aires", ImVec2(176.490143, 173.812057) },
        { "Asuncion", ImVec2(180.490158, 131.678711) },
        { "Porto Alegre", ImVec2(200.490158, 149.812469) },
        { "Brasillia", ImVec2(210.890167, 101.012444) },
        { "Salvador", ImVec2(242.623520, 88.745773) },
        { "Paramaribo", ImVec2(187.423492, 25.279079) },
        { "Manaus", ImVec2(176.756821, 60.212429) },
        { "Caxias", ImVec2(228.756836, 63.412430) },
        { "Campo Grande", ImVec2(187.690140, 114.345711) },
        { "Ji-Parana", ImVec2(165.556808, 81.279037) },

    };

    city_data[window.countries_name::EC] =
    {
        { "Reykjavik", ImVec2(180.253571, 278.350311) },
        { "Dublin", ImVec2(228.253098, 353.550262) },
        { "London", ImVec2(251.719772, 362.083588) },
        { "Glasgow", ImVec2(237.853104, 337.550262) },
        { "Paris", ImVec2(261.586761, 375.950256) },
        { "Hague", ImVec2(269.586761, 359.950256) },
        { "Berlin", ImVec2(295.453461, 359.950256) },
        { "Hamburg", ImVec2(287.986786, 349.816925) },
        { "Nuremberg", ImVec2(132.756790, 168.478714) },
        { "Madrid", ImVec2(240.520081, 419.950287) },
        { "Lisbon", ImVec2(221.853409, 425.016968) },
        { "Rome", ImVec2(294.653442, 409.550293) },
        { "Marseille", ImVec2(268.855408, 403.945953) },
        { "Bern", ImVec2(281.805145, 387.941315) },
        { "Palermo", ImVec2(298.578674, 429.731445) },
    };

    city_data[window.countries_name::Russia] =
    {
        { "Moscow", ImVec2(75.428047, 295.3076781) },
        { "Kazan", ImVec2(113.246201, 295.307678) },
        { "Samara", ImVec2(113.110039, 305.011139) },
        { "Volgograd", ImVec2(84.882584, 328.580383) },
        { "Ekaterinburg", ImVec2(145.190887, 280.201813) },
        { "Saint-Petersburg", ImVec2(37.580566, 263.599365) },
        { "Voronezh", ImVec2(63.462933, 314.893494) },
        { "Omsk", ImVec2(194.756790, 294.422913) },
        { "Novosibirsk", ImVec2(221.109756, 297.011139) },
        { "Krasnoyarsk", ImVec2(255.227402, 283.364075) },
        { "Stavropol", ImVec2(78.802628, 350.706665) },
        { "Yaroslavl", ImVec2(78.555107, 277.425079) },
        { "Irkutsk", ImVec2(294.303864, 311.079010) },
        { "Yakutsk", ImVec2(373.672241, 263.079041) },
        { "Chelyabinsk", ImVec2(150.591156, 293.271027) },
        { "Chita", ImVec2(324.235443, 304.329803) }, //55.768299, 283.754547
        { "Tver", ImVec2(55.768299, 283.754547) },
        { "Vologda", ImVec2(80.625450, 262.040253) },
        { "Kirov", ImVec2(116.339752, 265.754547) },
        { "Petrozavodsk", ImVec2(51.196869, 245.754532) },
        { "Nizhny Tagil", ImVec2(145.768341, 265.183105) }, //95.010284, 311.695160
        { "Saratov", ImVec2(95.010284, 311.695160) },
        { "Vorkuta", ImVec2(151.801376, 201.475372) },
        { "Vladivostok", ImVec2(393.651520, 354.551025) },
        { "Grozniy", ImVec2(89.959358, 356.455750) },

    };

    city_data[window.countries_name::Northeurope] =
    {
        { "Helsinki", ImVec2(117.252304, 230.247894) },
        { "Tallinn", ImVec2(117.252304, 236.247894) },
        { "Vaasa", ImVec2(106.752304, 209.747894) },
        { "Oslo", ImVec2(69.252289, 230.247894) },
        { "Stockholm", ImVec2(93.752289, 236.747910) },
        { "Sundsvall", ImVec2(91.164513, 214.858780) },
        { "Lulea", ImVec2(104.764526, 191.458786) },
    };

    city_data[window.countries_name::EastEC] =
    {
        { "Warsaw", ImVec2(30.130413, 18.204439) },
        { "Krakow", ImVec2(25.178030, 27.156826) },
        { "Bratislava", ImVec2(21.939932, 36.299686) },
        { "Budapest", ImVec2(23.273266, 44.109219) },
        { "Sofia", ImVec2(40.416134, 63.918755) },
        { "Athens", ImVec2(44.797089, 86.204483) },
        { "Bucharest", ImVec2(48.987568, 54.775890) },
    };

    city_data[window.countries_name::east_europe] =
    {
        { "Minsk", ImVec2(21.125013, 15.978427) },
        { "Kyiv", ImVec2(29.505972, 32.168915) },
        { "Kharkiv", ImVec2(47.029793, 36.359394) },
        { "Zhytomyr", ImVec2(16.721037, 33.881676) },
        { "Chisinau", ImVec2(23.029776, 50.073689) },
    };

    city_data[window.countries_name::Turk] =
    {
        { "Ankara", ImVec2(28.300053, 12.651188) },
        { "Damascus", ImVec2(40.300060, 36.270248) },
        { "Baghdad", ImVec2(61.823879, 38.555965) },
        { "Kirkuk", ImVec2(61.633404, 27.889292) },
        { "Samsun", ImVec2(37.252438, 4.270231) },
        { "Konya", ImVec2(22.966717, 18.746429) },
    };

    city_data[window.countries_name::North_WellWellWell] =
    {
        { "Riyadh", ImVec2(223.580673, 55.771702) },
        { "Aden", ImVec2(215.135040, 91.993950) },
        { "Cairo", ImVec2(165.801666, 32.660572) },
        { "Khartoum", ImVec2(169.357224, 78.438385) },
        { "N'Djamena", ImVec2(114.377556, 94.006775) },
        { "Zinder", ImVec2(91.789337, 90.242073) },
        { "Niamey", ImVec2(69.436409, 91.418541) },
        { "Abuja", ImVec2(86.994431, 107.771454) },
        { "Yamoussoukro", ImVec2(41.244473, 113.521454) },
        { "Dakar", ImVec2(7.244502, 65.771515) },
        { "Rabat", ImVec2(35.994480, 20.521545) },
        { "Bamako", ImVec2(35.994480, 92.771492) },
        { "Algiers", ImVec2(70.994453, 7.521555) },
        { "Tripoli", ImVec2(105.744431, 21.771545) },
    };

    city_data[window.countries_name::MidWellWellWell] =
    {
        { "Malabo", ImVec2(4.715996, 38.075298) },
        { "Bangui", ImVec2(32.480724, 34.781178) },
        { "Libreville", ImVec2(6.363057, 53.369427) },
        { "Juba", ImVec2(81.186646, 29.839996) },
        { "Addis Ababa", ImVec2(105.657249, 22.310579) },
    };

    city_data[window.countries_name::South_WellWellWell] =
    {
        { "Kinshasa", ImVec2(23.277018, 24.023441) },
        { "Bumba", ImVec2(36.218204, 17.199907) },
        { "Kananga", ImVec2(37.865265, 39.082279) },
        { "Kampala", ImVec2(72.453529, 18.141085) },
        { "Nairobi", ImVec2(89.159424, 23.552853) },
        { "Dodoma", ImVec2(80.191437, 41.762150) },
        { "Lusaka", ImVec2(55.691429, 72.012161) },
        { "Harare", ImVec2(64.941437, 82.012161) },
        { "Maputo", ImVec2(78.191437, 102.512177) },
        { "Bloemfontein", ImVec2(46.191425, 121.012184) },
        { "Upington", ImVec2(33.691422, 118.512177) },
        { "Cape Town", ImVec2(23.941416, 142.762192) },
        { "Windhoek", ImVec2(18.191414, 97.012169) },
    };

    city_data[window.countries_name::Indostan] =
    {
        { "Muscat", ImVec2(49.238419, 65.549530) },
        { "Tehran", ImVec2(27.764719, 16.917917) },
        { "Yazd", ImVec2(43.343678, 38.812668) },
        { "Bamian", ImVec2(81.448250, 22.602131) },
        { "Kandahar", ImVec2(71.974556, 35.233719) },
        { "Islamabad", ImVec2(101.027206, 27.444239) },
        { "Buhawalpur", ImVec2(90.290359, 43.654778) },
        { "New Delhi", ImVec2(116.185112, 50.391624) },
        { "Chandigarh", ImVec2(112.185112, 40.286354) },
        { "Rajkot", ImVec2(91.553520, 71.865326) },
        { "Solapur", ImVec2(108.816689, 85.760071) },
        { "Salem", ImVec2(116.185112, 105.128502) },
        { "Madurai", ImVec2(117.869324, 110.391663) },
        { "Dhaka", ImVec2(159.512756, 65.459679) },
        { "Naypyidaw", ImVec2(178.979431, 80.926346) },
        { "Bhilai", ImVec2(125.359772, 73.526421) },
        { "Bhubaneswar", ImVec2(142.833466, 78.368530) },
    };

    city_data[window.countries_name::Zakavkazie] =
    {
        { "Yerevan", ImVec2(17.043537, 14.748335) }
    };

    city_data[window.countries_name::Churki] =
    {
        { "Astana", ImVec2(86.299988, 22.282675) },
        { "Atyrau", ImVec2(20.888172, 45.576813) },
        { "Kyzylorda", ImVec2(58.064671, 58.517998) },
        { "Bishkek", ImVec2(96.417641, 67.929771) },
        { "Tashkent", ImVec2(78.064690, 75.459190) },
        { "Samarkand", ImVec2(66.535263, 80.870956) },
        { "Ashgabat", ImVec2(39.005833, 84.635666) },
        { "Nukus", ImVec2(46.702759, 68.027840) },
        { "Dushanbe", ImVec2(81.996902, 85.439621) },
    };

    city_data[window.countries_name::China] =
    {
        { "Ulaanbaatar", ImVec2(118.749847, 36.044868) },
        { "Erdenet", ImVec2(103.321190, 22.901987) },
        { "Beijing", ImVec2(146.900284, 69.679459) },
        { "Zibo", ImVec2(153.566956, 80.568359) },
        { "Shanghai", ImVec2(161.344727, 101.012810) },
        { "Hong Kong", ImVec2(139.541885, 135.640305) },
        { "Kunming", ImVec2(97.941864, 128.240295) },
        { "Wuhan", ImVec2(137.541885, 103.840279) },
        { "Chengdu", ImVec2(99.541862, 104.240280) },
        { "Guilin", ImVec2(121.646561, 120.686592) },
        { "Xi'an", ImVec2(116.313225, 93.067520) },
        { "Taiyuan", ImVec2(125.456085, 76.877037) },
        { "Baotou", ImVec2(121.075134, 66.019890) },
        { "Golmud", ImVec2(84.884636, 87.924660) },
        { "Hotan", ImVec2(27.551264, 81.638947) },
        { "Xining", ImVec2(88.884636, 80.115135) },
        { "Korla", ImVec2(47.668518, 59.457573) },
        { "Fuzhou", ImVec2(156.193161, 122.709167) },
    };

    city_data[window.countries_name::Indo_China] =
    {
        { "Hanoi", ImVec2(28.221739, 10.504876) },
        { "Vientiane", ImVec2(19.650307, 21.171547) },
        { "Bangkok", ImVec2(12.602684, 35.838223) },
        { "Phnom Penh", ImVec2(26.126499, 41.933460) },
        { "Pleiku", ImVec2(38.507458, 34.695515) }
    };

    city_data[window.countries_name::Samurai] =
    {
        { "Kuala Lumpur", ImVec2(24.590548, 166.036514) },
        { "Medan", ImVec2(18.695816, 173.404922) },
        { "Jakarta", ImVec2(41.643169, 199.089127) },
        { "Merauke", ImVec2(153.432571, 202.247009) },
        { "Jayapura", ImVec2(156.317337, 186.457550) },
        { "Port Moresby", ImVec2(180.379623, 206.668060) },
        { "Lae", ImVec2(177.642776, 199.089127) },
        { "Hanoi", ImVec2(28.221739, 10.504876) },
        { "Seoul", ImVec2(112.457100, 39.075222) },
        { "Tokyo", ImVec2(153.409515, 44.789509) },
        { "Hiroshima", ImVec2(129.219025, 48.979988) },
        { "Miyazaki", ImVec2(123.885681, 60.979996) },
        { "Asahikawa", ImVec2(163.884384, 8.789462) },
        { "Sendai", ImVec2(158.641373, 32.061432) },
        { "Bandar Seri Begawan", ImVec2(68.164940, 161.013672) },
        { "Bontang", ImVec2(75.783989, 174.918350) },
        { "Banjarmasin", ImVec2(64.736366, 187.680252) },
        { "Pontianak", ImVec2(48.736355, 174.346909) },
        { "General Santos", ImVec2(103.451828, 152.861115) },
        { "Manila", ImVec2(89.940163, 126.277924) },
        { "Osaka", ImVec2(141.703568, 47.655811) },
        { "Singapore", ImVec2(32.180634, 172.988953) },
    };

    city_data[window.countries_name::Austrilia] =
    {
        { "Canberra", ImVec2(124.383881, 98.622498) },
        { "Melbourne", ImVec2(105.494980, 106.844727) },
        { "Perth", ImVec2(10.383797, 79.289146) },
        { "Darwin", ImVec2(62.828281, 10.844653) },
        { "Townsville", ImVec2(114.828316, 31.066891) },
        { "Brisbane", ImVec2(138.161667, 65.066917) },
        { "Hanoi", ImVec2(28.221739, 10.504876) },
        { "Alice Springs", ImVec2(73.050507, 57.955799) },
        { "Broken Hill", ImVec2(99.272850, 83.511391) },
        { "Mount Isa", ImVec2(92.828400, 34.844696) },
        { "Rockhampton", ImVec2(128.383972, 45.955814) },
        { "Griffth", ImVec2(111.272858, 89.733620) },
        { "Auckland", ImVec2(213.210007, 99.786263) },
        { "Christchurch", ImVec2(199.432220, 132.452957) }
    };

    int color_offset = 15;
    if (countries.empty())
    countries =
    {
        { "North-America",  window.countries[window.countries_name::USA],                   ImVec2(-48.6336 + offset.x, 53.9273 + offset.y)   ,  ImVec2(1683, 2111), ImVec2(0.195, 0.123),       ImColor(96, 151, 223),   city_data[window.countries_name::USA],              },
        { "EU",             window.countries[window.countries_name::EC],                    ImVec2(241.1947 + offset.x, 22.4489 + offset.y)   ,  ImVec2(1320, 1969), ImVec2(0.18, 0.23),         ImColor(133, 205, 212),   city_data[window.countries_name::EC]                },
        { "North-Europe",   window.countries[window.countries_name::Northeurope],           ImVec2(377.7431 + offset.x, 0.8408 + offset.y)   ,   ImVec2(620, 1131),  ImVec2(0.1818, 0.1306),     ImColor(133, 205, 212),   city_data[window.countries_name::Northeurope]       },
        { "Australia",      window.countries[window.countries_name::Austrilia],             ImVec2(849.2101 + offset.x, 488.0281 + offset.y)   , ImVec2(1028, 805),  ImVec2(0.1206, 0.188),      ImColor(57, 57, 57),   city_data[window.countries_name::Austrilia]         },
        { "Russia",         window.countries[window.countries_name::Russia],                ImVec2(698.6149 + offset.x, 17.3418 + offset.y)   ,  ImVec2(2467, 1538), ImVec2(0.1442, 0.1806),     ImColor(130, 213, 87),   city_data[window.countries_name::Russia]            },
        { "China",          window.countries[window.countries_name::China],                 ImVec2(696.0981 + offset.x, 215.8585 + offset.y)   , ImVec2(886, 651),   ImVec2(0.2062, 0.153),      ImColor(221, 197, 86),   city_data[window.countries_name::China]             },
        { "Central Asia",   window.countries[window.countries_name::Churki],                ImVec2(567.2284 + offset.x, 177.5464 + offset.y)   , ImVec2(591, 423),   ImVec2(0.136, 0.1976),      ImColor(57, 57, 57),   city_data[window.countries_name::Churki]            },
        { "East EU",        window.countries[window.countries_name::EastEC],                ImVec2(416.0617 + offset.x, 180.4763 + offset.y)   , ImVec2(330, 420),   ImVec2(0.206, 0.2302),      ImColor(57, 57, 57),   city_data[window.countries_name::EastEC]            },
        { "East Europe",    window.countries[window.countries_name::east_europe],           ImVec2(443.122 + offset.x, 154.4864 + offset.y)   ,  ImVec2(263, 269),   ImVec2(0.1234, 0.126201),   ImColor(57, 57, 57),   city_data[window.countries_name::east_europe]       },
        { "IndoChina",      window.countries[window.countries_name::Indo_China],            ImVec2(693.4327 + offset.x, 306.3944 + offset.y)   , ImVec2(178, 267),   ImVec2(0.1648, 0.1238),     ImColor(57, 57, 57),   city_data[window.countries_name::Indo_China]        },
        { "Indostan",       window.countries[window.countries_name::Indostan],              ImVec2(586.6926 + offset.x, 272.443 + offset.y)   ,  ImVec2(828, 540),   ImVec2(0.1954, 0.1272),     ImColor(57, 57, 57),   city_data[window.countries_name::Indostan]          },
        { "Latin-USA",      window.countries[window.countries_name::LatinUSA],              ImVec2(70.8135 + offset.x, 296.821 + offset.y)   ,   ImVec2(461, 302),   ImVec2(0.216, 0.141),       ImColor(57, 57, 57),   city_data[window.countries_name::LatinUSA]          },
        { "North-Africa",   window.countries[window.countries_name::North_WellWellWell],    ImVec2(401.2982 + offset.x, 280.6873 + offset.y)   , ImVec2(1060, 520),  ImVec2(0.124, 0.120),       ImColor(163, 169, 96),   city_data[window.countries_name::North_WellWellWell]},
        { "East-Asia",      window.countries[window.countries_name::Samurai],               ImVec2(794.3515 + offset.x, 304.7909 + offset.y)   , ImVec2(1081, 1035), ImVec2(0.126, 0.1211),      ImColor(128, 111, 62),   city_data[window.countries_name::Samurai]           },
        { "South-America",  window.countries[window.countries_name::SouthernUSA],           ImVec2(100.3651 + offset.x, 460.8606 + offset.y)   , ImVec2(1203, 1225), ImVec2(0.14, 0.142),        ImColor(57, 57, 57),   city_data[window.countries_name::SouthernUSA]       },
        { "Mid-Africa",     window.countries[window.countries_name::MidWellWellWell],       ImVec2(438.9074 + offset.x, 340.2161 + offset.y)   , ImVec2(623, 291),   ImVec2(0.146, 0.1374),      ImColor(57, 57, 57),   city_data[window.countries_name::MidWellWellWell]   },
        { "South-Africa",   window.countries[window.countries_name::South_WellWellWell],    ImVec2(443.0285 + offset.x, 439.819 + offset.y)   ,  ImVec2(563, 849),   ImVec2(0.1316, 0.1986),     ImColor(57, 57, 57),   city_data[window.countries_name::South_WellWellWell]},
        { "Turkey",         window.countries[window.countries_name::Turk],                  ImVec2(463.8263 + offset.x, 226.1221 + offset.y)   , ImVec2(335, 243),   ImVec2(0.1596, 0.2266),     ImColor(57, 57, 57),   city_data[window.countries_name::Turk]              },
        { "Zakavkazie",     window.countries[window.countries_name::Zakavkazie],            ImVec2(491.7546 + offset.x, 202.2194 + offset.y)   , ImVec2(154, 101),   ImVec2(0.1432, 0.1956),     ImColor(57, 57, 57),   city_data[window.countries_name::Zakavkazie]        }
    };

    int hovered_country_id = -1;

    if (final_map_pos.x > 1025)
    {
        ImGui::ChangeAnimateValue("Map_Move", "PositionX", final_map_pos.x - 1250);
        animated_map_pos_x = final_map_pos.x - 1250;
        moved_pos.x = moved_pos.x - 1250;
        final_map_pos.x = final_map_pos.x - 1250;
    }
    if (final_map_pos.x < -1494)
    {
        ImGui::ChangeAnimateValue("Map_Move", "PositionX", final_map_pos.x + 1250);
        animated_map_pos_x = final_map_pos.x + 1250;
        moved_pos.x = moved_pos.x + 1250;
        final_map_pos.x = final_map_pos.x + 1250;
    }


    //map processing

    this->render_map_and_process_hitboxes(window, &countries, animated_map_scale, &hovered_country_id, ImVec2(cursor_pos.x, cursor_pos.y), final_map_pos, player_id, 1);
    //first map

    ImVec2 final_pos_secondary = final_map_pos;
    if (final_map_pos.x < -225)
    {
    
        final_pos_secondary.x += 1250;
    }
    else {
        final_pos_secondary.x -= 1250;
    }

   this->render_map_and_process_hitboxes(window, &countries, animated_map_scale, &hovered_country_id, ImVec2(cursor_pos.x, cursor_pos.y), final_pos_secondary, player_id, 2);

    process_and_sync_game_cycle(&countries, player_id, animated_map_scale, hovered_country_id);

}

//game tick and event processing
void process_events()
{
    static int game_length;

    if (g_menu.selected_game_mode == 0)
    {
        game_length = 30;
    }
    if (g_menu.selected_game_mode == 1)
    {
        game_length = 45;
    }

    //events switching
    if (g_map.global_tick > ((game_length / 5) * 60) * 0)       { g_map.game_events = g_map.game_events::PREPARATION_EVENT; }
    if (g_map.global_tick > ((game_length / 5) * 60) * 1)       { g_map.game_events = g_map.game_events::DOCKYARD_RELEASE ; }
    if (g_map.global_tick > ((game_length / 5) * 60) * 2)       { g_map.game_events = g_map.game_events::AIRCRAFR_RELEASE ; }
    if (g_map.global_tick > ((game_length / 5) * 60) * 3)       { g_map.game_events = g_map.game_events::NUCLEAR_DANGER   ; }
    if (g_map.global_tick > ((game_length / 5) * 60) * 4)       { g_map.game_events = g_map.game_events::ANIHILATION      ; }
    if (g_map.global_tick > ((game_length / 5) * 60) * 5)       { g_map.game_events = g_map.game_events::GAME_END         ; }

}

void game_event_timer()
{
    while (true)
    {
        process_events();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); //TODO : change it to 1 second in release version
        g_map.global_tick++;
    }
}

std::string convert_tick_to_timer()
{
    static int game_length;

    if (g_menu.selected_game_mode == 0)
    {
        game_length = 30;
    }
    if (g_menu.selected_game_mode == 1)
    {
        game_length = 45;
    }

    int seconds;
    int minutes;

    minutes = ((((game_length / 5) * (g_map.game_events + 1)) * 60) - g_map.global_tick) / 60;
    minutes = std::clamp(minutes, 0, 5);
    
    seconds = ((((game_length / 5) * (g_map.game_events + 1)) * 60) - g_map.global_tick - (minutes * 60));
    seconds = std::clamp(seconds, 0, 60);

    std::string minutes_string = std::to_string(minutes);
    if (minutes_string.length() < 2)
    {
        minutes_string.insert(minutes_string.begin(), '0');
    }
    std::string seconds_string = std::to_string(seconds);
    if (seconds_string.length() < 2)
    {
        seconds_string.insert(seconds_string.begin(), '0');
    }

    std::string time = minutes_string + ":" + seconds_string;

    return time;
}


char* format_currency(long long amount) {
    static char buffer[50];
    char temp[50];
    sprintf_s(temp,sizeof(temp), "%lld", amount);

    int len = strlen(temp);
    int dot_pos = len % 3;
    if (dot_pos == 0) dot_pos = 3;

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (i != 0 && (i % 3 == dot_pos % 3)) {
            buffer[j++] = '.';
        }
        buffer[j++] = temp[i];
    }
    buffer[j] = '\0';

    return buffer;
}


//game processing
void map_processing::process_and_sync_game_cycle(std::vector <country_data>* countries, int player_id, float animated_map_scale, int hovered_country_id)
{
    if (!tick_started)
    {
        std::thread(game_event_timer).detach();
        tick_started = true;
    }

    if (g_socket_control.player_role        ==      g_socket_control.player_role_enum::SERVER)
    {
        //server side 

        //tick and event update
        static int old_global_tick = 0;
        static int old_game_event  = 0;

        //sending global game tick to client
        if (old_global_tick != global_tick)         { g_socket_control.server_send_message("GAME_CYCLE:TICK_UPDATE:"         + std::to_string(global_tick));        old_global_tick = global_tick;        }

        //sending global game event to client
        if (old_game_event  != g_map.game_events)   { g_socket_control.server_send_message("GAME_CYCLE:GLOBAL_EVENT_UPDATE:" + std::to_string(g_map.game_events));  old_game_event = g_map.game_events;   }
    }
    else if (g_socket_control.player_role   ==      g_socket_control.player_role_enum::CLIENT)
    {
        //client side

        std::string message = g_socket_control.game_cycle_messages;
        if (g_socket_control.game_cycle_messages.find("GAME_CYCLE:TICK_UPDATE:") != std::string::npos)
        {
            //game tick update from server and making it up in global var
            message           .erase(0, 23);
            global_tick = std::stoi(message);
        }
        else if (g_socket_control.game_cycle_messages.find("GAME_CYCLE:GLOBAL_EVENT_UPDATE:") != std::string::npos)
        {
            //game event update from server and making it up in global var
            message.erase(0, 31);
            game_events = std::stoi(message);
        }
        
    }

    //ecomics cycle
    static int old_global_tick_for_economy;
    static float capital_goal_for_animation = g_menu.players[player_id].economics.capital;
    if (global_tick % 5 == 0 && old_global_tick_for_economy != global_tick)
    {
        old_global_tick_for_economy = global_tick;
        capital_goal_for_animation += g_menu.players[player_id].economics.capital_inflow * (g_menu.players[player_id].economics.capital_inflow_ratio / 100);
    }

    float progress = (capital_goal_for_animation - g_menu.players[player_id].economics.capital) / 10;


    if (progress > 1)
        g_menu.players[player_id].economics.capital += progress;
    else
        g_menu.players[player_id].economics.capital = capital_goal_for_animation;

    bool is_server = g_socket_control.player_role == g_socket_control.player_role_enum::SERVER;

    //timer visual
    {
        auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, convert_tick_to_timer().c_str());
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x / 2 - 30, 30 - 5), ImVec2(screen_x / 2 + 30, 30 + 23), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x / 2 - 30, 30 - 5), ImVec2(screen_x / 2 + 30, 30 + 23), ImColor(79, 255, 69, 130));
        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x / 2 - textsize.x / 2, 30), ImColor(255, 255, 255), convert_tick_to_timer().c_str());
    }

    //timer visual
    {
        static const char* stage_names[] =
        {
            "Preparation time",
            "Mobilisation",
            "Mobilisation : 2",
            "Nuclear Danger",
            "Anihiliation",
            "The End",
        };

        auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, stage_names[game_events]);
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x / 2 - textsize.x, 75 - textsize.y / 2), ImVec2(screen_x / 2 + textsize.x, 75 + textsize.y), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x / 2 - textsize.x, 75 - textsize.y / 2), ImVec2(screen_x / 2 + textsize.x, 75 + textsize.y), ImColor(79, 255, 69, 130));
        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x / 2 - textsize.x / 2, 70), ImColor(255, 255, 255), stage_names[game_events]);
    }

    //playerlist
    {
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(screen_x - 210, 19), ImVec2(screen_x - 25, 25 + (25 * g_menu.players.size())), ImColor(0, 0, 0, 90));
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(screen_x - 210, 19), ImVec2(screen_x - 25, 25 + (25 * g_menu.players.size())), ImColor(79, 255, 69, 130));

        for (int i = 0; i < g_menu.players.size(); i++)
        {
            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, 17.f, ImVec2(screen_x - 200, 25 + (25 * i)), ImColor(255, 255, 255), g_menu.players[i].name.c_str());
        }
    }

    static int last_playerlist_pos_y = 50 + (25 * g_menu.players.size());

    //building ui
    {
        static int opened_tab_right_ui;
        ImGui::SetNextWindowPos(ImVec2(screen_x - 220, last_playerlist_pos_y));
        ImGui::SetNextWindowSize(ImVec2(screen_x - 25, last_playerlist_pos_y + 500));
        ImGui::Begin("Building Window", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove 
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDecoration);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
            {
                    {
                        ImGui::PushFont(g_xgui.fonts[2].font_addr);
                        {
                            ImGui::ButtonEx(std::string(format_currency(int(g_menu.players[player_id].economics.capital)) + std::string("$")).c_str(), ImVec2(160, 40), ImGuiButtonFlags_NoInput);
                        }
                        ImGui::PopFont();

                        ImGui::PushFont(g_xgui.fonts[3].font_addr);
                        {
                            ImGui::NewLine();

                            if (ImGui::Button("Building mode", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 1)
                                    opened_tab_right_ui = 0;
                                else
                                opened_tab_right_ui = 1;
                            }
                            ImGui::NewLine();

                            if (ImGui::Button("Diplomacy", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 2)
                                    opened_tab_right_ui = 0;
                                else
                                    opened_tab_right_ui = 2;
                            }
                            ImGui::NewLine();

                            if (ImGui::Button("Economics", ImVec2(160, 55)))
                            {
                                if (opened_tab_right_ui == 3)
                                    opened_tab_right_ui = 0;
                                else
                                    opened_tab_right_ui = 3;
                            }
                            ImGui::NewLine();
                        }
                }
                ImGui::PopFont();
            }
            ImGui::PopStyleVar();
        }

        ImGui::End();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            opened_tab_right_ui = 0;
        }

        static int should_build_building_id = 0;

        switch (opened_tab_right_ui)
        {
        case 1:
        {
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 - 400, screen_y - 270));
            ImGui::SetNextWindowSize(ImVec2(800, 400));
            ImGui::Begin("Building", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDecoration);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
                {
                    ImGui::PushFont(g_xgui.fonts[3].font_addr);
                    {
                        if (ImGui::Button("AIR", ImVec2(250, 25))) {}                                                          ImGui::SameLine(); if (ImGui::Button("WATER", ImVec2(250, 25))) {}                                                          ImGui::SameLine(); if (ImGui::Button("MISSILE", ImVec2(250, 25))) {}
                        if (ImGui::Button("Aircraft Factory [ 25.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::AIRCRAFT_FACTORY; } ImGui::SameLine(); if (ImGui::Button("Shipyard [ 20.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::SHIPYARD; }         ImGui::SameLine(); if (ImGui::Button("Missile Defense [ 9.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::MISSILE_DEFENSE; }
                        if (ImGui::Button("Field Airstrip [ 10.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::FIELD_AIR_STRIP; }  ImGui::SameLine(); if (ImGui::Button(" ", ImVec2(250, 50))) {}                                                          ImGui::SameLine(); if (ImGui::Button("Missile Silo [ 13.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::MISSILE_SILO; }
                        if (ImGui::Button("Permanent Airfield [ 20.000.000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::PERMANENT_AIRFIELD; }ImGui::SameLine(); if (ImGui::Button("   ", ImVec2(250, 50))) {}                                                          ImGui::SameLine(); if (ImGui::Button("Radar [ 7 000 000 ]", ImVec2(250, 50))) { should_build_building_id = buildings::RADAR; }
                    }
                    ImGui::PopFont();
                }
                ImGui::PopStyleVar();
            }
            ImGui::End();
        }
        break;

        }

        if (should_build_building_id != 0)
        {
            static int price;
            static int animaton_for_no_money_text;

            switch (should_build_building_id)
            {
                case AIRCRAFT_FACTORY:      { price = 25000000; } break;
                case FIELD_AIR_STRIP:       { price = 10000000; } break;
                case PERMANENT_AIRFIELD:    { price = 20000000; } break;
                case SHIPYARD:              { price = 20000000; } break;
                case MISSILE_DEFENSE:       { price = 9000000;  } break;
                case MISSILE_SILO:          { price = 13000000; } break;
                case RADAR:                 { price = 7000000;  } break;
            }

            if (true)
            {

                if (hovered_country_id == g_menu.players[player_id].control_region)
                {
                    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), 5 * animated_map_scale, ImColor(255, 0, 0, 250));

                    if (should_build_building_id == MISSILE_DEFENSE)
                    {
                        ImGui::GetForegroundDrawList()->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), 50.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
                    }

                    if (ImGui::IsMouseDown(0))
                    {
                        ImVec4 pixel_color;
                        g_convex_math.GetScreenPixelWithGDI(cursor_pos.x, cursor_pos.y, pixel_color);

                        bool color_check;
                        if (should_build_building_id == SHIPYARD)
                            color_check = pixel_color.y < 0.09;
                        else
                            color_check = pixel_color.x > 0.03 && pixel_color.y > 0.09 && pixel_color.z > 0.03;

                        if (g_menu.players[player_id].economics.capital >= price && color_check && cursor_pos.x > 100 && cursor_pos.y > 100 && cursor_pos.x < screen_x - 100 && cursor_pos.y < screen_y - 100)
                        {
                            opened_tab_right_ui = 0;
                            capital_goal_for_animation = g_menu.players[player_id].economics.capital - price;
                            building new_build;
                            new_build.building_type = should_build_building_id;
                            new_build.pos = ImVec2(cursor_pos.x, cursor_pos.y);
                            new_build.progress_of_building = 0;
                            new_build.endurance = 0;
                            new_build.hovered = false;
                            new_build.selected = NOT_SELECTED;

                            switch (new_build.building_type)
                            {
                                case AIRCRAFT_FACTORY:      { new_build.name = "Aircraft factory"; } break;
                                case FIELD_AIR_STRIP:       { new_build.name = "Field Airstrip"; } break;
                                case PERMANENT_AIRFIELD:    { new_build.name = "Permanent Airfield"; } break;
                                case SHIPYARD:              { new_build.name = "Shipyard"; } break;
                                case MISSILE_DEFENSE:       { new_build.name = "Missile Defense"; } break;
                                case MISSILE_SILO:          { new_build.name = "Missile Silo"; } break;
                                case RADAR:                 { new_build.name = "Radar"; } break;
                            }

                            countries->at(g_menu.players[player_id].control_region).buildings.push_back(new_build);

                            should_build_building_id = 0;
                        }
                        else
                        {
                            opened_tab_right_ui = 0;
                            should_build_building_id = 0;
                        }
                    }
                }
            }

        }
    }

    //selector
    {
        bool should_selector = true;
        if (g_map.cursor_pos.x > g_map.opened_menu_size.Min.x && g_map.cursor_pos.x < g_map.opened_menu_size.Max.x)
        {
            if (g_map.cursor_pos.y > g_map.opened_menu_size.Min.y && g_map.cursor_pos.y < g_map.opened_menu_size.Max.y)
            {
                should_selector = false;
            }
        }
        if (should_selector)
        {
            static int old_cursor_pos_x, old_cursor_pos_y;
            if (ImGui::IsMouseDown(0))
            {
                if (old_cursor_pos_x == 0 && old_cursor_pos_y == 0)
                {
                    old_cursor_pos_x = cursor_pos.x;
                    old_cursor_pos_y = cursor_pos.y;
                }

                if (old_cursor_pos_x != cursor_pos.x && old_cursor_pos_y != cursor_pos.y)
                {
                    selector_zone = ImRect(ImVec2(old_cursor_pos_x, old_cursor_pos_y), ImVec2(cursor_pos.x, cursor_pos.y));
                    ImGui::GetForegroundDrawList()->AddRect(selector_zone.Min, selector_zone.Max, ImColor(79, 255, 69, 130));
                }
            }
            else if (ImGui::IsMouseReleased(0))
            {
                selector_zone = ImRect(ImVec2(0, 0), ImVec2(0, 0));
                old_cursor_pos_x = 0;
                old_cursor_pos_y = 0;
            }
        }
    }
}
