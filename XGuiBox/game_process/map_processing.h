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
        if (val == 0) return 0;  // Точки коллинеарны
        return (val > 0) ? 1 : 2;  // 1 - по часовой стрелке, 2 - против
    }

    std::vector<Point> GetConvexHull(std::vector<Point>& points) {
        int n = points.size();
        if (n < 3) return points;

        std::vector<Point> hull;

        int leftmost = 0;
        for (int i = 1; i < n; i++) {
            if (points[i].x < points[leftmost].x) {
                leftmost = i;
            }
        }

        int p = leftmost, q;
        do {
            hull.push_back(points[p]);


            q = (p + 1) % n;
            for (int i = 0; i < n; i++) {
                if (orientation(points[p], points[i], points[q]) == 2) {
                    q = i;
                }
            }

            p = q;

        } while (p != leftmost);

        return hull;
    }

    std::vector<Point> InterpolateConvexHullPoints(const std::vector<Point>& hull, float interpolation_factor) {
        std::vector<Point> interpolated_points;
        int n = hull.size();
        if (n < 2) return hull;

        for (int i = 0; i < n; i++) {
            Point p1 = hull[i];
            Point p2 = hull[(i + 1) % n];

            interpolated_points.push_back(p1);

            for (float t = interpolation_factor; t < 1.0f; t += interpolation_factor) {
                float x = p1.x + t * (p2.x - p1.x);
                float y = p1.y + t * (p2.y - p1.y);
                interpolated_points.push_back(Point(x, y));
            }
        }

        return interpolated_points;
    }

    const unsigned char* GetTexturePixels(IDirect3DTexture9* texture, IDirect3DDevice9* device, int& width, int& height) {
        IDirect3DDevice9* pDevice = device;
        IDirect3DTexture9* pTexture = texture;

        D3DSURFACE_DESC desc;
        pTexture->GetLevelDesc(0, &desc);
        width = desc.Width;
        height = desc.Height;

        D3DLOCKED_RECT lockedRect;
        pTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_READONLY);

        unsigned char* pixels = new unsigned char[lockedRect.Pitch * height];

        memcpy(pixels, lockedRect.pBits, lockedRect.Pitch * height);

        pTexture->UnlockRect(0);

        return pixels;
    }

    std::vector<Point> GetFilledPixels(const unsigned char* texture_pixels, int width, int height, int* step2) {
        std::vector<Point> filled_pixels;
        const int min_points = 2500;

        int step = ((1) > (static_cast<int>(sqrt((width * height) / static_cast<float>(min_points))))) ? (1) : (static_cast<int>(sqrt((width * height) / static_cast<float>(min_points))));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                if (x % step != 0 || y % step != 0)
                    continue;

                int idx = (y * width + x) * 4;
                unsigned char r = texture_pixels[idx + 0];
                unsigned char g = texture_pixels[idx + 1];
                unsigned char b = texture_pixels[idx + 2];
                unsigned char a = texture_pixels[idx + 3];

                if (a == 255) {
                    filled_pixels.push_back(Point(x, y));
                }
            }
        }

        *step2 = step;
        return filled_pixels;
    }

    std::vector<Point> GetConvexHullWithMorePoints(std::vector<Point>& points, float interpolation_factor) {

        std::vector<Point> hull = GetConvexHull(points);

        return InterpolateConvexHullPoints(hull, interpolation_factor);
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

struct city_vector_data
{
    std::string city_name;
    ImVec2 city_pos;

    bool selected;
    bool hovered;
};

struct country_data
{
    std::string name;
    IDirect3DTexture9* texture;

    ImVec2 position;
    ImVec2 size;
    ImVec2 hitbox_size;

    ImColor color;

    std::vector<city_vector_data> cities;

    bool hitbox_get = false;
    std::vector<Point> convex_hull = {};
    std::vector<ImVec2> convex_hull_screen_coords;

};

class map_processing
{
public:

    //tech vars
    int screen_x, screen_y;

    POINT  cursor_pos;

    //selector
    ImRect selector_zone;

    //cycles
    void process_and_sync_game_cycle();

    void process_map(window_profiling window, int screen_size_x, int screen_size_y, int player_id);

    void render_map_and_process_hitboxes(window_profiling window, std::vector <country_data>* countries, float animated_map_scale, int* hovered_id, ImVec2 cursor_pos, ImVec2 map_pos, int player_id);

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

