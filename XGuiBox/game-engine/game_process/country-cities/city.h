#pragma once
#include "../../resources/imgui_resource/imgui.h"
#include "../map_processing.h"
#include "../../tools/tools.h"

class city_processing
{
public:

    void update_hitboxes(std::vector <country_data>* countries)
    {
        static bool read;

        if (!read)
        {
            static std::vector<country_data> temp;
            g_tools.read_convex_file("convex_hull.data", temp);
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
    }

    void process_country_hitboxes(int index, window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
    {
        auto data = countries->at(index);
        static float map_scale2 = 0.240010;
        auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + map_pos.x * animated_map_scale;
        auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + map_pos.y * animated_map_scale;
        auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) + map_pos.x * animated_map_scale;
        auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) + map_pos.y * animated_map_scale;

        auto sizex = secondposx - posx;
        auto sizey = secondposy - posy;

        int texture_size_x = data.size.x;  int texture_size_y = data.size.y;

        if (!countries->at(index).hitbox_get)
        {

            if (index == window.countries_name::Indostan)
            {
                int start_index = 19;
                int end_index = 36;

                if (end_index < countries->at(index).convex_hull.size())
                {
                    countries->at(index).convex_hull.erase(countries->at(index).convex_hull.begin() + start_index, countries->at(index).convex_hull.begin() + end_index + 1);
                }
            }
            Point mouse_point = { static_cast<int>(cursor_pos.x), static_cast<int>(cursor_pos.y) };
            countries->at(index).hitbox_get = true;
        }


        float scale = animated_map_scale;

        std::vector <Point> converted_to_screen_convex;

        for (int i2 = 0; i2 < countries->at(index).convex_hull.size(); i2++)
        {
            Point pt = countries->at(index).convex_hull[i2];
            ImVec2 screen_pos = ImVec2(posx + pt.x * data.hitbox_size.x * scale, posy + pt.y * data.hitbox_size.y * scale);
            converted_to_screen_convex.push_back(Point(screen_pos.x, screen_pos.y));

            if (index == window.countries_name::Russia)
            {
                if (i2 == 0)
                {
                    countries->at(index).convex_hull[i2].x = 199;
                    countries->at(index).convex_hull[i2].y = 1589;
                }

            }
            if (index == window.countries_name::EastEC)
            {
                if (i2 == 0)
                {
                    countries->at(index).convex_hull[i2].x = 622; countries->at(index).convex_hull[i2].y = 2033;
                }
                if (i2 == 1)
                {
                    countries->at(index).convex_hull[i2].x = 34; countries->at(index).convex_hull[i2].y = 78;
                }
            }
            if (index == window.countries_name::Austrilia)
            {
                if (i2 == 0)
                {
                    countries->at(index).convex_hull[i2].x = 202;
                    countries->at(index).convex_hull[i2].y = 174;
                }
            }
            if (index == window.countries_name::Samurai)
            {
                if (i2 == 0)
                {
                    countries->at(index).convex_hull[i2].x = 178;
                    countries->at(index).convex_hull[i2].y = 1316;
                }
                if (i2 == 1)
                {
                    countries->at(index).convex_hull[i2].x = 812;
                    countries->at(index).convex_hull[i2].y = 684;
                }
            }
            if (index == window.countries_name::EC)
            {
                if (i2 == 0)
                {
                    countries->at(index).convex_hull[i2].x = 254; countries->at(index).convex_hull[i2].y = 1030;
                }
                if (i2 == 1)
                {
                    countries->at(index).convex_hull[i2].x = 60; countries->at(index).convex_hull[i2].y = 388;
                }
                if (i2 == 2)
                {
                    countries->at(index).convex_hull[i2].x = 158; countries->at(index).convex_hull[i2].y = 268;
                }
                if (i2 == 3)
                {
                    countries->at(index).convex_hull[i2].x = 238; countries->at(index).convex_hull[i2].y = 180;
                }
                if (i2 == 4)
                {
                    countries->at(index).convex_hull[i2].x = 304; countries->at(index).convex_hull[i2].y = 148;
                }
                if (i2 == 5)
                {
                    countries->at(index).convex_hull[i2].x = 384; countries->at(index).convex_hull[i2].y = 128;
                }
                if (i2 == 6)
                {
                    countries->at(index).convex_hull[i2].x = 436; countries->at(index).convex_hull[i2].y = 90;
                }
                if (i2 == 7)
                {
                    countries->at(index).convex_hull[i2].x = 538; countries->at(index).convex_hull[i2].y = 42;
                }
                if (i2 == 8)
                {
                    countries->at(index).convex_hull[i2].x = 634; countries->at(index).convex_hull[i2].y = 16;
                }


            }
            if (index == window.countries_name::Northeurope)
            {
                if (i2 == 20)
                {
                    countries->at(index).convex_hull[i2].x = 532;
                    countries->at(index).convex_hull[i2].y = 1960;
                }
                if (i2 == 21)
                {
                    countries->at(index).convex_hull[i2].x = 436;
                    countries->at(index).convex_hull[i2].y = 1948;
                }
                if (i2 == 22)
                {
                    countries->at(index).convex_hull[i2].x = 364;
                    countries->at(index).convex_hull[i2].y = 1884;
                }
                if (i2 == 23)
                {
                    countries->at(index).convex_hull[i2].x = 295;
                    countries->at(index).convex_hull[i2].y = 1831;
                }
                if (i2 == 24)
                {
                    countries->at(index).convex_hull[i2].x = 364;
                    countries->at(index).convex_hull[i2].y = 1820;
                }
                if (i2 == 2)
                {
                    countries->at(index).convex_hull[i2].x = 418;
                    countries->at(index).convex_hull[i2].y = 82;
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
            *hovered_id = index;
        }
    }

	void process_country_and_cities(int index, window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id, int function_count)
	{
        auto data = countries->at(index);

        auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale;
        auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale;
        auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale;
        auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale;

        auto sizex = secondposx - posx;
        auto sizey = secondposy - posy;

        auto fontsize = 17 + animated_map_scale * 3;
        auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(fontsize, FLT_MAX, -1.f, data.name.c_str());


        if (*hovered_id != -1)
        {
            if (*hovered_id == index)
            {
                data.color = ImColor(int(data.color.Value.x * 255) + 50, int(data.color.Value.y * 255) + 50, int(data.color.Value.z * 255) + 50);

                auto pos = ImVec2(countries->at(index).position.x * animated_map_scale - (countries->at(index).size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                    countries->at(index).position.y * animated_map_scale - (countries->at(index).size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale);
            }

        }


        data.color.Value.w = 255.f;

        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)data.texture,

            ImVec2(data.position.x * animated_map_scale - (data.size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                data.position.y * animated_map_scale - (data.size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale),

            ImVec2(data.position.x * animated_map_scale + (data.size.x * animated_map_scale * g_map.map_scale2) / 2 + map_pos.x * animated_map_scale,
                data.position.y * animated_map_scale + (data.size.y * animated_map_scale * g_map.map_scale2) / 2 + map_pos.y * animated_map_scale),
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
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - 1 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale - 1 * animated_map_scale), ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale + 1 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale + 1 * animated_map_scale), ImColor(0, 0, 0, alpha_for_city_text));
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - 0.5 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale - 0.5 * animated_map_scale), ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale + 0.5 * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale + 0.5 * animated_map_scale), data.color);

            auto textsize_for_city = g_xgui.fonts[3].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, data.cities[city_id].name.c_str());

            ImVec2 text_pos = ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25);

            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 - (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 + (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2 + (0.5 * animated_map_scale), posy + data.cities[city_id].pos.y * animated_map_scale - 25 - (0.5 * animated_map_scale)), ImColor(0.f, 0.f, 0.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());


            if (!countries->at(index).cities[city_id].selected)
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(data.color.Value.x, data.color.Value.y, data.color.Value.z, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());

            if (countries->at(index).cities[city_id].highlighted)
            {
                ImGui::GetForegroundDrawList()->AddCircle(ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale, posy + data.cities[city_id].pos.y * animated_map_scale), 15.f * animated_map_scale, ImColor(255, 255, 0), 0, 2);
            }

            g_map.process_object_selections(function_count, true, index, player_id, countries, &countries->at(index).cities[city_id], animated_map_scale, map_pos);

            if (countries->at(index).cities[city_id].selected != NOT_SELECTED)
            {
                ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].pos.y * animated_map_scale - 25), ImColor(1.f, 1.f, 1.f, alpha_for_city_text / 255.f), data.cities[city_id].name.c_str());
            }

        }
	}

};
inline city_processing g_city_processing;