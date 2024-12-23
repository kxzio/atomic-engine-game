﻿#include <vector>
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

        //server nickname and client nickname
        std::string                server_nickname;
        std::string                client_nickname;

        //playerlists (client and server)
        std::string                server_playerlist;
        std::string                client_playerlist;

        //playerlist (client and server)
        std::string                server_ready_lobby_list;
        std::string                client_ready_lobby_list;

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
        server_client_space::server_client_menu_information::server_playerlist += server_client_space::server_client_menu_information::server_nickname + +",";
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
        server_client_space::server_client_menu_information::server_playerlist.clear();
        server_client_space::server_client_menu_information::server_ready_lobby_list.clear();
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

                if (server_client_space::IsRequest(message, "USER.JOIN:"))
                {
                    std::string        nickname = message.erase(0, 10);
                    add_client(socket, nickname);
                }
                else if (server_client_space::IsRequest(message, "PLAYER.READY.LOBBY:"))
                {
                    std::string nickname = message.erase(0, 19);
                    server_client_space::server_client_menu_information::server_ready_lobby_list += nickname + ",";
                    send_message("PLAYER.READY.LOBBY.LIST:" + server_client_space::server_client_menu_information::server_ready_lobby_list);
                }
                else if (server_client_space::IsRequest(message, "PLAYER.NOTREADY.LOBBY:"))
                {
                    std::string nickname = message.erase(0, 22);
                    auto pos2 = server_client_space::server_client_menu_information::server_ready_lobby_list.find(nickname + ",");
                    if (pos2 != std::string::npos) {
                        server_client_space::server_client_menu_information::server_ready_lobby_list.erase(pos2, nickname.length() + 1);
                    }
                    send_message("PLAYER.READY.LOBBY.LIST:" + server_client_space::server_client_menu_information::server_ready_lobby_list);
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
        // Добавляем никнейм в список игроков
        server_client_space::server_client_menu_information::server_playerlist += nickname + ",";
        send_message("PLAYERLIST:" + server_client_space::server_client_menu_information::server_playerlist);
        send_message("PLAYER.READY.LOBBY.LIST:" + server_client_space::server_client_menu_information::server_ready_lobby_list);
    }

    void handle_disconnect(std::shared_ptr<tcp::socket> socket)
    {
        auto it = std::find_if(clients_.begin(), clients_.end(),
            [&socket](const ClientInfo& client) { return client.socket == socket; });

        if (it != clients_.end())
        {
            std::string nickname = it->nickname;
            clients_.erase(it);

            auto pos = server_client_space::server_client_menu_information::server_playerlist.find(nickname + ",");
            if (pos != std::string::npos) {
                server_client_space::server_client_menu_information::server_playerlist.erase(pos, nickname.length() + 1);
            }

            auto pos2 = server_client_space::server_client_menu_information::server_ready_lobby_list.find(nickname + ",");
            if (pos2 != std::string::npos) {
                server_client_space::server_client_menu_information::server_ready_lobby_list.erase(pos2, nickname.length() + 1);
            }

            send_message("PLAYER.READY.LOBBY.LIST:" + server_client_space::server_client_menu_information::server_ready_lobby_list);
            send_message("PLAYERLIST:" + server_client_space::server_client_menu_information::server_playerlist);
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
        server_client_space::server_client_menu_information::client_playerlist.clear();
        server_client_space::server_client_menu_information::client_ready_lobby_list.clear();
        server_client_space::server_client_menu_information::chat_messages.clear();
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

                if (server_client_space::IsRequest(message, "PLAYERLIST:")) {
                    server_client_space::server_client_menu_information::client_playerlist = message.erase(0, 11);
                }
                else if (server_client_space::IsRequest(message, "PLAYER.READY.LOBBY.LIST:"))
                {
                    server_client_space::server_client_menu_information::client_ready_lobby_list = message.erase(0, 24);
                }
                else if (server_client_space::IsRequest(message, "SERVER:CLOSED_CONNECTION"))
                {
                    this->disconnect();
                    game_scenes_params::main_menu_tabs = 5;
                }
                else if (server_client_space::IsRequest(message, "SERVER:GAME_START"))
                {
                    game_scenes_params::global_game_scene_tab = game_scenes_params::global_game_tabs::game_process;
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


    //game global scenes
    switch (game_scenes_params::global_game_scene_tab)
    {
        case game_scenes_params::global_game_tabs::menu:
        {
            //main menu window setup
            ImGui::SetNextWindowPos(ImVec2(screen_x / 2 - 400 / 2, screen_y / 2 - 400 / 2));
            ImGui::SetNextWindowSize(ImVec2(400, 400));

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

                    std::vector<std::string> visual_playerlist;
                    std::string temp;

                    for (int i = 0; i < server_client_space::server_client_menu_information::server_playerlist.size(); i++)
                    {
                        if (server_client_space::server_client_menu_information::server_playerlist[i] == ',')
                        {
                            visual_playerlist.push_back(temp);
                            temp.clear();
                        }
                        else
                        {
                            temp += server_client_space::server_client_menu_information::server_playerlist[i];
                        }
                    }

                    if (!temp.empty())
                    {
                        visual_playerlist.push_back(temp);
                    }

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    auto visual_ready_playerlist = server_client_space::split_string(server_client_space::server_client_menu_information::server_ready_lobby_list, ',');

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

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (ready_for_game)
                            server_client_space::server_client_menu_information::server_ready_lobby_list += server_client_space::server_client_menu_information::server_nickname + ",";

                        if (!ready_for_game)
                        {
                            auto pos = server_client_space::server_client_menu_information::server_ready_lobby_list.find(server_client_space::server_client_menu_information::server_nickname + ",");
                            if (pos != std::string::npos) {
                                server_client_space::server_client_menu_information::server_ready_lobby_list.erase(pos, server_client_space::server_client_menu_information::server_nickname.length() + 1);
                            }
                        }
                        server->send_message("PLAYER.READY.LOBBY.LIST:" + server_client_space::server_client_menu_information::server_ready_lobby_list);

                        last_ready_for_game = ready_for_game;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(319);
                    if (ImGui::Button("Disconnect"))
                    {
                        server->send_message("SERVER:CLOSED_CONNECTION");
                        server->stop_connection();
                        io_context->stop();
                        game_scenes_params::main_menu_tabs = 0;
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
                            server_client_space::server_client_menu_information::add_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            last_selected_game_mode = selected_game_mode;
                        }


                        if (all_players_are_ready)
                        if (ImGui::Button("Start Game!", ImVec2(380, 35)))
                        {
                            game_scenes_params::global_game_scene_tab = game_scenes_params::game_process;
                            server->send_message("SERVER:GAME_START");
                        }
                    }
                    ImGui::End();

                    break;
                }
                case  CLIENT_JOIN: {
                    ImGui::InputText("Server IP", client_ip, 128);
                    if (ImGui::Button("Join Server", ImVec2(380, 35)))
                    {
                        game_scenes_params::global_game_scene_tab = 4;
                        server_client_space::server_client_menu_information::client_nickname = nickname;
                        std::thread(run_client, std::string(client_ip), 1234, std::string(nickname)).detach();
                    }
                    break;
                }
                case  CLIENT_MENU: {
                    ImGui::Text("Connected to the server!");

                    std::vector<std::string> visual_playerlist;
                    std::string temp;

                    for (int i = 0; i < server_client_space::server_client_menu_information::client_playerlist.size(); i++)
                    {
                        if (server_client_space::server_client_menu_information::client_playerlist[i] == ',')
                        {
                            visual_playerlist.push_back(temp);
                            temp.clear();
                        }
                        else
                        {
                            temp += server_client_space::server_client_menu_information::client_playerlist[i];
                        }
                    }

                    if (!temp.empty())
                    {
                        visual_playerlist.push_back(temp);
                    }

                    ImGui::BeginListBox("Players", ImVec2(380, 64));

                    auto visual_ready_playerlist = server_client_space::split_string(server_client_space::server_client_menu_information::client_ready_lobby_list, ',');

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

                    ImGui::NewLine();
                    static bool last_ready_for_game;
                    ImGui::Checkbox(" READY", &ready_for_game);
                    if (last_ready_for_game != ready_for_game)
                    {
                        if (last_ready_for_game != ready_for_game)
                        {
                            if (ready_for_game)
                                client->send_message("PLAYER.READY.LOBBY:" + server_client_space::server_client_menu_information::client_nickname);

                            if (!ready_for_game)
                            {
                                client->send_message("PLAYER.NOTREADY.LOBBY:" + server_client_space::server_client_menu_information::client_nickname);
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
                        game_scenes_params::main_menu_tabs = 0;

                    }

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

        }
        break;

        case  game_scenes_params::global_game_tabs::game_process   :
        {
            g_map.process_map(window, screen_x, screen_y);
        }
        break;

        case  game_scenes_params::global_game_tabs::total_results  :
        {

        }
        break;
    }


}
