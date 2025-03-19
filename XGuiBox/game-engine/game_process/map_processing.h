#pragma once
#include "../resources/imgui_resource/imgui.h"

#include "../xguibox/xgui.h"
#include "../resources/imgui_resource/imgui_internal.h"
#include "../main/menu.h"
#include "../resources/window_profiling/window.h"
#include <cmath>
#include <limits>
#include <iostream> 
#include <algorithm> 
#include <fstream>

#include "structs/country_struct.h"
#include "units/units_class.h"


#define WIN32_LEAN_AND_MEAN
#ifndef YOUR_HEADER_FILE_H 
#define YOUR_HEADER_FILE_H
#endif  //YOUR_HEADER_FILE_H


class NavigableArea {
public:

    // Сохраняем точки в файл
    void SaveNavigableAreaToFile(const std::vector<ImVec2>& points, const std::string& filename) {
        std::ofstream out_file(filename);
        if (!out_file.is_open()) {
            std::cerr << "Не удалось открыть файл для записи." << std::endl;
            return;
        }

        for (const auto& point : points) {
            out_file << point.x << " " << point.y << "\n";  // Записываем координаты точки
        }
        out_file.close();
    }

    // Загружаем точки из файла
    bool LoadNavigableAreaFromFile(std::vector<ImVec2>& points, const std::string& filename) {
        std::ifstream in_file(filename);
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть файл для чтения." << std::endl;
            return false;
        }

        points.clear();  // Очищаем текущие данные

        float x, y;
        while (in_file >> x >> y) {
            points.push_back({ x, y });  // Восстанавливаем точку
        }

        in_file.close();
        return true;
    }

    std::vector<ImVec2> points;  // Точки, которые образуют проходимую область

    // Функция округления до ближайшего кратного 10
    ImVec2 SnapToGrid(const ImVec2& point) {
        float snappedX = round(point.x / 10.0f) * 10.0f;
        float snappedY = round(point.y / 10.0f) * 10.0f;
        return ImVec2(snappedX, snappedY);
    }

    // Добавление точки, если она уникальна
    void AddPoint(ImVec2 pos) {
        ImVec2 snapped_pos = SnapToGrid(pos);
        if (!ContainsPoint(snapped_pos)) {
            points.push_back(snapped_pos);
        }
    }

    // Функция для рисования области на экране
    void Draw(ImVec2 map_pos, float animated_map_scale, ImVec2 cursor) {
        if (ImGui::IsMouseDown(0)) {
            ImVec2 cursor_pos = ImVec2(cursor.x, cursor.y - 500);
            AddPoint(cursor_pos);
        }

        // Отрисовка сетки
        for (const auto& p : points) {
            ImVec2 pos = ImVec2(map_pos.x * animated_map_scale + p.x * animated_map_scale,
                map_pos.y * animated_map_scale + p.y * animated_map_scale);
            ImGui::GetForegroundDrawList()->AddRectFilled(
                ImVec2(pos.x, pos.y),
                ImVec2(pos.x + 10 * animated_map_scale, pos.y + 10 * animated_map_scale),
                IM_COL32(255, 0, 0, 255), 2.0f
            );
        }
    }

    // Добавьте этот метод в класс NavigableArea
    bool IsPathValid(const std::vector<ImVec2>& path, ImVec2 map_pos, float animated_map_scale) {
        // Если путь пуст или состоит из одной точки, можно считать его невалидным или валидным в зависимости от требований.
        if (path.size() < 2)
            return false;

        // Шаг дискретизации вдоль каждого сегмента (в пикселях)
        const float sampleStep = 1.0f;

        // Проходим по всем сегментам пути
        for (size_t i = 0; i < path.size() - 1; ++i) {
            ImVec2 start = path[i];
            ImVec2 end = path[i + 1];
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            float segmentLength = std::sqrt(dx * dx + dy * dy);

            // Определяем количество шагов вдоль сегмента
            int steps = (((1) > (static_cast<int>(segmentLength / sampleStep))) ? (1) : (static_cast<int>(segmentLength / sampleStep)));

            // Проходим с дискретизацией по сегменту
            for (int step = 0; step <= steps; ++step) {
                float t = static_cast<float>(step) / steps;
                // Вычисляем промежуточную точку пути (в экранных координатах)
                ImVec2 sample = ImVec2(start.x + dx * t, start.y + dy * t);

                bool sampleOnNavigable = false;

                // Проверяем, попадает ли данная точка в какой-либо из прямоугольников навигационной области
                // Каждый прямоугольник соответствует сохранённой точке и имеет размер 10 * animated_map_scale
                for (const auto& cell : points) {
                    // Определяем позицию прямоугольника на экране
                    ImVec2 cellTopLeft = ImVec2((map_pos.x + cell.x) * animated_map_scale,
                        (map_pos.y + cell.y) * animated_map_scale);
                    float cellSize = 10 * animated_map_scale;
                    ImVec2 cellBottomRight = ImVec2(cellTopLeft.x + cellSize, cellTopLeft.y + cellSize);

                    // Если точка sample лежит внутри прямоугольника, то переходим к следующему шагу
                    if (sample.x >= cellTopLeft.x && sample.x <= cellBottomRight.x &&
                        sample.y >= cellTopLeft.y && sample.y <= cellBottomRight.y) {
                        sampleOnNavigable = true;
                        break;
                    }
                }

                // Если хотя бы одна из промежуточных точек не попадает ни в один прямоугольник, путь не валиден
                if (!sampleOnNavigable)
                    return false;
            }
        }
        return true;
    }


    // Сохранение и загрузка
    void SaveToFile(const std::string& filename) {
        SaveNavigableAreaToFile(points, filename);
    }

    bool LoadFromFile(const std::string& filename) {
        return LoadNavigableAreaFromFile(points, filename);
    }

private:
    bool ContainsPoint(const ImVec2& p) const {
        return std::find_if(points.begin(), points.end(), [&](const ImVec2& point) {
            return fabs(point.x - p.x) < 0.1f && fabs(point.y - p.y) < 0.1f;
            }) != points.end();
    }

};

class map_processing
{
public:


    //map
    std::vector < country_data > countries;
    float map_scale2 = 0.240010;
    bool drag_n_drop;

    //tech vars
    int screen_x, screen_y;

    POINT  cursor_pos;

    //selector
    ImRect selector_zone;

    //menus
    ImRect opened_menu_size;
    int last_iteration_was_with_function_number = 0;

    bool selection_for_nuclear_strike;
    int current_striking_building_id; // BUILDING THAT STRIKES
    int current_striking_boat_id = -1;
    std::vector < nuclear_strike_target > air_strike_targets;
    
    //units
    std::vector < units_base > units;

    //cycles
    void process_and_sync_game_cycle(std::vector <country_data>* countries, int player_id, float animated_map_scale, int hovered_country_id);

    void process_object_selections(int function_count, bool city, int current_country, int player_id, std::vector <country_data>* countries, map_objects* object, float animated_map_scale, ImVec2 map_pos);

    void process_unit_selections(units_base* unit, ImVec2 pos, float animated_map_scale);

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

