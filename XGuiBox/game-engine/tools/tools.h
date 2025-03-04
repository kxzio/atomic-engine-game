#pragma once
#include <vector>
#include <string>
#include "../game_process/map_processing.h"
#include <sstream>
#include <random>
#include "../main/menu.h"

class tools
{
public:

    //time
    char* format_currency(long long amount) {
        static char buffer[50];
        char temp[50];
        sprintf_s(temp, sizeof(temp), "%lld", amount);

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

    std::string convert_tick_to_timer(int tick_duration_ms = 15)
    {
        static int game_length;

        if (g_menu.selected_game_mode == 0)
        {
            game_length = 30;
        }
        else if (g_menu.selected_game_mode == 1)
        {
            game_length = 45;
        }

        int total_game_time_sec = ((game_length / 5) * (g_map.game_events + 1)) * 60;
        int elapsed_time_sec = (g_map.global_tick * tick_duration_ms) / 1000;
        int remaining_time_sec = (((total_game_time_sec - elapsed_time_sec) > (0)) ? (total_game_time_sec - elapsed_time_sec) : (0));

        int minutes = remaining_time_sec / 60;
        int seconds = remaining_time_sec % 60;

        std::string minutes_string = (minutes < 10 ? "0" : "") + std::to_string(minutes);
        std::string seconds_string = (seconds < 10 ? "0" : "") + std::to_string(seconds);

        return minutes_string + ":" + seconds_string;
    }

    //main tools
    void read_convex_file(const std::string& input_filename, std::vector<country_data>& countries)
    {
        std::ifstream file(input_filename);
        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл." << std::endl;
            return;
        }

        std::string line;
        country_data current_country;
        while (std::getline(file, line)) {
            // Пропускаем пустые строки
            if (line.empty()) {
                continue;
            }

            // Если это имя страны, создаём новый объект country_data
            if (line.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
                if (!current_country.name.empty()) {
                    // Сохраняем предыдущую страну
                    countries.push_back(current_country);
                }
                current_country = country_data();  // Создаём новый объект для следующей страны
                current_country.name = line;  // Имя страны
            }
            else {
                // Чтение координат
                std::istringstream ss(line);
                float x, y;
                char comma;
                if (ss >> x >> comma >> y) {
                    current_country.convex_hull.push_back(Point(x, y));  // Добавляем точку
                }
            }
        }

        // Добавляем последнюю страну
        if (!current_country.name.empty()) {
            countries.push_back(current_country);
        }

        file.close();
    }

    int generate_unique_int()
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

    bool is_point_in_rect(const ImVec2& point, const ImRect& rect) {
        ImVec2 min = rect.Min;
        ImVec2 max = rect.Max;

        // Корректируем мин и макс в случае отрицательных размеров
        if (min.x > max.x) std::swap(min.x, max.x);
        if (min.y > max.y) std::swap(min.y, max.y);

        return (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y);
    }


    // trajectories
    float calculate_segments(ImVec2 start, ImVec2 end, float animated_map_scale)
    {
        float distance = (std::sqrt((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y))) / animated_map_scale;

        return distance * 4;
    }

    std::vector<ImVec2> get_trajectory_points(ImVec2 start, ImVec2 end, float height, int segments = 20)
    {
        std::vector<ImVec2> points;

        // Количество сегментов для отрисовки дуги
        const int num_segments = segments;

        // Вычисление точек дуги
        for (int i = 0; i <= num_segments; ++i) {
            float t = (float)i / (float)num_segments;
            float x = start.x + t * (end.x - start.x);
            float y = start.y + t * (end.y - start.y) - height * sin(t * IM_PI);
            points.push_back(ImVec2(x, y));
        }

        return points;
    }

    ImVec2 get_trajectory_one_point(ImVec2 start, ImVec2 end, int id, float height, int segments)
    {

        int num_segments = segments;

        // Вычисляем позицию точки на дугообразной траектории
        float t = (float)id / (float)num_segments;
        float x = start.x + t * (end.x - start.x);
        float y = start.y + t * (end.y - start.y) - height * sin(t * IM_PI); // Дуга через синус


        return ImVec2(x, y);
    }

    std::vector<ImVec2> get_trajectory_stright_points(ImVec2 start, ImVec2 end, int s) {

        int num_segments = 200;
        std::vector<ImVec2> points;

        // Вычисление точек на прямой линии     
        for (int i = 0; i <= num_segments; ++i) {
            float t = (float)i / (float)num_segments;
            float x = start.x + t * (end.x - start.x);
            float y = start.y + t * (end.y - start.y);
            points.push_back(ImVec2(x, y));
        }

        return points;
    }

    ImVec2 get_trajectory_stright_one_point(ImVec2 start, ImVec2 end, int id, int segments)
    {
        int num_segments = segments;

        float t = (float)id / (float)num_segments;
        float x = start.x + t * (end.x - start.x);
        float y = start.y + t * (end.y - start.y);

        return ImVec2(x, y);
    }

    float calculate_distance(ImVec2 point, ImVec2 circle_center) {

        return sqrt((pow(circle_center.x - point.x, 2) + pow(circle_center.y - point.y, 2)));
    }

    void draw_trajectory_arc(ImDrawList* draw_list, ImVec2 start, ImVec2 end, ImVec2 bombpos, float height, ImU32 color, float segments) {

        auto points = get_trajectory_points(start, end, height, segments);
        // Отрисовка дуги

        bool direction_right;

        if (start.x < end.x)
            direction_right = true;
        else
            direction_right = false;

        // Находим точку, до которой дошла цель
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

    void draw_trajectory_stright_arc(ImDrawList* draw_list, ImVec2 start, ImVec2 end, ImVec2 bombpos, ImU32 color, float segments) {

        auto points = get_trajectory_stright_points(start, end, segments);
        // Отрисовка дуги

        bool direction_right;

        if (start.x < end.x)
            direction_right = true;
        else
            direction_right = false;

        // Находим точку, до которой дошла цель
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

    //updating map
    void update_cursor_and_screen_size(int screen_x, int screen_y)
    {
        GetCursorPos(&g_map.cursor_pos);
        ScreenToClient(g_xgui.hwnd_of_process, &g_map.cursor_pos);

        g_map.screen_x = screen_x;
        g_map.screen_y = screen_y;

    }

    void process_mouse3_scroll(ImVec2* moved_pos, float* map_scale, float* animated_map_scale)
    {
        //scroll +- and limting scrolling
        if (ImGui::GetIO().MouseWheel > 0)
        {
            float old_scale = *map_scale;

            if (*map_scale < 5.25)
            {
                *map_scale += 0.25f;

                moved_pos->x -= 70 / *map_scale;
                moved_pos->y -= 20 / *map_scale;

            }
            else
                *map_scale = 5.25;

        }
        if (ImGui::GetIO().MouseWheel < 0)
        {
            if (*map_scale > 2)
            {
                *map_scale -= 0.25f;

            }
            else
                *map_scale = 2;
        }

        //animation of scroll
        *animated_map_scale = ImGui::LerpAnimate("Map_Move", "Scale", true, *map_scale, 15, ImGui::INTERP);

        //limiting map movement engine
        static float limits_x[2]; static float limits_y[2];
        if (*map_scale == 1.25)
        {
            limits_x[0] = -50;   limits_x[1] = -218;
            limits_y[0] = 241;   limits_y[1] = 241;
        }
        else
        {
            limits_x[0] = -138 + 17 * *map_scale;   limits_x[1] = -370 - 175 * *map_scale;
            limits_y[0] = 326 + 17 * *map_scale;   limits_y[1] = -18 - 160 * *map_scale;
        }

        //limiting moving map
        if (moved_pos->y > limits_y[0])
            moved_pos->y = limits_y[0];

        if (moved_pos->y < limits_y[1])
            moved_pos->y = limits_y[1];
    }

    void move_map(ImVec2* moved_pos, float map_scale)
    {
        //moving map
        if (ImGui::IsKeyDown(ImGuiKey_W))
        {
            moved_pos->y += 11.5 / (map_scale / 2);
        }

        if (ImGui::IsKeyDown(ImGuiKey_A))
        {
            moved_pos->x += 11.5 / (map_scale / 2);
        }

        if (ImGui::IsKeyDown(ImGuiKey_S))
        {
            moved_pos->y -= 11.5 / (map_scale / 2);
        }

        if (ImGui::IsKeyDown(ImGuiKey_D))
        {
            moved_pos->x -= 11.5 / (map_scale / 2);
        }
    }


    void process_mouse3_movement(ImVec2* moved_pos, float map_scale)
    {
        static POINT old_cursor_pos;
        static bool MiddleMouseMove;

        static bool store_cursor_pos;
        store_cursor_pos = !store_cursor_pos;

        if (store_cursor_pos)
            old_cursor_pos = g_map.cursor_pos;

        if (ImGui::IsMouseDown(2)) {

            {
                MiddleMouseMove = true;
            }
        }

        if (MiddleMouseMove)
        {
            moved_pos->x -= (old_cursor_pos.x - g_map.cursor_pos.x) / (map_scale / 2) * 2;
            moved_pos->y -= (old_cursor_pos.y - g_map.cursor_pos.y) / (map_scale / 2) * 2;
        }
        if (ImGui::IsMouseReleased(2)) {
            MiddleMouseMove = false;
            old_cursor_pos.x = 0;
            old_cursor_pos.y = 0;
        }
    }

};
inline tools g_tools;

