#include <D3D11.h>
#include <string>
#include "../../resources/imgui_resource/imgui.h"
#include <vector>

#include "city_struct.h"
#include "building_struct.h"

// Convex hull struct for country_hitbox
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

    bool GetScreenPixelWithGDI(int x, int y, ImVec4& outColor)
    {
        HDC hdcScreen = GetDC(nullptr); // Дескриптор экрана
        if (!hdcScreen) return false;

        COLORREF color = GetPixel(hdcScreen, x, y); // Читаем пиксель
        ReleaseDC(nullptr, hdcScreen);             // Освобождаем дескриптор

        if (color == CLR_INVALID) return false;

        // Конвертация COLORREF в RGBA
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

//Main struct
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


    //boat map dragging;
    int what_type_of_boat_are_we_dragging;
    int what_building_is_dragging;


};