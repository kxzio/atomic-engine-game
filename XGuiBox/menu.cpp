#include <vector>
#include <map>
#include <unordered_set>
#include <mutex>

#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <thread>

#include <memory>
#include <deque>

#include <algorithm>
#include <string>
#include <functional>
#include <fstream>

#include "menu.h"

#include "resources/imgui_resource/imgui.h"
#include "resources/window_profiling/window.h"
#include "resources/imgui_resource/imgui_internal.h"

#include "../XGuiBox/game_process/map_processing.h"
#include "../XGuiBox/xguibox/xgui.h"

#include <fmod.hpp>

#include <cmath>
#include <limits>

#include <d3d9.h>

#pragma comment(lib, "fmod_vc.lib")

#define WIN32_LEAN_AND_MEAN
#ifndef YOUR_HEADER_FILE_H 
#define YOUR_HEADER_FILE_H
#endif  //YOUR_HEADER_FILE_H

using boost::asio::ip::tcp;

int getRandomNumber(int min, int max)
{
    static std::random_device rd;  // Источник случайных чисел
    static std::mt19937 gen(rd()); // Генератор случайных чисел
    std::uniform_int_distribution<> dist(min, max); // Диапазон [min, max]

    return dist(gen);
}

//sound system
namespace game_sound_system  
{
    FMOD::System* sound_system ;
    FMOD::System* sound_system2;

    std::vector< FMOD::Sound*> sounds;

    FMOD::Channel* channel  = 0;
    FMOD::Channel* channel2 = 0;

    static bool    fmod_init = 0;
}

namespace game_scenes_params 
{
    //scene frames
    static int global_game_scene_tab;
    enum global_game_tabs
    {
        menu,
        country_select,
        game_process,
        total_results,
    };

    static int main_menu_tabs;
}

namespace server_client_space
{
    namespace server_client_menu_information
    {
        //chat
        std::mutex                 chat_mutex;
        std::deque < std::string > chat_messages;

        void add_message(const std::string& msg)
        {
            std::lock_guard<std::mutex> lock(server_client_menu_information::chat_mutex);
            server_client_menu_information::chat_messages.push_back(msg);
            if (server_client_menu_information::chat_messages.size() > 15) {
                server_client_menu_information::chat_messages.pop_front();
            }
        }

        std::string serialize_vector_players(const std::vector<player>& players)
        {
            std::ostringstream oss;
            for (const auto& p : players)
            {
                // Сериализация основных данных игрока
                oss << p.id << "," << p.name << "," << p.control_region << "," << int(p.ready_to_play) << ",";

                // Сериализация экономических данных
                oss << p.economics.capital << "," << p.economics.capital_inflow << "," << p.economics.capital_inflow_ratio << ",";

                // Сериализация зданий
                oss << p.region_buildings.size() << ","; // Количество зданий
                for (const auto& b : p.region_buildings)
                {
                    oss << b.building_type << "," << b.progress_of_building << "," << b.endurance << ","
                        << b.pos.x << "," << b.pos.y << "," << int(b.size_converted_to_map) << ",";
                }

                oss << ";"; // Разделитель между игроками
            }
            return oss.str();
        }

        std::vector<player> deserialize_vector_players(const std::string& data)
        {
            std::vector<player> players;
            std::istringstream iss(data);
            std::string player_token;

            while (std::getline(iss, player_token, ';'))
            {
                std::istringstream player_stream(player_token);
                player p;

                std::string id_str, name, control_region_str, ready_to_play_str;
                std::getline(player_stream, id_str, ',');
                std::getline(player_stream, name, ',');
                std::getline(player_stream, control_region_str, ',');
                std::getline(player_stream, ready_to_play_str, ',');

                p.id = std::stoi(id_str);
                p.name = name;
                p.control_region = std::stoi(control_region_str);
                p.ready_to_play = std::stoi(ready_to_play_str);

                // Десериализация экономических данных
                std::string capital_str, capital_inflow_str, capital_inflow_ratio_str;
                std::getline(player_stream, capital_str, ',');
                std::getline(player_stream, capital_inflow_str, ',');
                std::getline(player_stream, capital_inflow_ratio_str, ',');
                p.economics.capital = std::stof(capital_str);
                p.economics.capital_inflow = std::stof(capital_inflow_str);
                p.economics.capital_inflow_ratio = std::stof(capital_inflow_ratio_str);

                // Десериализация зданий
                std::string buildings_count_str;
                std::getline(player_stream, buildings_count_str, ',');
                int buildings_count = std::stoi(buildings_count_str);
                for (int i = 0; i < buildings_count; ++i)
                {
                    building b;
                    std::string building_type_str, progress_of_building_str, endurance_str, pos_x_str, pos_y_str, size_converted_to_map_str;

                    std::getline(player_stream, building_type_str, ',');
                    std::getline(player_stream, progress_of_building_str, ',');
                    std::getline(player_stream, endurance_str, ',');
                    std::getline(player_stream, pos_x_str, ',');
                    std::getline(player_stream, pos_y_str, ',');
                    std::getline(player_stream, size_converted_to_map_str, ',');

                    b.building_type = std::stoi(building_type_str);
                    b.progress_of_building = std::stoi(progress_of_building_str);
                    b.endurance = std::stoi(endurance_str);
                    b.pos = ImVec2(std::stof(pos_x_str), std::stof(pos_y_str));
                    b.size_converted_to_map = std::stoi(size_converted_to_map_str);

                    p.region_buildings.push_back(b);
                }

                players.push_back(p);
            }
            return players;
        }

        std::string serialize_players(const std::vector<player>& players, int player_id)
        {
            for (const auto& p : players)
            {
                if (p.id == player_id)
                {
                    std::ostringstream oss;

                    // Сериализация основных данных игрока
                    oss << p.id << "," << p.name << "," << p.control_region << "," << int(p.ready_to_play) << ",";

                    // Сериализация экономических данных
                    oss << p.economics.capital << "," << p.economics.capital_inflow << "," << p.economics.capital_inflow_ratio << ",";

                    // Сериализация зданий
                    oss << p.region_buildings.size() << ","; // Количество зданий
                    for (const auto& b : p.region_buildings)
                    {
                        oss << b.building_type << "," << b.progress_of_building << "," << b.endurance << ","
                            << b.pos.x << "," << b.pos.y << "," << int(b.size_converted_to_map) << ",";
                    }

                    return oss.str();
                }
            }

        }

        bool deserialize_players(std::vector<player>& players, const std::string& player_data)
        {
            std::istringstream player_stream(player_data);

            // Десериализация данных игрока
            player new_player;
            std::string id_str, name, control_region_str, ready_to_play_str;

            if (!(std::getline(player_stream, id_str, ',') &&
                std::getline(player_stream, name, ',') &&
                std::getline(player_stream, control_region_str, ',') &&
                std::getline(player_stream, ready_to_play_str, ',')))
            {
                return false; // Невозможно разобрать данные
            }

            new_player.id = std::stoi(id_str);
            new_player.name = name;
            new_player.control_region = std::stoi(control_region_str);
            new_player.ready_to_play = std::stoi(ready_to_play_str);

            // Десериализация экономических данных
            std::string capital_str, capital_inflow_str, capital_inflow_ratio_str;
            if (!(std::getline(player_stream, capital_str, ',') &&
                std::getline(player_stream, capital_inflow_str, ',') &&
                std::getline(player_stream, capital_inflow_ratio_str, ',')))
            {
                return false; // Невозможно разобрать экономические данные
            }

            new_player.economics.capital = std::stof(capital_str);
            new_player.economics.capital_inflow = std::stof(capital_inflow_str);
            new_player.economics.capital_inflow_ratio = std::stof(capital_inflow_ratio_str);

            // Десериализация зданий
            std::string buildings_count_str;
            if (!std::getline(player_stream, buildings_count_str, ','))
                return false; // Невозможно разобрать количество зданий

            int buildings_count = std::stoi(buildings_count_str);
            for (int i = 0; i < buildings_count; ++i)
            {
                building b;
                std::string building_type_str, progress_of_building_str, endurance_str, pos_x_str, pos_y_str, size_converted_to_map_str;

                if (!(std::getline(player_stream, building_type_str, ',') &&
                    std::getline(player_stream, progress_of_building_str, ',') &&
                    std::getline(player_stream, endurance_str, ',') &&
                    std::getline(player_stream, pos_x_str, ',') &&
                    std::getline(player_stream, pos_y_str, ',') &&
                    std::getline(player_stream, size_converted_to_map_str, ',')))
                {
                    return false; // Невозможно разобрать данные здания
                }

                b.building_type = std::stoi(building_type_str);
                b.progress_of_building = std::stoi(progress_of_building_str);
                b.endurance = std::stoi(endurance_str);
                b.pos = ImVec2(std::stof(pos_x_str), std::stof(pos_y_str));
                b.size_converted_to_map = std::stoi(size_converted_to_map_str);

                new_player.region_buildings.push_back(b);
            }

            // Поиск игрока с таким ID в существующем векторе
            for (auto& existing_player : players)
            {
                if (existing_player.id == new_player.id)
                {
                    existing_player = new_player; // Обновление игрока
                    return true;
                }
            }

            // Если игрок с таким ID не найден, добавляем нового
            players.push_back(new_player);
            return true;
        }

        player find_player_by_nickname(std::string nickname)
        {
            for (int i = 0; i < g_menu.players.size(); i++)
            {
                if (g_menu.players[i].name == nickname)
                {
                    return g_menu.players[i];
                }
            }
        }

        //server nickname and client nickname
        std::string                server_nickname;
        std::string                client_nickname;

        static bool                client_nickname_is_free;
    }

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

    bool IsRequest(std::string socket, std::string message)
    {
        return socket.find(message) != std::string::npos;
    }

}

class ChatServer
{
public:

    ChatServer(boost::asio::io_context& io_context, const std::string& ip, short port, const std::string& nickname)
        : acceptor_(io_context, tcp::endpoint(boost::asio::ip::make_address(ip), port)), nickname_(nickname)
    {
        g_menu.players.push_back(player{ nickname, int(g_menu.players.size()), 0 });
        std::string serialized_data = "CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
        send_message(serialized_data);
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
        server_client_space::server_client_menu_information::chat_messages.clear();
        g_menu.players.clear();
    }

    void send_and_update_player_class(int id)
    {
        std::string serialized_data = "CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id );
        send_message(serialized_data);
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
                    server_client_space::server_client_menu_information::add_message("Client connected /green/");
                }
                do_accept();
            });
    }

    void do_read(std::shared_ptr<tcp::socket> socket)
    {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(*socket, *buffer, "\n", [this, socket, buffer](boost::system::error_code ec, std::size_t) {
            if (!ec)
            {

                std::istream    is(buffer.get());
                std::string     message;
                std::getline(is, message);

                if (server_client_space::IsRequest(message, "CLASS.PLAYERS:")) 
                {
                    std::string players_data = message.erase(0, 14);
                    server_client_space::server_client_menu_information::deserialize_players(g_menu.players, players_data);
                }
                else if (server_client_space::IsRequest(message, "CLASS.PLAYERS_VECTOR:"))
                {
                    std::string players_data = message.erase(0, 21);
                    g_menu.players = server_client_space::server_client_menu_information::deserialize_vector_players(players_data);
                }
                else if (server_client_space::IsRequest(message, "USER.JOIN:"))
                {
                    std::string        nickname = message.erase(0, 10);
                    add_client(socket, nickname);
                }
                else if (server_client_space::IsRequest(message, "PLAYER.READY.LOBBY:"))
                {
                    int id = std::stoi(message.erase(0, 19));
                    g_menu.players[id].ready_to_play = true;

                    std::string serialized_data = "CLASS.PLAYERS" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
                    send_message(serialized_data);
                }
                else if (server_client_space::IsRequest(message, "PLAYER.NOTREADY.LOBBY:"))
                {
                    int id = std::stoi(message.erase(0, 22));
                    g_menu.players[id].ready_to_play = false;

                    std::string serialized_data = "CLASS.PLAYERS" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
                    send_message(serialized_data);
                }
                else if (server_client_space::IsRequest(message, "GAME_CYCLE:"))
                {
                    g_socket_control.game_cycle_messages = message;
                }
                else
                {
                    server_client_space::server_client_menu_information::add_message(message);
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
        g_menu.players.push_back(player{ nickname, int(g_menu.players.size()), 0 });
        std::string serialized_data = "CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
        send_message(serialized_data);
    }

    void handle_disconnect(std::shared_ptr<tcp::socket> socket)
    {
        auto it = std::find_if(clients_.begin(), clients_.end(),
            [&socket](const ClientInfo& client) { return client.socket == socket; });

        if (it != clients_.end())
        {
            std::string nickname = it->nickname;
            clients_.erase(it);

            auto player_it = std::find_if(g_menu.players.begin(), g_menu.players.end(),
                [&nickname](const player& p) { return p.name == nickname; });

            if (player_it != g_menu.players.end()) {
                g_menu.players.erase(player_it);
            }

            std::string serialized_data = "CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
            send_message(serialized_data);

            server_client_space::server_client_menu_information::add_message(nickname + " has left the server /red/");
            send_message(nickname + " has left the server /red/");
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
                    do_read();
                    server_client_space::server_client_menu_information::add_message("Connected to server /green/");

                    std::string message_for_nick = "USER.JOIN:" + nickname_;
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
        server_client_space::server_client_menu_information::chat_messages.clear();
        g_menu.players.clear();
    }

    void send_and_update_player_class(int id)
    {
        std::string serialized_data = "CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
        send_message(serialized_data);
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

                if (server_client_space::IsRequest(message, "CLASS.PLAYERS:")) {
                    std::string players_data = message.erase(0, 14);
                    server_client_space::server_client_menu_information::deserialize_players(g_menu.players, players_data);
                }
                else if (server_client_space::IsRequest(message, "CLASS.PLAYERS_VECTOR:"))
                {
                    std::string players_data = message.erase(0, 21);
                    g_menu.players = server_client_space::server_client_menu_information::deserialize_vector_players(players_data);
                }
                else if (server_client_space::IsRequest(message, "SERVER:CLOSED_CONNECTION"))
                {
                    this->disconnect();
                    game_scenes_params::main_menu_tabs = 5;
                }
                else if (server_client_space::IsRequest(message, "SERVER:GAME_START"))
                {
                    g_socket_control.player_role = g_socket_control.player_role_enum::CLIENT;
                    game_scenes_params::global_game_scene_tab = game_scenes_params::global_game_tabs::game_process;
                }
                else if (server_client_space::IsRequest(message, "GAME_CYCLE:"))
                {
                    g_socket_control.game_cycle_messages = message;
                }
                else
                {
                    server_client_space::server_client_menu_information::add_message(message);
                }
                do_read();
            }
            });
    }

    tcp::socket socket_;
    std::string nickname_;
};

//main context
std::shared_ptr<boost::asio::io_context> io_context;

//server
std::shared_ptr<ChatServer> server;
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

//client
std::shared_ptr<ChatClient> client;
void run_client(const std::string& host, short port, const std::string& nickname)
{
    // Создаём новый io_context и executor
    io_context = std::make_shared<boost::asio::io_context>();

    client = std::make_shared<ChatClient>(*io_context, host, port, nickname);
    io_context->run();
}

void socket_control::server_send_message(std::string message)
{
    server->send_message(message);
}

void socket_control::client_send_message(std::string message)
{
    client->send_message(message);
}

void socket_control::client_send_player_class(int id)
{
    std::string serialized_data = "CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
    client->send_message(serialized_data);
}
void socket_control::server_send_player_class(int id)
{
    std::string serialized_data = "CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
    server->send_message(serialized_data);
}

void menu::render(window_profiling window) 
{
    if (!game_sound_system::fmod_init)
    {
        game_sound_system::fmod_init = true;
    }

    ImGui::GetStyle().Colors[ImGuiCol_WindowBg]         = ImColor(0, 0, 0); 
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Button]           = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]     = ImColor(35, 35, 35);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]    = ImColor(15, 15, 15);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]   = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Border]           = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_Separator]        = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_ChildBg]          = ImColor(5, 5, 5, 255);
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


    //game global scenes
    switch (game_scenes_params::global_game_scene_tab)
    {
        case game_scenes_params::global_game_tabs::menu:
        {
            //main menu window setup
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 - 400 / 2, screen_y / 2 - 400 / 2));
            ImGui::SetNextWindowSize(ImVec2(400, 490));

            //menu
            static bool r = true;
            ImGui::Begin("Defcon 2.0", &r, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);

            //main menu code
            switch (game_scenes_params::main_menu_tabs)
            {
                case  MAIN_MENU: {
                    ImGui::NewLine();
                    ImGui::Text("Welcome to Atomic Bulletin!");

                    ImGui::NewLine();
                    ImGui::InputText("Nickname", nickname, 128);
                    ImGui::NewLine();

                    if (ImGui::Button("Create Server", ImVec2(380, 35))) {
                        game_scenes_params::main_menu_tabs = 1;
                    }
                    if (ImGui::Button("Join Server", ImVec2(380, 35))) {
                        game_scenes_params::main_menu_tabs = 3;
                    }
                    break;
                }
                case  SERVER_CREATE: {
                    ImGui::InputText("Server IP", server_ip, 128);

                    if (ImGui::Button("Create Server", ImVec2(380, 35)))
                    {
                        game_scenes_params::main_menu_tabs = 2;
                        server_client_space::server_client_menu_information::server_nickname = nickname;
                        std::thread(run_server, std::string(server_ip), 1234, std::string(nickname)).detach();
                    }
                    break;
                }
                case  SERVER_MENU:
                {
                    ImGui::Text((std::string("Server is running on IP ") + server_ip).c_str());

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    int ready_players = 0;
                    for (int i = 0; i < players.size(); i++)
                    {
                        if (i == 0)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 40)));
                        // [ administrator ]

                        if (i != 0)
                            ImGui::Text("%s", players[i].name.c_str());
                        else
                            ImGui::Text("%s", std::string(players[i].name + " [ administrator ]").c_str());

                        int restored_y = ImGui::GetCursorPosY();

                        if (players[i].ready_to_play) 
                        {
                            ready_players++;
                            ImGui::SameLine();
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                            ImGui::SetCursorPosX(355);
                            bool r = true;
                            ImGui::Checkmark(std::string(players[i].name + std::string("checkmark")).c_str());

                            ImGui::SetCursorPosY(restored_y);
                        }

                        if (i == 0)
                            ImGui::PopStyleColor();
                    }

                    if (ready_players == players.size())
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

                        std::lock_guard<std::mutex> lock(server_client_space::server_client_menu_information::chat_mutex);
                        for (std::string msg : server_client_space::server_client_menu_information::chat_messages)
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

                        if (server_client_space::server_client_menu_information::chat_messages.size() != last_chat_size)
                        {
                            ImGui::SetScrollHereY(1.0f);
                            last_chat_size = server_client_space::server_client_menu_information::chat_messages.size();
                        }
                    }

                    ImGui::EndListBox();

                    ImGui::InputText("Message", message, 128);

                    if (ImGui::Button("Send", ImVec2(380, 35)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                        server->send_message(server_client_space::server_client_menu_information::server_nickname + ": " + std::string(message));
                        server_client_space::server_client_menu_information::add_message(server_client_space::server_client_menu_information::server_nickname + ": " + std::string(message));
                        memset(message, 0, sizeof(message));
                        ImGui::SetItemDefaultFocus();

                    }

                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::Text("Select region");

                    static int last_selected_region = 0;
                    static int selected_region = 0;
                    const char* region_selection[] =
                    { "North-America", "EU", "North-Europe", "Australia", "Russia", "China", "Central Asia", "East EU", "East Europe", "IndoChina", "Indostan", "Latin-USA", "North-Africa", "East-Asia", "South-America", "Mid-Africa", "South-Africa", "Turkey", "Transcaucasia" };

                    ImGui::Combo("Region", &selected_region, region_selection, IM_ARRAYSIZE(region_selection));
                    if (last_selected_region != selected_region)
                    {
                        last_selected_region = selected_region;

                        players[server_client_space::server_client_menu_information::
                                find_player_by_nickname
                                (server_client_space::server_client_menu_information::server_nickname).id].control_region = selected_region;

                        server->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

                    }

                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (ready_for_game)
                        {
                            players[server_client_space::server_client_menu_information::
                                find_player_by_nickname
                                (server_client_space::server_client_menu_information::server_nickname).id].ready_to_play = true;

                            server->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

                        }

                        if (!ready_for_game)
                        {
                            players[server_client_space::server_client_menu_information::
                                find_player_by_nickname
                                (server_client_space::server_client_menu_information::server_nickname).id].ready_to_play = false;

                            server->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

                        }

                        last_ready_for_game = ready_for_game;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(313);
                    if (ImGui::Button("Disconnect"))
                    {
                        server->send_message("SERVER:CLOSED_CONNECTION");
                        server->stop_connection();
                        io_context->stop();
                        game_scenes_params::main_menu_tabs = 0;
                    }

                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 - 400 / 2) - 500, screen_y / 2 - 400 / 2));
                    ImGui::SetNextWindowSize(ImVec2(400, 106));

                    ImGui::Begin("Server game settings");
                    {
                        static int last_selected_game_mode = 0;
                        const char* game_modes_selection[] = { "Default (30 mins)", "Long (45 mins)" };
                        ImGui::Combo("Game mode", &selected_game_mode, game_modes_selection, IM_ARRAYSIZE(game_modes_selection));
                        if (last_selected_game_mode != selected_game_mode) {
                            server->send_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            server_client_space::server_client_menu_information::add_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            last_selected_game_mode = selected_game_mode;
                        }

                        if (all_players_are_ready)
                        if (ImGui::Button("Start Game!", ImVec2(380, 35)))
                        {
                            g_socket_control.player_role = g_socket_control.player_role_enum::SERVER;
                            game_scenes_params::global_game_scene_tab = game_scenes_params::game_process;
                            server->send_message("SERVER:GAME_START");
                        }
                    }
                    ImGui::End();


                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 + 800) - 500, screen_y / 2 - 400 / 2));
                    ImGui::SetNextWindowSize(ImVec2(400, 150));

                    ImGui::Begin("Playerlist");
                    {
                        ImGui::BeginListBox("Countries", ImVec2(380, 107));
                        {
                            for (int i = 0; i < players.size(); i++)
                            {
                                ImGui::Text("%s", players[i].name.c_str());
                                
                                ImGui::SameLine();

                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 255, 100)));

                                ImGui::SetCursorPosX(100);
                                ImGui::Text("%s", region_selection[players[i].control_region]);

                                ImGui::PopStyleColor();

                            }

                        }
                        ImGui::EndListBox();
                    }
                    ImGui::End();

                    break;
                }
                case  CLIENT_JOIN: {
                    ImGui::InputText("Server IP", client_ip, 128);
                    if (ImGui::Button("Join Server", ImVec2(380, 35)))
                    {
                        game_scenes_params::main_menu_tabs = 4;
                        server_client_space::server_client_menu_information::client_nickname = nickname;
                        std::thread(run_client, std::string(client_ip), 1234, std::string(nickname)).detach();
                    }
                    break;
                }
                case  CLIENT_MENU: 
                {
                    ImGui::Text("Connected to the server!");

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    for (int i = 0; i < players.size(); i++)
                    {
                        if (i == 0)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 40)));
                        // [ administrator ]

                        if (i != 0)
                            ImGui::Text("%s", players[i].name.c_str());
                        else
                            ImGui::Text("%s", std::string(players[i].name + " [ administrator ]").c_str());

                        int restored_y = ImGui::GetCursorPosY();

                        if (players[i].ready_to_play)
                        {
                            ImGui::SameLine();
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                            ImGui::SetCursorPosX(355);
                            bool r = true;
                            ImGui::Checkmark(std::string(players[i].name + std::string("checkmark")).c_str());

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
                        std::lock_guard<std::mutex> lock(server_client_space::server_client_menu_information::chat_mutex);
                        for (std::string msg : server_client_space::server_client_menu_information::chat_messages)
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

                        if (server_client_space::server_client_menu_information::chat_messages.size() != last_chat_size)
                        {
                            ImGui::SetScrollHereY(1.0f);
                            last_chat_size = server_client_space::server_client_menu_information::chat_messages.size();
                        }
                    }

                    ImGui::EndListBox();

                    ImGui::InputText("Message", message, 128);
                    if (ImGui::Button("Send", ImVec2(380, 35)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                        client->send_message(server_client_space::server_client_menu_information::client_nickname + ": " + std::string(message));
                        memset(message, 0, sizeof(message));
                    }

                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::Text("Select region");

                    static int last_selected_region = 0;
                    static int selected_region = 0;
                    const char* region_selection[] =
                    { "North-America", "EU", "North-Europe", "Australia", "Russia", "China", "Central Asia", "East EU", "East Europe", "IndoChina", "Indostan", "Latin-USA", "North-Africa", "East-Asia", "South-America", "Mid-Africa", "South-Africa", "Turkey", "Transcaucasia" };

                    ImGui::Combo("Region", &selected_region, region_selection, IM_ARRAYSIZE(region_selection));
                    if (last_selected_region != selected_region)
                    {
                        last_selected_region = selected_region;
                        players[server_client_space::server_client_menu_information::
                            find_player_by_nickname
                            (server_client_space::server_client_menu_information::client_nickname).id].control_region = selected_region;
                        
                        client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::client_nickname).id);

                    }

                    ImGui::NewLine();

                    ImGui::Separator();

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (true)
                        {
                            if (ready_for_game)
                            {
                                players[server_client_space::server_client_menu_information::
                                    find_player_by_nickname
                                    (server_client_space::server_client_menu_information::client_nickname).id].ready_to_play = true;
                                
                                client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::client_nickname).id);

                            }

                            if (!ready_for_game)
                            {
                                players[server_client_space::server_client_menu_information::
                                    find_player_by_nickname
                                    (server_client_space::server_client_menu_information::client_nickname).id].ready_to_play = false;
                                
                                client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::client_nickname).id);

                            }

                            last_ready_for_game = ready_for_game;
                        }
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(313);
                    if (ImGui::Button("Disconnect"))
                    {
                        if (client != nullptr)
                        {
                            client->disconnect();
                            client.reset();
                        }

                        io_context->stop();
                        io_context->reset();
                        game_scenes_params::main_menu_tabs = 0;

                    }

                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 + 800) - 500, screen_y / 2 - 400 / 2));
                    ImGui::SetNextWindowSize(ImVec2(400, 150));

                    ImGui::Begin("Playerlist");
                    {
                        ImGui::BeginListBox("Countries", ImVec2(380, 107));
                        {
                            for (int i = 0; i < players.size(); i++)
                            {
                                ImGui::Text("%s", players[i].name.c_str());

                                ImGui::SameLine();

                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 255, 100)));

                                ImGui::SetCursorPosX(100);
                                ImGui::Text("%s", region_selection[players[i].control_region]);

                                ImGui::PopStyleColor();

                            }

                        }
                        ImGui::EndListBox();
                    }
                    ImGui::End();

                    break;

                }
                case  ERROR_MENU: {
                    ImGui::Text("Server closed connection!");
                    if (ImGui::Button("Back to menu", ImVec2(380, 35)))
                    {
                        game_scenes_params::main_menu_tabs = 0;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::End();

        }
        break;

        case  game_scenes_params::global_game_tabs::country_select :
        {
            //main menu window setup
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 
                
                - 400 / 2, screen_y / 2 - 400 / 2));
            ImGui::SetNextWindowSize(ImVec2(400, 400));

            //menu
            static bool r = true;
            ImGui::Begin("Select region", &r, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);
            {

            }
            ImGui::End();
        }
        break;

        case  game_scenes_params::global_game_tabs::game_process   :
        {
            if (g_socket_control.player_role == g_socket_control.player_role_enum::CLIENT)
            {
                g_map.process_map(window, screen_x, screen_y, server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::client_nickname).id);
            }
            if (g_socket_control.player_role == g_socket_control.player_role_enum::SERVER)
            {
                g_map.process_map(window, screen_x, screen_y, server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);
            }
        }
        break;

        case  game_scenes_params::global_game_tabs::total_results  :
        {

        }
        break;
    }


}
