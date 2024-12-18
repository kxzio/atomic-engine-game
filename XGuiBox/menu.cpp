#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <deque>
#include <algorithm>
#include <fstream>
#include "resources/window_profiling/window.h"
#include "menu.h"
#include "resources/imgui_resource/imgui.h"
#include "../XGuiBox/xguibox/xgui.h"
#include "resources/imgui_resource/imgui_internal.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmod.hpp>
#include <cmath>
#include <limits>
#pragma comment(lib, "fmod_vc.lib")

#include <d3d9.h>
#include <map>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#ifndef YOUR_HEADER_FILE_H 
#define YOUR_HEADER_FILE_H
#endif  //YOUR_HEADER_FILE_H


using boost::asio::ip::tcp;
static int tab = 0;

FMOD::System*  sound_system;
FMOD::System* sound_system2;
FMOD::Sound*   sound;
FMOD::Sound*   sound2;
FMOD::Sound*   sound3;
FMOD::Sound* sound4;
FMOD::Channel* channel = 0;
FMOD::Channel* channel2 = 0;
static bool    fmod_init = 0;



std::vector<std::string> split_string(const std::string& str, char delimiter) 
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string temp;

    while (std::getline(ss, temp, delimiter)) {
        result.push_back(temp);
    }

    return result;
}
int getRandomNumber(int min, int max) 
{
    static std::random_device rd;  // Источник случайных чисел
    static std::mt19937 gen(rd()); // Генератор случайных чисел
    std::uniform_int_distribution<> dist(min, max); // Диапазон [min, max]

    return dist(gen);
}
namespace server_client_menu_information
{
    std::mutex                 chat_mutex;
    std::deque < std::string > chat_messages;

    std::string                server_nickname;
    std::string                client_nickname;
    
    std::string                server_playerlist;
    std::string                client_playerlist;

    std::string                server_ready_lobby_list;
    std::string                client_ready_lobby_list;

    static bool                client_nickname_is_free;
}

void add_message(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(server_client_menu_information::chat_mutex);
    server_client_menu_information::chat_messages.push_back(msg );
    if (server_client_menu_information::chat_messages.size() > 15) {
        server_client_menu_information::chat_messages.pop_front();
    }
}

bool IsRequest(std::string socket, std::string message)
{
    return socket.find(message) != std::string::npos;
}

class ChatServer 
{
public:

    ChatServer(boost::asio::io_context& io_context, const std::string& ip, short port, const std::string& nickname)
        : acceptor_(io_context, tcp::endpoint(boost::asio::ip::make_address(ip), port)), nickname_(nickname) 
    {
       

        server_client_menu_information::server_playerlist += server_client_menu_information::server_nickname + + ",";
        do_accept();
    }

    void send_message(const std::string& message) 
    {
        for (auto& client : clients_) 
        {
            boost::asio::async_write(*client.socket, boost::asio::buffer(message + "\n"),
                [](boost::system::error_code, std::size_t) {});
        }
    }

    void stop_connection()
    {
        acceptor_.close();
        for (auto& client : clients_)
        {
            if (client.socket && client.socket->is_open()) {
                client.socket->close();
            }
        }
        clients_.clear();
        server_client_menu_information::chat_messages           .clear();
        server_client_menu_information::server_playerlist       .clear();
        server_client_menu_information::server_ready_lobby_list .clear();
    }

private:

    struct ClientInfo 
    {
        std::shared_ptr<tcp::socket> socket;
        std::string nickname;
    };

    void do_accept() 
    {
        auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) 
            {
                if (!ec) 
                {
                    clients_.push_back({ socket, "" });
                    do_read(socket);
                    add_message("Client connected /green/");
                }
                do_accept();
            });
    }

    void do_read(std::shared_ptr<tcp::socket> socket) 
    {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::      async_read_until(*socket, *buffer, "\n", [this, socket, buffer](boost::system::error_code ec, std::size_t) {
            if (!ec)
            {
                std::istream    is(buffer.get());
                std::string     message;
                std::getline    (is, message);

                if (IsRequest(message, "USER.JOIN:")) 
                {
                    std::string        nickname = message.erase(0, 10);
                    add_client(socket, nickname);
                }
                else if (IsRequest(message, "PLAYER.READY.LOBBY:")) 
                {
                    std::string nickname = message.erase(0, 19);
                    server_client_menu_information::server_ready_lobby_list += nickname + ",";
                    send_message("PLAYER.READY.LOBBY.LIST:" + server_client_menu_information::server_ready_lobby_list);
                }
                else if (IsRequest(message, "PLAYER.NOTREADY.LOBBY:"))
                {
                    std::string nickname = message.erase(0, 22);
                    auto pos2 = server_client_menu_information::server_ready_lobby_list.find(nickname + ",");
                    if (pos2 != std::string::npos) {
                        server_client_menu_information::server_ready_lobby_list.erase(pos2, nickname.length() + 1);
                    }
                    send_message("PLAYER.READY.LOBBY.LIST:" + server_client_menu_information::server_ready_lobby_list);
                }
                else 
                {
                    add_message(message);
                    send_message(message);
                }
                do_read(socket);
            }
            else {
                handle_disconnect(socket);
            }
            });
    }

    void add_client(std::shared_ptr<tcp::socket> socket, std::string nickname) 
    {

        // Обновляем никнейм клиента в списке
        for (auto& client : clients_) {
            if (client.socket == socket) {
                client.nickname = nickname;
                break;
            }
        }
        // Добавляем никнейм в список игроков
        server_client_menu_information::server_playerlist += nickname + ",";
        send_message("PLAYERLIST:" + server_client_menu_information::server_playerlist);
        send_message("PLAYER.READY.LOBBY.LIST:" + server_client_menu_information::server_ready_lobby_list);
    }

    void handle_disconnect(std::shared_ptr<tcp::socket> socket) 
    {
        auto it = std::find_if(clients_.begin(), clients_.end(),
            [&socket](const ClientInfo& client) { return client.socket == socket; });

        if (it != clients_.end()) 
        {
            std::string nickname = it->nickname;
            clients_.        erase(it);

            auto pos = server_client_menu_information::server_playerlist.find(nickname + ",");
            if (pos != std::string::npos) {
                server_client_menu_information::server_playerlist       .erase(pos, nickname.length() + 1);
            }

            auto pos2 = server_client_menu_information::server_ready_lobby_list.find(nickname + ",");
            if (pos2 != std::string::npos) {
                server_client_menu_information::server_ready_lobby_list .erase(pos2, nickname.length() + 1);
            }

            send_message    ("PLAYER.READY.LOBBY.LIST:" + server_client_menu_information::server_ready_lobby_list);
            send_message    ("PLAYERLIST:" + server_client_menu_information::server_playerlist);
            add_message     (nickname + " has left the server /red/");
            send_message    (nickname + " has left the server /red/");
        }
    }

    tcp::acceptor           acceptor_;
    std::vector<ClientInfo> clients_;
    std::string             nickname_;
};

class ChatClient
{
public:

    ChatClient(boost::asio::io_context& io_context, const std::string& host, short port, const std::string& nickname)
        : socket_(io_context), nickname_(nickname) 
    {
        tcp::resolver    resolver(io_context);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, auto) 
            {
            if (!ec) 
            {
                do_read    ();
                add_message("Connected to server /green/");

                std::string message_for_nick = "USER.JOIN:" + nickname_ ;
                send_message(message_for_nick);
                
            }
            });
    }

    void send_message(const std::string& message) 
    {
        std::string full_message = message;
        boost::asio::async_write(socket_, boost::asio::buffer(full_message + "\n"),
            [](boost::system::error_code, std::size_t) {});
    }

    void disconnect() 
    {
        socket_.close(); 
        server_client_menu_information::client_playerlist.clear();
        server_client_menu_information::client_ready_lobby_list.clear();
        server_client_menu_information::chat_messages.clear();
    }

private:

    void do_read() 
    {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(socket_, *buffer, "\n", [this, buffer](boost::system::error_code ec, std::size_t) {
            if (!ec) 
            {
                std::istream     is(buffer.get());
                std::string      message;
                std::getline(is, message);

                if      (IsRequest(message,"PLAYERLIST:")) {
                    server_client_menu_information::client_playerlist = message     .erase(0, 11);
                }
                else if (IsRequest(message, "PLAYER.READY.LOBBY.LIST:"))
                {
                    server_client_menu_information::client_ready_lobby_list = message.erase(0, 24);
                }
                else if (IsRequest(message, "SERVER:CLOSED_CONNECTION"))
                {
                    this->disconnect();
                    tab = 5;
                }
                else 
                {
                    add_message(message);
                }
                do_read();
            }
            });
    }

    tcp::socket socket_;
    std::string nickname_;
};

std::shared_ptr<ChatServer> server;
std::shared_ptr<ChatClient> client;
std::shared_ptr<boost::asio::io_context> io_context;

void run_server(const std::string& ip, short port, const std::string& nickname)
{
    if (server) {
        io_context->stop();  // Останавливаем io_context, чтобы завершить все асинхронные операции
        io_context->run();
        server.reset();   // Сбросим старое подключение
    }

    // Создаём новый io_context и executor
    io_context = std::make_shared<boost::asio::io_context>();

    // Создаём новый сервер с новым io_context
    server = std::make_shared<ChatServer>(*io_context, ip, port, nickname);

    io_context->run();
}

void run_client(const std::string& host, short port, const std::string& nickname)
{
    // Создаём новый io_context и executor
    io_context = std::make_shared<boost::asio::io_context>();

    client = std::make_shared<ChatClient>(*io_context, host, port, nickname);
    io_context->run();
}

ImVec2 GetTextureSize(IDirect3DTexture9* texture) {
    if (!texture) {
        return ImVec2(0, 0); 
    }

    D3DSURFACE_DESC desc;
    HRESULT hr = texture->GetLevelDesc(0, &desc); 
    if (FAILED(hr)) {
        std::cerr << "Failed to get texture description. HRESULT: " << std::hex << hr << std::endl;
        return ImVec2(0, 0);
    }

    return ImVec2(static_cast<float>(desc.Width), static_cast<float>(desc.Height));
}
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

struct city_vector_data
{
    std::string city_name;
    ImVec2 city_pos;
};

struct country_data 
{
    std::string name;
    IDirect3DTexture9* texture;
    ImVec2 position;
    ImVec2 size; 
    ImVec2 hitbox_size;
    std::vector<city_vector_data> cities;

    bool hitbox_get = false;
    std::vector<Point> convex_hull = {};
    std::vector<ImVec2> convex_hull_screen_coords;
};


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
std::vector<unsigned char> alphas;

std::vector<Point> GetFilledPixels(const unsigned char* texture_pixels, int width, int height, int* step2) {
    std::vector<Point> filled_pixels;
    const int min_points = 2500;
    int step = std::max(1, static_cast<int>(sqrt((width * height) / static_cast<float>(min_points))));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            if (x % step != 0 || y % step != 0)
                continue;

            int idx = (y * width + x) * 4; 
            unsigned char r = texture_pixels[idx + 0];
            unsigned char g = texture_pixels[idx + 1];
            unsigned char b = texture_pixels[idx + 2];
            unsigned char a = texture_pixels[idx + 3];
            alphas.push_back(a);
            

            if (a == 255) {
                filled_pixels.push_back(Point(x, y)); 
            }
        }
    }
    std::ofstream file("alphas");
    for (int y = 0; y < alphas.size(); y++) {
        file << std::to_string(alphas[y]) << std::endl;
    }
    file.close();
    *step2 = step;
    return filled_pixels;
}


struct ComparePoints {
    bool operator()(const Point& p1, const Point& p2) const {
        return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
    }
};

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
std::vector<Point> GetBoundaryPoints(const std::vector<Point>& filled_pixels, int width, int height, int step) {
    std::unordered_set<size_t> pixel_set; 
    std::vector<Point> boundary_points;

    auto point_to_index = [width](int x, int y) {
        return static_cast<size_t>(y * width + x);
        };

    for (const auto& point : filled_pixels) {

        if (point.x % step == 0 && point.y % step == 0) {
            pixel_set.insert(point_to_index(point.x, point.y));
        }
    }


    for (const auto& point : filled_pixels) {
        if (point.x % step != 0 || point.y % step != 0) continue; 

        bool is_boundary = false;


        const std::vector<Point> neighbors = {
            {point.x - step, point.y}, {point.x + step, point.y},
            {point.x, point.y - step}, {point.x, point.y + step},
            {point.x - step, point.y - step}, {point.x + step, point.y - step},
            {point.x - step, point.y + step}, {point.x + step, point.y + step}
        };


        for (const auto& neighbor : neighbors) {

            if (neighbor.x < 0 || neighbor.y < 0 || neighbor.x >= width || neighbor.y >= height ||
                pixel_set.find(point_to_index(neighbor.x, neighbor.y)) == pixel_set.end()) {
                is_boundary = true;
                break;
            }
        }

        if (is_boundary) {
            boundary_points.push_back(point);
        }
    }

    return boundary_points;
}
bool IsPointInsideBoundary(const Point& point, const std::vector<Point>& boundary_points) {
    int intersections = 0;
    int n = boundary_points.size();

    if (n < 3) return false; 

    for (int i = 0; i < n; i++) {
        const Point& p1 = boundary_points[i];
        const Point& p2 = boundary_points[(i + 1) % n]; 

        if ((p1.y > point.y) != (p2.y > point.y)) {
            float x_intersect = p1.x + (point.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
            if (point.x < x_intersect) {
                intersections++;
            }
        }
    }

    return (intersections % 2) != 0;
}
bool IsPointInExactShape(const Point& point, const std::vector<Point>& filled_pixels, const ImVec2& hitbox_size, float scale, const ImVec2& pos) {
    for (const Point& pixel : filled_pixels) {
        float screen_x = pos.x + pixel.x * hitbox_size.x * scale;
        float screen_y = pos.y + pixel.y * hitbox_size.y * scale;
        if (static_cast<int>(point.x) == static_cast<int>(screen_x) &&
            static_cast<int>(point.y) == static_cast<int>(screen_y)) {
            return true;
        }
    }
    return false;
}
void CopyToClipboard(const std::string& text) {
    // Открываем буфер обмена
    if (!OpenClipboard(nullptr)) {
        return;
    }

    // Очищаем буфер обмена
    EmptyClipboard();

    // Выделяем память для текста
    size_t size = (text.size() + 1) * sizeof(char);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hGlobal) {
        // Копируем текст в выделенную память
        memcpy(GlobalLock(hGlobal), text.c_str(), size);
        GlobalUnlock(hGlobal);

        // Устанавливаем текст в буфер обмена
        SetClipboardData(CF_TEXT, hGlobal);
    }

    // Закрываем буфер обмена
    CloseClipboard();

    // Память освобождается системой после использования буфера
}
void menu::render(window_profiling window) 
{
    if (!fmod_init)
    {
        //FMOD::System_Create(&sound_system);
        //FMOD::System_Create(&sound_system2);
        //sound_system->init(8, FMOD_INIT_NORMAL, 0);
        //sound_system2->init(8, FMOD_INIT_NORMAL, 0);
        fmod_init = true;
        //sound_system->createSound("sound.wav", FMOD_DEFAULT, 0, &sound);
        //sound_system->createSound("sound2.wav", FMOD_DEFAULT, 0, &sound2);
        //sound_system->createSound("sound3.wav", FMOD_DEFAULT, 0, &sound3);
        //sound_system2->createSound("st.wav", FMOD_DEFAULT, 0, &sound4);
    }

    channel2->setVolume(0.2);
    bool playing = false;

    if (!playing)
    {
        sound_system2->playSound(sound4, 0, false, &channel2);
        playing = true;
    }

    ImGui::GetStyle().Colors[ImGuiCol_WindowBg]         = ImColor(0, 0, 0); 
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Button]           = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]     = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]   = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Border]           = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_Separator]        = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_CheckMark]        = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab]    = ImColor(79, 255, 69, 130);

    ImGui::GetStyle().WindowPadding     = ImVec2(10, 10);
    ImGui::GetStyle().ScrollbarRounding = 0;
    ImGui::GetStyle().ScrollbarSize     = 4.f;


    //get screen size
    static bool screen_size_get = false;
    static int  screen_x, screen_y;
    //screen size get vars
    if (!screen_size_get) 
    {
        HDC hDCScreen = GetDC(NULL);

        screen_x = GetDeviceCaps(hDCScreen, HORZRES);
        screen_y = GetDeviceCaps(hDCScreen, VERTRES);

        ReleaseDC(NULL, hDCScreen);
        screen_size_get = true;
        

    }

    static bool should_change_offset[64];
    static int extra_offset;
    
    const char* text1[] =
    {
        "   --- Welcome to Atomic Bulletin (2024) ---",
        "--------------------------------------------",
        "-------------- version 0.03 ----------------",
        ">  | DLL LOADED : (DirectX 9.0)    ",
        ">  | DLL LOADED : (SoundSystem)    ",
        ">  | DLL LOADED : (ServerConfig)   ",
        ">  | DLL LOADED : (UserProfile)    ",
        ">  | DLL LOADED : (Freetype)       ",
        "                   - Freetype.dll           ",
    };
    const char* text2[] =
    {
                "> Bulletin statement on the Energy Department’s Oppenheimer decision Rachel Bronson",
        ": 04 Jul 2023 - Bulletin of The Atomic Scientists TL; DR: In 2019, the Department of Energy",
        "                               ",
        "vacated the 1954 Atomic Energy Commission decision that revoked Dr.J.Robert Oppenheimer's security ",
        "clearance as discussed by the authors , and the Bulletin applauds this important decision.",
        "                                  "
        "                                            "
    };
    const char* text3[] =
    {
                "> Element	Most Stable Isotope	Half-life of Most Stable Isotope",
                "                               ",
        "> Technetium	Tc - 91	4.21 x 106 years",
        "                               ",
        "> Promethium	Pm - 145	17.4 years",
        "> Polonium	Po - 209	102 years",
        "> Astatine	At - 210	8.1 hours",
        "> Radon	Rn - 222	3.82 days",
        "> Francium	Fr - 223	22 minutes",
        "> Radium	Ra - 226	1600 years",
        "> Actinium	Ac - 227	21.77 years",
        "> Thorium	Th - 229	7.54 x 104 years",
        "> Protactinium	Pa - 231	3.28 x 104 years",
        "> Uranium	U - 238	4.47×109 years",

    };
    const char* text4[] =
    {

                "> Abstract : ABSTRACT Recently declassified material and other information that has never before appeared",
        "in the public domain allow the authors to explain some of the workings of the Nuclear Emergency Support Team",
        "                               ",
        "> (NEST)—often one of the first units to respond whenever there is a nuclear incident, ",
        "                               ",
        "whether it involves a nuclear reactor or a nuclear weapon.Long the subject of mystique, NEST is often",
        "depicted on screen as a secretive government unit with highly specialized capabilities and harrowing missions.The reality ",
        "                                  "
    };
    const char* text5[] =
    {
        "                                     .",
        "                         .                           .",
        "",
        "",
        "              .                        .                        .",
        "                             .                   .",
        "               [5][4][5]",
        "                                       .",
        "                      .        .               .        .",
        "",
        "       .                  .                         .                  .",
        "",
        "                 .[3]        _[3]          .",
        "                      .           .[2]   .           .",
        "                                ._._     .",
        "                               .    .~~.    .",
        "   .          .[4] .         .[2].[1]  .[2].         .[4] .          .",
        "                               .    .     .    .",
        "                                .    ~- . - ~.",
        "                     .           .[2]   .           .",
        "         .[3] - [3]          .",
        "",
        "      .                  .                         .                  .",
        "",
        "                     .        ~~.",
        "                                       ~",
        "               [5]           .[4]        .[5]",
        "                                       .",
        "              .                                                 .",
        "",
        "",
        "                         .                           .",
        "                                       .",
        ""
    };
    const char* text6[] =
    {
        "> Blast Zone Radii",
        "[3 different bomb types]",
        "                               ",
            "    ______________________   ______________________   ______________________",
            "    |                      | |                      | |                      |",
            "    |    -[10 KILOTONS] - | |     -[1 MEGATON] - | |    -[20 MEGATONS] - |",
            "    |----------------------| |----------------------| |----------------------|",
            "    | Airburst - 1, 980 ft  | | Airburst - 8, 000 ft  | | Airburst - 17, 500 ft |",
            "    |______________________| |______________________| |______________________|",
            "    |                      | |                      | |                      |",
            "    |[1]  0.5 miles      | |[1]  2.5 miles      | |[1]  8.75 miles     |",
            "    |[2]  1 mile         | |[2]  3.75 miles     | |[2]  14 miles       |",
            "    |[3]  1.75 miles     | |[3]  6.5 miles      | |[3]  27 miles       |",
            "    |[4]  2.5 miles      | |[4]  7.75 miles     | |[4]  31 miles       |",
            "     |[5]  3 miles        | |[5]  10 miles       | |[5]  35 miles       |",
            "     |                      | |                      | |                      |",
            "     |______________________| |______________________| |______________________ | "
    };
    const char* text7[] =
    {
                "> is at once more mundane and more remarkable.Formed in the 1970s in response to a spate of nuclear blackmail attempts,",
        "NEST has been at the center of every major nuclear event from the accident at Three Mile Island to the disaster at",
        "                               ",
        "> Fukushima.Other operations, unknown to the public, are described here for perhaps the first time.Historical accounts ",
        "provide a glimpse into the breadth of the organization’s missions, from neutralizing terrorist nuclear devices",
        "                               ",
        "to responding to nuclear reactor accidents.The diversity of NEST’s missions and the uniqueness of its scientific",
        "capabilities set the unit apart as a national asset."
    };

    const char* text8[] =
    {
                "> Abstract : ABSTRACT On December 16. 2022, the Department of Energy vacated the 1954 Atomic Energy",
                "                               ",
        "Commission decision that revoked Dr.J.Robert Oppenheimer’s security clearance.",
        "The Bulletin applauds this important decision.",
        "                                  ",
        "> 04 Mar 2023-Bulletin of The Atomic ScientistsTL; DR: The Nuclear Emergency Support Team(NEST) ",
        "as discussed by the authors is one of the first units to respond whenever there is a nuclear incident,",
        "                               ",
        "> whether it involves a nuclear reactor or a nuclear weapon, and has been at the center of every major",
        "nuclear event from the accident at Three Mile Island to the disaster at Fukushima."
    };



    const char** textArrays[] = { text1, text2, text3, text4, text5, text6 , text7 , text8 };
    const int textArraySizes[] = { ARRAYSIZE(text1), ARRAYSIZE(text2), ARRAYSIZE(text3), ARRAYSIZE(text4), ARRAYSIZE(text5), ARRAYSIZE(text6), ARRAYSIZE(text7), ARRAYSIZE(text8) };


    static int currentArrayIndex = 0;                                 
    static int currentIndex = 0;                                      
    static std::vector<uint8_t> animationCompleted;
    static int offset = 0;                                            
    static bool allTextsCompleted = false;                            
    static double pauseStartTime = -1.0;                              


    const char** text = textArrays[currentArrayIndex];
    int textCount = textArraySizes[currentArrayIndex];
    static int uniqueCounter = 0; 

    animationCompleted.resize(64, false);
    
    sound-> setMode(FMOD_LOOP_OFF);
    sound2->setMode(FMOD_LOOP_OFF);
    sound3->setMode(FMOD_LOOP_OFF);

    for (int i = 0; i < textCount; i++)
    {
        if (i < currentIndex)
        {
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[1].font_addr, 16,
                ImVec2(55, screen_y - 70 + 20 * (i + 1) - offset),
                ImColor(79, 255, 69, 190),
                text[i]
            );
        }
        else if (i == currentIndex)
        {
            
            std::string uniqueText = text[i] + std::string("#") + std::to_string(uniqueCounter);

            static char last_char;
            char current_char;
            std::string currentText = ImGui::AnimatedChar(uniqueText, 1.8f, &current_char);

            static int count;
            if (current_char != ' ' && last_char != current_char)
            {
                count++;

                if (count == 6) {
                    sound_system->playSound(sound, 0, false, &channel);
                    count = 0;
                    sound_system->update();
                }
                last_char = current_char;
            }
            ImGui::GetBackgroundDrawList()->AddText(g_xgui.fonts[1].font_addr, 16,
                ImVec2(55, screen_y - 70 + 20 * (i + 1) - offset),
                ImColor(79, 255, 69, 190),
                currentText.c_str()
            );

            ImVec2 cursorPos = ImVec2(55, screen_y - 70 + 20 * (i + 1) - offset);
            ImVec2 textSize = g_xgui.fonts[1].font_addr->CalcTextSizeA(16, FLT_MAX, -1, currentText.c_str());

            ImVec2 rectStart = ImVec2(cursorPos.x + textSize.x + 3, cursorPos.y + textSize.y);
            ImVec2 rectEnd = ImVec2(rectStart.x + 5.0f, rectStart.y - textSize.y);
            ImU32 rectColor = IM_COL32(79, 255, 69, 255);

            if (static_cast<int>(ImGui::GetTime() * 7) % 2 == 0)
            {
                ImGui::GetBackgroundDrawList()->AddRectFilled(rectStart, rectEnd, rectColor);
            }

            if (currentText.length() >= std::strlen(text[i]))
            {
                animationCompleted[i] = true;
                currentIndex++; 
                offset += 20; 
                pauseStartTime = ImGui::GetTime(); 
            }
        }
        else
        {
            break;
        }
    }
    if (currentIndex == textCount)
    {
        // Ждем окончания паузы
        if (!(pauseStartTime >= 0 && ImGui::GetTime() - pauseStartTime < 10.0))
        {
            uniqueCounter++;

            int random = 0;
            do
            {
                random = getRandomNumber(0, 7);

            } while (random == currentArrayIndex);
            currentArrayIndex = random;

            {
                animationCompleted.clear();
                currentIndex = 0;
                offset = 0;
                pauseStartTime = -1.0; 
            }
        }
    }


    //input values
    static char message     [128];
    static char client_ip   [128];
    static char nickname    [128] = "Guest";
    static char server_ip   [128] = "127.0.0.1";
    static bool ready_for_game;
    static bool all_players_are_ready;

    //tabs names
    enum tabs
    {
        MAIN_MENU,
        SERVER_CREATE,
        SERVER_MENU,
        CLIENT_JOIN,
        CLIENT_MENU,
        ERROR_MENU
    };


    enum global_game_tabs
    {
        menu,
        country_select,
        game_process  ,
        total_results ,
    };

    //game process
    static int global_game_scene_tab;

    //game global scenes
    switch (global_game_scene_tab)
    {
        case global_game_tabs::menu:
        {
            //main menu window setup
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 - 400 / 2, screen_y / 2 - 400 / 2));
            ImGui::SetNextWindowSize(ImVec2(400, 400));

            //menu
            static bool r = true;
            ImGui::Begin("Defcon 2.0", &r, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);

            //main menu code
            switch (tab)
            {
                case  MAIN_MENU: {
                    ImGui::NewLine();
                    ImGui::Text("Welcome to Atomic Bulletin!");

                    ImGui::NewLine();
                    ImGui::InputText("Nickname", nickname, 128);
                    ImGui::NewLine();

                    if (ImGui::Button("Create Server", ImVec2(380, 35))) {
                        sound_system->playSound(sound3, 0, false, &channel);
                        sound_system->update();
                        tab = 1;
                    }
                    if (ImGui::Button("Join Server", ImVec2(380, 35))) {
                        sound_system->playSound(sound3, 0, false, &channel);
                        sound_system->update();
                        tab = 3;
                    }
                    break;
                }
                case  SERVER_CREATE: {
                    ImGui::InputText("Server IP", server_ip, 128);

                    if (ImGui::Button("Create Server", ImVec2(380, 35)))
                    {
                        tab = 2;
                        server_client_menu_information::server_nickname = nickname;
                        std::thread(run_server, std::string(server_ip), 1234, std::string(nickname)).detach();
                        sound_system->playSound(sound2, 0, false, &channel);
                        sound_system->update();
                    }
                    break;
                }
                case  SERVER_MENU:
                {
                    ImGui::Text((std::string("Server is running on IP ") + server_ip).c_str());

                    std::vector<std::string> visual_playerlist;
                    std::string temp;

                    for (int i = 0; i < server_client_menu_information::server_playerlist.size(); i++)
                    {
                        if (server_client_menu_information::server_playerlist[i] == ',')
                        {
                            visual_playerlist.push_back(temp);
                            temp.clear();
                        }
                        else
                        {
                            temp += server_client_menu_information::server_playerlist[i];
                        }
                    }

                    if (!temp.empty())
                    {
                        visual_playerlist.push_back(temp);
                    }

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    auto visual_ready_playerlist = split_string(server_client_menu_information::server_ready_lobby_list, ',');

                    int ready_players = 0;
                    for (int i = 0; i < visual_playerlist.size(); i++)
                    {
                        if (i == 0)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 40)));
                        // [ administrator ]

                        if (i != 0)
                            ImGui::Text("%s", visual_playerlist[i].c_str());
                        else
                            ImGui::Text("%s", std::string(visual_playerlist[i] + " [ administrator ]").c_str());

                        int restored_y = ImGui::GetCursorPosY();

                        if (true)
                        {
                            for (int i2 = 0; i2 < visual_ready_playerlist.size(); i2++)
                            {
                                if (visual_ready_playerlist[i2] == visual_playerlist[i])
                                {
                                    ready_players++;
                                    ImGui::SameLine();
                                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                    ImGui::SetCursorPosX(360);
                                    bool r = true;
                                    ImGui::Checkmark(std::string(visual_playerlist[i] + std::string("checkmark")).c_str());
                                }
                            }
                            ImGui::SetCursorPosY(restored_y);
                        }

                        if (i == 0)
                            ImGui::PopStyleColor();
                    }

                    if (ready_players == visual_playerlist.size())
                    {
                        all_players_are_ready = true;
                    }
                    else
                        all_players_are_ready = false;

                    ImGui::EndListBox();


                    ImGui::NewLine();
                    ImGui::Text("Chat:");
                    ImGui::NewLine();

                    ImGui::BeginListBox("Messages", ImVec2(380, 100));

                    {

                        std::lock_guard<std::mutex> lock(server_client_menu_information::chat_mutex);
                        for (std::string msg : server_client_menu_information::chat_messages)
                        {
                            if (msg.find("/red/") != std::string::npos)
                            {
                                msg.erase(msg.find("/red/"), 5);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 25, 25)));
                                ImGui::Arrow(ImGuiDir_Left, ImColor(255, 25, 25));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else if (msg.find("/green/") != std::string::npos)
                            {
                                msg.erase(msg.find("/green/"), 7);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(79, 255, 69)));
                                ImGui::Arrow(ImGuiDir_Right, ImColor(79, 255, 69));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else if (msg.find("/blue/") != std::string::npos)
                            {
                                msg.erase(msg.find("/blue/"), 6);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(104, 154, 252)));
                                ImGui::Arrow(ImGuiDir_Right, ImColor(104, 154, 252));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else
                                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));

                            ImGui::Text("%s", msg.c_str());

                            ImGui::PopStyleColor();
                        }

                        static int last_chat_size;

                        if (server_client_menu_information::chat_messages.size() != last_chat_size)
                        {
                            ImGui::SetScrollHereY(1.0f);
                            last_chat_size = server_client_menu_information::chat_messages.size();
                        }
                    }

                    ImGui::EndListBox();

                    ImGui::InputText("Message", message, 128);

                    if (ImGui::Button("Send", ImVec2(380, 35)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                        server->send_message(server_client_menu_information::server_nickname + ": " + std::string(message));
                        add_message(server_client_menu_information::server_nickname + ": " + std::string(message));
                        memset(message, 0, sizeof(message));
                        ImGui::SetItemDefaultFocus();

                    }

                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (ready_for_game)
                            server_client_menu_information::server_ready_lobby_list += server_client_menu_information::server_nickname + ",";

                        if (!ready_for_game)
                        {
                            auto pos = server_client_menu_information::server_ready_lobby_list.find(server_client_menu_information::server_nickname + ",");
                            if (pos != std::string::npos) {
                                server_client_menu_information::server_ready_lobby_list.erase(pos, server_client_menu_information::server_nickname.length() + 1);
                            }
                        }
                        server->send_message("PLAYER.READY.LOBBY.LIST:" + server_client_menu_information::server_ready_lobby_list);

                        last_ready_for_game = ready_for_game;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(319);
                    if (ImGui::Button("Disconnect"))
                    {
                        server->send_message("SERVER:CLOSED_CONNECTION");
                        server->stop_connection();
                        io_context->stop();

                        tab = 0;

                        sound_system->playSound(sound3, 0, false, &channel);
                        sound_system->update();
                    }

                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 - 400 / 2) - 500, screen_y / 2 - 400 / 2));
                    ImGui::SetNextWindowSize(ImVec2(400, 400));

                    ImGui::Begin("Server game settings");
                    {
                        static int last_selected_game_mode = 0;
                        static int selected_game_mode = 0;
                        const char* game_modes_selection[] = { "Default (30 mins)", "Fast (15 mins)", "Long (45 mins)" };
                        ImGui::Combo("Game mode", &selected_game_mode, game_modes_selection, IM_ARRAYSIZE(game_modes_selection));
                        if (last_selected_game_mode != selected_game_mode) {
                            server->send_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            add_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            last_selected_game_mode = selected_game_mode;
                        }


                        if (all_players_are_ready)
                            if (ImGui::Button("Start Game!", ImVec2(380, 35)))
                            {
                                global_game_scene_tab = game_process;
                            }
                    }
                    ImGui::End();

                    break;
                }
                case  CLIENT_JOIN: {
                    ImGui::InputText("Server IP", client_ip, 128);
                    if (ImGui::Button("Join Server", ImVec2(380, 35)))
                    {
                        tab = 4;
                        server_client_menu_information::client_nickname = nickname;
                        std::thread(run_client, std::string(client_ip), 1234, std::string(nickname)).detach();
                        sound_system->playSound(sound2, 0, false, &channel);
                        sound_system->update();
                    }
                    break;
                }
                case  CLIENT_MENU: {
                    ImGui::Text("Connected to the server!");

                    std::vector<std::string> visual_playerlist;
                    std::string temp;

                    for (int i = 0; i < server_client_menu_information::client_playerlist.size(); i++)
                    {
                        if (server_client_menu_information::client_playerlist[i] == ',')
                        {
                            visual_playerlist.push_back(temp);
                            temp.clear();
                        }
                        else
                        {
                            temp += server_client_menu_information::client_playerlist[i];
                        }
                    }

                    if (!temp.empty())
                    {
                        visual_playerlist.push_back(temp);
                    }

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    auto visual_ready_playerlist = split_string(server_client_menu_information::client_ready_lobby_list, ',');

                    for (int i = 0; i < visual_playerlist.size(); i++)
                    {
                        if (i == 0)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 40)));
                        // [ administrator ]

                        if (i != 0)
                            ImGui::Text("%s", visual_playerlist[i].c_str());
                        else
                            ImGui::Text("%s", std::string(visual_playerlist[i] + " [ administrator ]").c_str());

                        int restored_y = ImGui::GetCursorPosY();

                        if (true)
                        {
                            for (int i2 = 0; i2 < visual_ready_playerlist.size(); i2++)
                            {
                                if (visual_ready_playerlist[i2] == visual_playerlist[i])
                                {
                                    ImGui::SameLine();
                                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                    ImGui::SetCursorPosX(360);
                                    bool r = true;
                                    ImGui::Checkmark(std::string(visual_playerlist[i] + std::string("checkmark")).c_str());
                                }
                            }
                            ImGui::SetCursorPosY(restored_y);
                        }

                        if (i == 0)
                            ImGui::PopStyleColor();
                    }

                    ImGui::EndListBox();


                    ImGui::NewLine();
                    ImGui::Text("Chat:");
                    ImGui::NewLine();

                    ImGui::BeginListBox("Messages", ImVec2(380, 100));

                    {
                        std::lock_guard<std::mutex> lock(server_client_menu_information::chat_mutex);
                        for (std::string msg : server_client_menu_information::chat_messages)
                        {
                            if (msg.find("/red/") != std::string::npos)
                            {
                                msg.erase(msg.find("/red/"), 5);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 25, 25)));
                                ImGui::Arrow(ImGuiDir_Left, ImColor(255, 25, 25));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else if (msg.find("/green/") != std::string::npos)
                            {
                                msg.erase(msg.find("/green/"), 7);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(79, 255, 69)));
                                ImGui::Arrow(ImGuiDir_Right, ImColor(79, 255, 69));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else if (msg.find("/blue/") != std::string::npos)
                            {
                                msg.erase(msg.find("/blue/"), 6);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(104, 154, 252)));
                                ImGui::Arrow(ImGuiDir_Right, ImColor(104, 154, 252));
                                ImGui::Text("  "); ImGui::SameLine();
                            }
                            else
                                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));

                            ImGui::Text("%s", msg.c_str());

                            ImGui::PopStyleColor();
                        }

                        static int last_chat_size;

                        if (server_client_menu_information::chat_messages.size() != last_chat_size)
                        {
                            ImGui::SetScrollHereY(1.0f);
                            last_chat_size = server_client_menu_information::chat_messages.size();
                        }
                    }

                    ImGui::EndListBox();

                    ImGui::InputText("Message", message, 128);
                    if (ImGui::Button("Send", ImVec2(380, 35)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                        client->send_message(server_client_menu_information::client_nickname + ": " + std::string(message));
                        memset(message, 0, sizeof(message));
                    }


                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (last_ready_for_game != ready_for_game)
                        {
                            if (ready_for_game)
                                client->send_message("PLAYER.READY.LOBBY:" + server_client_menu_information::client_nickname);

                            if (!ready_for_game)
                            {
                                client->send_message("PLAYER.NOTREADY.LOBBY:" + server_client_menu_information::client_nickname);
                            }

                            last_ready_for_game = ready_for_game;
                        }

                        last_ready_for_game = ready_for_game;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(319);
                    if (ImGui::Button("Disconnect"))
                    {
                        if (client != nullptr)
                        {
                            client->disconnect();
                            client.reset();
                        }

                        io_context->stop();
                        io_context->reset();
                        tab = 0;

                        sound_system->playSound(sound3, 0, false, &channel);
                        sound_system->update();
                    }

                    break;

                }
                case  ERROR_MENU: {
                    ImGui::Text("Server closed connection!");
                    if (ImGui::Button("Back to menu", ImVec2(380, 35)))
                    {
                        tab = 0;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::End();

        }
        break;
        case global_game_tabs::country_select :
        {

        }
        break;

        case global_game_tabs::game_process   :
        {
            static ImVec2 final_map_pos;
            static float  map_scale = 1.25;
            static float  animated_map_scale;
            static ImVec2 moved_pos = ImVec2(-115, 241.5);
            static POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(g_xgui.hwnd_of_process, &cursor_pos);

            static ImVec2 cursor_pos_screened_value;
            cursor_pos_screened_value.x = (((screen_x * map_scale) / 2 - (cursor_pos.x * map_scale)) / (map_scale * 2));
            cursor_pos_screened_value.y = (((screen_y * map_scale) / 2 - (cursor_pos.y * map_scale)) / (map_scale * 2));
            ImVec2 cursor_rel_to_map = ImVec2(
                (cursor_pos.x - moved_pos.x - screen_x / 2) / map_scale,
                (cursor_pos.y - moved_pos.y - screen_y / 2) / map_scale
            );
            if (ImGui::GetIO().MouseWheel > 0)
            {
                float old_scale = map_scale;

                if (map_scale < 5.25)
                {
                    map_scale += 0.25f;

                    //moved_pos.x += cursor_rel_to_map.x / 10 * (old_scale - map_scale) * map_scale;
                    //moved_pos.y += cursor_rel_to_map.y / 10 * (old_scale - map_scale) * map_scale;

                    moved_pos.x -= 70 / map_scale;
                    moved_pos.y -= 20 / map_scale;

                }               
                else 
                    map_scale = 5.25;

            }
            if (ImGui::GetIO().MouseWheel < 0)
            {
                if (map_scale > 1.25)
                {
                    map_scale -= 0.25f;

                }
                else
                    map_scale = 1.25;
            }

            //ImGui::GetBackgroundDrawList()->AddText(ImVec2(50, 74), ImColor(255, 255, 0), std::to_string(cursor_pos_screened_value.x).c_str());
            //ImGui::GetBackgroundDrawList()->AddText(ImVec2(50, 86), ImColor(255, 255, 0), std::to_string(cursor_pos_screened_value.y).c_str());
            animated_map_scale = ImGui::LerpAnimate("Map_Move", "Scale", true, map_scale, 15, ImGui::INTERP);
            
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

            if (moved_pos.y > limits_y[0])
                moved_pos.y = limits_y[0];

            if (moved_pos.y < limits_y[1])
                moved_pos.y = limits_y[1];

            if (moved_pos.x > limits_x[0])
                moved_pos.x = limits_x[0];

            if (moved_pos.x < limits_x[1])
                moved_pos.x = limits_x[1];

            ImGui::GetBackgroundDrawList()->AddText(ImVec2(50, 70), ImColor(255, 255, 0), std::to_string(cursor_pos.x).c_str());
            ImGui::GetBackgroundDrawList()->AddText(ImVec2(50, 82), ImColor(255, 255, 0), std::to_string(cursor_pos.y).c_str());

            float animated_map_pos_x = ImGui::LerpAnimate("Map_Move", "PositionX", true, moved_pos.x, 14, ImGui::INTERP);
            float animated_map_pos_y = ImGui::LerpAnimate("Map_Move", "PositionY", true, moved_pos.y, 14, ImGui::INTERP);
            final_map_pos = ImVec2(animated_map_pos_x, animated_map_pos_y);

            //0.195, 0.123 North America
            //0.18, 0.23 EU
            //0.14, 0.142 South AMerica
            //0.216, 0.141 Latin
            //0.1244, 0.127001 east europe
            //0.124, 0.120 North Africa
            //0.146, 0.1374 Mid Africa
            //0.1316, 0.1986 South Africa
            //0.1954, 0.1272 Indostan
            //0.2062, 0.153 China
            //0.136, 0.1976 Central Asia
            //0.1206, 0.188 Australia
            //0.126, 0.1211 East-Asia
            //0.1648, 0.1238 IndoChina
            //0.1596, 0.2266 Turkey
            //0.1432, 0.1956 Transcausia
            //0.1442, 0.1806 Russia
            //0.1234, 0.126201 East Europe
            //0.206, 0.2302 East EU
            //0.1818, 0.1306 North Europe

            static const ImVec2 offset = ImVec2(500, 1);

            static std::vector < city_vector_data > city_data[19];
            
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
                { "Yarevan", ImVec2(17.043537, 14.748335) }
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
                { "Christchurch", ImVec2(199.432220, 132.452957) },
            };

            static std::vector < country_data > countries =
            {       
                { "North-America",  window.countries[window.countries_name::USA               ], ImVec2(-48.6336  + offset.x, 53.9273   + offset.y)   , ImVec2(1683, 2111 ), ImVec2(0.195, 0.123),      city_data[window.countries_name::USA]               },
                { "EU",             window.countries[window.countries_name::EC                ], ImVec2(241.1947  + offset.x, 22.4489   + offset.y)   , ImVec2(1320, 1969 ), ImVec2(0.18, 0.23),        city_data[window.countries_name::EC]                },
                { "North-Europe",   window.countries[window.countries_name::Northeurope       ], ImVec2(377.7431  + offset.x, 0.8408    + offset.y)   , ImVec2(620, 1131  ), ImVec2(0.1818, 0.1306),    city_data[window.countries_name::Northeurope]       },
                { "Australia",      window.countries[window.countries_name::Austrilia         ], ImVec2(849.2101  + offset.x, 488.0281  + offset.y)   , ImVec2(1028, 805  ), ImVec2(0.1206, 0.188),     city_data[window.countries_name::Austrilia]         },
                { "Russia",         window.countries[window.countries_name::Russia            ], ImVec2(698.6149  + offset.x, 17.3418   + offset.y)   , ImVec2(2467, 1538 ), ImVec2(0.1442, 0.1806),    city_data[window.countries_name::Russia]            },
                { "China",          window.countries[window.countries_name::China             ], ImVec2(696.0981  + offset.x, 215.8585  + offset.y)   , ImVec2(886, 651   ), ImVec2(0.2062, 0.153),     city_data[window.countries_name::China]             },
                { "Central Asia",   window.countries[window.countries_name::Churki            ], ImVec2(567.2284  + offset.x, 177.5464  + offset.y)   , ImVec2(591, 423   ), ImVec2(0.136, 0.1976),     city_data[window.countries_name::Churki]            },
                { "East EU",        window.countries[window.countries_name::EastEC            ], ImVec2(416.0617  + offset.x, 180.4763  + offset.y)   , ImVec2(330, 420   ), ImVec2(0.206, 0.2302),     city_data[window.countries_name::EastEC]            },
                { "East Europe",    window.countries[window.countries_name::east_europe       ], ImVec2(443.122   + offset.x, 154.4864  + offset.y)   , ImVec2(263, 269   ), ImVec2(0.1234, 0.126201),  city_data[window.countries_name::east_europe]       },
                { "Indostan",       window.countries[window.countries_name::Indostan          ], ImVec2(586.6926  + offset.x, 272.443   + offset.y)   , ImVec2(828, 540   ), ImVec2(0.1954, 0.1272),    city_data[window.countries_name::Indostan]          },
                { "IndoChina",      window.countries[window.countries_name::Indo_China        ], ImVec2(693.4327  + offset.x, 306.3944  + offset.y)   , ImVec2(178, 267   ), ImVec2(0.1648, 0.1238),    city_data[window.countries_name::Indo_China]        },
                { "Latin-USA",      window.countries[window.countries_name::LatinUSA          ], ImVec2(70.8135   + offset.x, 296.821   + offset.y)   , ImVec2(461, 302   ), ImVec2(0.216, 0.141),      city_data[window.countries_name::LatinUSA]          },
                { "North-Africa",   window.countries[window.countries_name::North_WellWellWell], ImVec2(401.2982  + offset.x, 280.6873  + offset.y)   , ImVec2(1060, 520  ), ImVec2(0.124, 0.120),      city_data[window.countries_name::North_WellWellWell]},
                { "East-Asia",      window.countries[window.countries_name::Samurai           ], ImVec2(794.3515  + offset.x, 304.7909  + offset.y)   , ImVec2(1081, 1035 ), ImVec2(0.126, 0.1211),     city_data[window.countries_name::Samurai]           },
                { "South-America",  window.countries[window.countries_name::SouthernUSA       ], ImVec2(100.3651  + offset.x, 460.8606  + offset.y)   , ImVec2(1203, 1225 ), ImVec2(0.14, 0.142),       city_data[window.countries_name::SouthernUSA]       },
                { "Mid-Africa",     window.countries[window.countries_name::MidWellWellWell   ], ImVec2(438.9074  + offset.x, 340.2161  + offset.y)   , ImVec2(623, 291   ), ImVec2(0.146, 0.1374),     city_data[window.countries_name::MidWellWellWell]   },
                { "South-Africa",   window.countries[window.countries_name::South_WellWellWell], ImVec2(443.0285  + offset.x, 439.819   + offset.y)   , ImVec2(563, 849   ), ImVec2(0.1316, 0.1986),    city_data[window.countries_name::South_WellWellWell]},
                { "Turkey",         window.countries[window.countries_name::Turk              ], ImVec2(463.8263  + offset.x, 226.1221  + offset.y)   , ImVec2(335, 243   ), ImVec2(0.1596, 0.2266),    city_data[window.countries_name::Turk]              },
                { "",  window.countries[window.countries_name::Zakavkazie                     ], ImVec2(491.7546  + offset.x, 202.2194  + offset.y)   , ImVec2(154, 101   ), ImVec2(0.1432, 0.1956),    city_data[window.countries_name::Zakavkazie]        }
            };

            int hovered_country_id = -1;
            ImColor country_colors[] =
            {
                ImColor(255, 150, 79),
                ImColor(255, 200, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(255, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
                ImColor(47, 79, 79),
            };
            const static ImVec2 country_name_pos[] =
            {
                ImVec2(1183, 689)
            };

            static int current_country;
            static int current_hitbox;


            for (int i = 0; i < countries.size(); i++)
            {
                auto data = countries[i];
                static float map_scale2 = 0.240010;
                auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale;
                auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale;
                auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) + final_map_pos.x * animated_map_scale;
                auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) + final_map_pos.y * animated_map_scale;

                auto sizex = secondposx - posx;
                auto sizey = secondposy - posy;

                int texture_size_x = data.size.x;  int texture_size_y = data.size.y;

                if (!countries[i].hitbox_get)
                {
                    const unsigned char* texture_pixels = GetTexturePixels(data.texture, g_pd3dDevice, texture_size_x, texture_size_y);
                    int step;
                    std::vector<Point> filled_pixels = GetFilledPixels(texture_pixels, texture_size_x, texture_size_y, &step);
                    //countries[i].convex_hull = filled_pixels;
                    countries[i].convex_hull = GetConvexHullWithMorePoints(filled_pixels, 0.8);

                    if (i == window.countries_name::Indostan)
                    {
                        int start_index = 19;
                        int end_index = 36;

                        if (end_index < countries[i].convex_hull.size())
                        {
                            countries[i].convex_hull.erase(countries[i].convex_hull.begin() + start_index, countries[i].convex_hull.begin() + end_index + 1);
                        }
                    }


                    Point mouse_point = { static_cast<int>(cursor_pos.x), static_cast<int>(cursor_pos.y) };
                    countries[i].hitbox_get = true;
                    texture_pixels = 0;
                    filled_pixels.clear();
                }

                float scale = animated_map_scale;

                std::vector <Point> converted_to_screen_convex;

                for (int i2 = 0; i2 < countries[i].convex_hull.size(); i2++)
                {
                    Point pt = countries[i].convex_hull[i2];
                    ImVec2 screen_pos = ImVec2(posx + pt.x * data.hitbox_size.x * scale, posy + pt.y * data.hitbox_size.y * scale);
                    converted_to_screen_convex.push_back(Point(screen_pos.x, screen_pos.y));

                    if (i2 == current_hitbox && i == current_country)
                    {
                       /*
                            #ifndef DEBUG
                            ImGui::GetForegroundDrawList()->AddRect(
                                screen_pos,
                                ImVec2(screen_pos.x + 5 * animated_map_scale, screen_pos.y + 5 * animated_map_scale),
                                ImColor(255, 0, 0)
                            );
                            #endif       
                        */
                    }
                    if (i == window.countries_name::Russia)
                    {
                        if (i2 == 0)
                        {
                            countries[i].convex_hull[i2].x = 199;
                            countries[i].convex_hull[i2].y = 1589;
                        }

                    }
                    if (i == window.countries_name::EastEC)
                    {
                        if (i2 == 0)
                        {
                            countries[i].convex_hull[i2].x = 622; countries[i].convex_hull[i2].y = 2033;
                        }
                        if (i2 == 1)
                        {
                            countries[i].convex_hull[i2].x = 34; countries[i].convex_hull[i2].y = 78;
                        }
                    }
                    if (i == window.countries_name::Austrilia)
                    {
                        if (i2 == 0)
                        {
                            countries[i].convex_hull[i2].x = 202;
                            countries[i].convex_hull[i2].y = 174;
                        }
                    }
                    if (i == window.countries_name::Samurai)
                    {
                        if (i2 == 0)
                        {
                            countries[i].convex_hull[i2].x = 178;
                            countries[i].convex_hull[i2].y = 1316;
                        }
                        if (i2 == 1)
                        {
                            countries[i].convex_hull[i2].x = 812;
                            countries[i].convex_hull[i2].y = 684;
                        }
                    }
                    if (i == window.countries_name::EC)
                    {
                        if (i2 == 0)
                        {
                            countries[i].convex_hull[i2].x = 254;  countries[i].convex_hull[i2].y = 1030;
                        }
                        if (i2 == 1)
                        {
                            countries[i].convex_hull[i2].x = 60;  countries[i].convex_hull[i2].y =388;
                        }
                        if (i2 == 2)
                        {
                            countries[i].convex_hull[i2].x = 158;  countries[i].convex_hull[i2].y = 268;
                        }
                        if (i2 == 3)
                        {
                            countries[i].convex_hull[i2].x = 238;  countries[i].convex_hull[i2].y = 180;
                        }
                        if (i2 == 4)
                        {
                            countries[i].convex_hull[i2].x = 304;  countries[i].convex_hull[i2].y = 148;
                        }
                        if (i2 == 5)
                        {
                            countries[i].convex_hull[i2].x = 384;  countries[i].convex_hull[i2].y = 128;
                        }
                        if (i2 == 6)
                        {
                            countries[i].convex_hull[i2].x = 436;  countries[i].convex_hull[i2].y = 90;
                        }
                        if (i2 == 7)
                        {
                            countries[i].convex_hull[i2].x = 538;  countries[i].convex_hull[i2].y = 42;
                        }
                        if (i2 == 8)
                        {
                            countries[i].convex_hull[i2].x = 634;  countries[i].convex_hull[i2].y = 16;
                        }


                    }
                    if (i == window.countries_name::Northeurope)
                    {
                        if (i2 == 20)
                        {
                            countries[i].convex_hull[i2].x = 532;
                            countries[i].convex_hull[i2].y = 1960;
                        }
                        if (i2 == 21)
                        {
                            countries[i].convex_hull[i2].x = 436;
                            countries[i].convex_hull[i2].y = 1948;
                        }
                        if (i2 == 22)
                        {
                            countries[i].convex_hull[i2].x = 364;
                            countries[i].convex_hull[i2].y = 1884;
                        }
                        if (i2 == 23)
                        {
                            countries[i].convex_hull[i2].x = 295;
                            countries[i].convex_hull[i2].y = 1831;
                        }
                        if (i2 == 24)
                        {
                            countries[i].convex_hull[i2].x = 364;
                            countries[i].convex_hull[i2].y = 1820;
                        }
                        if (i2 == 2)
                        {
                            countries[i].convex_hull[i2].x = 418;
                            countries[i].convex_hull[i2].y = 82;
                        }
                    }

                    /*
                    ImGui::GetForegroundDrawList()->AddRect(
                        screen_pos,
                        ImVec2(screen_pos.x + 2 * animated_map_scale, screen_pos.y + 2 * animated_map_scale),
                        country_colors[i]
                    );
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[0].font_addr, 15, screen_pos, ImColor(255, 255, 255), std::to_string(i2).c_str());
                    */
                }

                int last_pointed_country = 0;
                if (IsPointInsidePolygon2(Point(cursor_pos.x, cursor_pos.y), converted_to_screen_convex))
                {
                    
                    posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale;
                    posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale;
                    secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale;
                    secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale;

                    sizex = secondposx - posx;
                    sizey = secondposy - posy;
                    hovered_country_id = i;

                }

            }

            for (int i = 0; i < countries.size(); i++)
            {
                auto data = countries[i];
                static float map_scale2 = 0.240010;
                

                auto posx = data.position.x * animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale;
                auto posy = data.position.y * animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale;
                auto secondposx = data.position.x * animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale;
                auto secondposy = data.position.y * animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale;

                auto sizex = secondposx - posx;
                auto sizey = secondposy - posy;

                auto fontsize = 20 + animated_map_scale * 3;
                auto textsize = g_xgui.fonts[2].font_addr->CalcTextSizeA(fontsize, FLT_MAX, -1.f, data.name.c_str());

                //if (map_scale > 1.25)
                {


                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 - 2, posy + sizey / 2 - textsize.y / 2), ImColor(0, 0, 0, 200), data.name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 + 2, posy + sizey / 2 - textsize.y / 2), ImColor(0, 0, 0, 200), data.name.c_str());

                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2, posy + sizey / 2 - textsize.y / 2 - 2), ImColor(0, 0, 0, 200), data.name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 , posy + sizey / 2 - textsize.y / 2 + 2), ImColor(0, 0, 0, 200), data.name.c_str());

                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 - 2, posy + sizey / 2 - textsize.y / 2 - 2), ImColor(0, 0, 0, 200), data.name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 + 2, posy + sizey / 2 - textsize.y / 2 + 2), ImColor(0, 0, 0, 200), data.name.c_str());

                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 + 2, posy + sizey / 2 - textsize.y / 2 - 2), ImColor(0, 0, 0, 200), data.name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2 - 2, posy + sizey / 2 - textsize.y / 2 + 2), ImColor(0, 0, 0, 200), data.name.c_str());

                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2, posy + sizey / 2 - textsize.y / 2), country_colors[i], data.name.c_str());
                    ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2, posy + sizey / 2 - textsize.y / 2), ImColor(255, 255, 255, 120), data.name.c_str());


                    for (int city_id = 0; city_id < data.cities.size(); city_id++)
                    {

                        ImGui::GetForegroundDrawList()->AddRect(ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale - 2 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale - 2 * animated_map_scale), ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale + 2 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale + 2 * animated_map_scale), ImColor(255, 255, 255, 255));
                        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale - 0.5 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale - 0.5 * animated_map_scale), ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale + 0.5 * animated_map_scale, posy + data.cities[city_id].city_pos.y * animated_map_scale + 0.5 * animated_map_scale), country_colors[i]);

                        auto textsize_for_city = g_xgui.fonts[3].font_addr->CalcTextSizeA(17, FLT_MAX, -1.f, data.cities[city_id].city_name.c_str());

                        
                        ImVec2 text_pos = ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].city_pos.y * animated_map_scale - 25);

                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x, text_pos.y - 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());
                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x, text_pos.y + 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());

                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x - 1.5, text_pos.y - 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());
                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x + 1.5, text_pos.y + 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());

                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x + 1.5, text_pos.y - 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());
                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(text_pos.x - 1.5, text_pos.y + 1.5), ImColor(0, 0, 0, 255), data.cities[city_id].city_name.c_str());

                        ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[3].font_addr, 17, ImVec2(posx + data.cities[city_id].city_pos.x * animated_map_scale - textsize_for_city.x / 2, posy + data.cities[city_id].city_pos.y * animated_map_scale - 25), ImColor(255, 255, 255, 255), data.cities[city_id].city_name.c_str());
                    }
                    if (hovered_country_id != -1)
                    {
                        if (hovered_country_id == i)
                        {
                            country_colors[i] = ImColor(255, 50, 0);
                            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[2].font_addr, fontsize, ImVec2(posx + sizex / 2 - textsize.x / 2, posy + sizey / 2 - textsize.y / 2), ImColor(255, 255, 255), data.name.c_str());

                            auto pos = ImVec2(countries[hovered_country_id].position.x * animated_map_scale - (countries[hovered_country_id].size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale,
                                countries[hovered_country_id].position.y * animated_map_scale - (countries[hovered_country_id].size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale);

                            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[0].font_addr, 15, ImVec2(30, 70), ImColor(255, 255, 255), std::to_string(ImVec2(cursor_pos.x - pos.x, cursor_pos.y - pos.y).x / animated_map_scale).c_str());
                            ImGui::GetForegroundDrawList()->AddText(g_xgui.fonts[0].font_addr, 15, ImVec2(30, 100), ImColor(255, 255, 255), std::to_string(ImVec2(cursor_pos.x - pos.x, cursor_pos.y - pos.y).y / animated_map_scale).c_str());

                            if (ImGui::IsMouseClicked(0))
                            {
                                CopyToClipboard(std::to_string(ImVec2(cursor_pos.x - pos.x, cursor_pos.y - pos.y).x / animated_map_scale) + ", " + std::to_string(ImVec2(cursor_pos.x - pos.x, cursor_pos.y - pos.y).y / animated_map_scale));
                            }
                        }

                    }
                }
                

                ImGui::GetBackgroundDrawList()->AddImage(
                    (ImTextureID)data.texture,

                    ImVec2(data.position.x* animated_map_scale - (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale,
                        data.position.y* animated_map_scale - (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale),

                    ImVec2(data.position.x* animated_map_scale + (data.size.x * animated_map_scale * map_scale2) / 2 + final_map_pos.x * animated_map_scale,
                        data.position.y* animated_map_scale + (data.size.y * animated_map_scale * map_scale2) / 2 + final_map_pos.y * animated_map_scale),
                    ImVec2(0, 0),
                    ImVec2(1, 1),
                    country_colors[i]
                );
                
            }

            static ImVec2 current_hitbox_pos;

            static ImVec2 moved_coord;



            /*
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                current_hitbox = 0;
                current_country++;
                current_hitbox_pos.x = countries[current_country].convex_hull[current_hitbox].x;
                current_hitbox_pos.y = countries[current_country].convex_hull[current_hitbox].y;
                moved_coord.x = current_hitbox_pos.x;
                moved_coord.y = current_hitbox_pos.y;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                current_hitbox = 0;
                current_country--;
                current_hitbox_pos.x = countries[current_country].convex_hull[current_hitbox].x;
                current_hitbox_pos.y = countries[current_country].convex_hull[current_hitbox].y;
                moved_coord.x = current_hitbox_pos.x;
                moved_coord.y = current_hitbox_pos.y;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                current_hitbox--;
                current_hitbox_pos.x = countries[current_country].convex_hull[current_hitbox].x;
                current_hitbox_pos.y = countries[current_country].convex_hull[current_hitbox].y;
                moved_coord.x = current_hitbox_pos.x;
                moved_coord.y = current_hitbox_pos.y;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                current_hitbox++;
                current_hitbox_pos.x = countries[current_country].convex_hull[current_hitbox].x;
                current_hitbox_pos.y = countries[current_country].convex_hull[current_hitbox].y;
                moved_coord.x = current_hitbox_pos.x;
                moved_coord.y = current_hitbox_pos.y;
            }

            if (ImGui::IsKeyDown(ImGuiKey_I))
            {
                moved_coord.y = moved_coord.y - 2;
            }
            else if (ImGui::IsKeyDown(ImGuiKey_K))
            {
                moved_coord.y = moved_coord.y + 2;
            }


            if (ImGui::IsKeyDown(ImGuiKey_J))
            {
                moved_coord.x = moved_coord.x - 2;
            }
            else if (ImGui::IsKeyDown(ImGuiKey_L))
            {
                moved_coord.x = moved_coord.x + 2;
            }

            if (ImGui::IsMouseClicked(0))
            {
                countries[current_country].convex_hull.push_back(Point(cursor_pos.x - countries[current_country].position.x, cursor_pos.y - countries[current_country].position.y));
            }

            countries[current_country].convex_hull[current_hitbox].x = moved_coord.x;
            countries[current_country].convex_hull[current_hitbox].y = moved_coord.y;
            current_hitbox_pos.x = countries[current_country].convex_hull[current_hitbox].x;
            current_hitbox_pos.y = countries[current_country].convex_hull[current_hitbox].y;
            #endif
            */

        }
        break;

        case global_game_tabs::total_results  :
        {

        }
        break;
    }


}
