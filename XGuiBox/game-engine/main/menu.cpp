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

#include "../resources/imgui_resource/imgui.h"
#include "../resources/window_profiling/window.h"
#include "../resources/imgui_resource/imgui_internal.h"
#include "../game_process/map_processing.h"
#include "../xguibox/xgui.h"

#include <fmod.hpp>

#include <cmath>
#include <limits>

#include <d3d11.h>

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
    class my_room
    {
    public:
        std::string name;
        int unique_id;
    };

    std::vector< my_room > rooms;

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

        std::string serialize_unit(int unique_id)
        {
            auto bomb_ptr = std::find_if(g_map.units.begin(), g_map.units.end(), [&](units_base target)
                {
                    return target.unique_id == unique_id;
                });
       
            units_base units = *bomb_ptr;
            std::ostringstream oss;
            {
                // Сериализация основных данных игрока
                oss << units.health << "," 
                    << units.spawnpos_converted_to_map << ","
                    << units.owner_country_id << "," 
                    << units.owner_building_id << "," 
                    << units.airplane << "," 
                    << units.warship << "," 
                    << units.class_of_unit << "," 
                    << units.converted_spawn_pos.x << "," 
                    << units.converted_spawn_pos.y << ","
                    << units.unique_id;
            }
            return oss.str();
        }

        bool deserialize_unit(const std::string& data)
        {

                std::istringstream unit_stream(data);
                units_base u;
                std::string field;

                if (std::getline(unit_stream, field, ',')) u.health = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.spawnpos_converted_to_map = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.owner_country_id = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.owner_building_id = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.airplane = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.warship = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.class_of_unit = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.converted_spawn_pos.x = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.converted_spawn_pos.y = std::stoi(field);
                else return false;
                if (std::getline(unit_stream, field, ',')) u.unique_id = std::stoi(field);
                else return false;

                // Поиск игрока с таким ID в существующем векторе
                for (auto& existing_unit : g_map.units)
                {
                    if (existing_unit.unique_id == u.unique_id)
                    {
                        existing_unit = u; // Обновление игрока
                        return true;
                    }
                }

                // Если игрок с таким ID не найден, добавляем нового
                g_map.units.push_back(u);
                return true;
        }

        std::string serialize_unit_health(int unique_id)
        {
            auto bomb_ptr = std::find_if(g_map.units.begin(), g_map.units.end(), [&](units_base target)
                {
                    return target.unique_id == unique_id;
                });

            units_base units = *bomb_ptr;
            std::ostringstream oss;
            {
                // Сериализация основных данных игрока
                oss << units.unique_id << ",";
                oss << units.health << ",";
            }
            return oss.str();
        }

        bool deserialize_unit_health(const std::string& data)
        {

            std::istringstream unit_stream(data);
            units_base u;
            std::string field;

                if (std::getline(unit_stream, field, ',')) u.unique_id = std::stoi(field);
            else return false;
                if (std::getline(unit_stream, field, ',')) u.health = std::stoi(field);
            else return false;

            // Поиск игрока с таким ID в существующем векторе
            for (auto& existing_unit : g_map.units)
            {
                if (existing_unit.unique_id == u.unique_id)
                {
                    existing_unit.health = u.health; // Обновление игрока
                    return true;
                }
            }

            // Если игрок с таким ID не найден, добавляем нового
            g_map.units.push_back(u);
            return true;
        }


        std::string serialize_unit_pos(int region)
        {
            std::ostringstream oss;
            {
                for (const auto& units : g_map.units)
                {
                    if (units.owner_country_id != region)
                        continue;

                    // Сериализация основных данных игрока
                    oss << units.move_offset.x << "," << units.move_offset.y << "," << units.health << "," << units.unique_id;

                    oss << ";";
                }
            }
            return oss.str();
        }

        bool deserialize_unit_pos(const std::string& data)
        {
            std::istringstream iss(data);
            std::string segment;

            std::vector<units_base> new_units;

            // Разделение строки на сегменты по `;`
            while (std::getline(iss, segment, ';')) 
            {
                if (segment.empty()) continue;

                units_base unit;
                std::istringstream segment_stream(segment);
                std::string field;

                if (std::getline(segment_stream, field, ',')) unit.move_offset.x = std::stoi(field);
                else return false;

                if (std::getline(segment_stream, field, ',')) unit.move_offset.y = std::stoi(field);
                else return false;

                if (std::getline(segment_stream, field, ',')) unit.health = std::stoi(field);
                else return false;

                if (std::getline(segment_stream, field, ',')) unit.unique_id = std::stoi(field);
                else return false;

                for (auto& existing_unit : g_map.units)
                {
                    if (existing_unit.unique_id == unit.unique_id)
                    {
                        existing_unit.move_offset.x = unit.move_offset.x;
                        existing_unit.move_offset.y = unit.move_offset.y;
                    }
                }

            }

            return true;

        }

        std::string serialize_vector_players(const std::vector<player>& players)
        {
            std::ostringstream oss;
            for (const auto& p : players)
            {
                // Сериализация основных данных игрока
                oss << p.id << "," << p.name << "," << p.control_region << "," << int(p.ready_to_play) << ",";

                // Сериализация экономических данных
                oss << p.economics.capital << "," << p.economics.capital_inflow << "," << p.economics.capital_inflow_ratio;

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

        std::string serialize_city(int city_id, int country_id)
        {
            std::ostringstream oss;

            // Сериализация данных города с добавлением ID страны
            oss << country_id << "," << g_map.countries[country_id].cities[city_id].name << "," << g_map.countries[country_id].cities[city_id].pos.x << "," << g_map.countries[country_id].cities[city_id].pos.y << "," << g_map.countries[country_id].cities[city_id].population;

            return oss.str();
        }

        std::string serialize_building(int building_id, int country_id)
        {
            std::ostringstream oss;


            // Сериализация данных здания с добавлением ID страны
            oss << country_id << "," << g_map.countries[country_id].buildings[building_id].name << "," << g_map.countries[country_id].buildings[building_id].pos.x << "," << g_map.countries[country_id].buildings[building_id].pos.y << "," << g_map.countries[country_id].buildings[building_id].building_type
                << "," << g_map.countries[country_id].buildings[building_id].progress_of_building << "," << g_map.countries[country_id].buildings[building_id].endurance << "," << g_map.countries[country_id].buildings[building_id].size_converted_to_map;

            return oss.str();
        }

        bool deserialize_city(std::vector<country_data>& countries, const std::string& city_data)
        {
            std::istringstream city_stream(city_data);

            // Десериализация данных города
            city new_city;
            std::string country_id_str, name, x_str, y_str, population_str;

            if (!(std::getline(city_stream, country_id_str, ',') &&
                std::getline(city_stream, name, ',') &&
                std::getline(city_stream, x_str, ',') &&
                std::getline(city_stream, y_str, ',') &&
                std::getline(city_stream, population_str, ',')))
            {
                return false; // Невозможно разобрать данные
            }

            int country_id = std::stoi(country_id_str);
            new_city.name = name;
            new_city.pos.x = std::stof(x_str);
            new_city.pos.y = std::stof(y_str);
            new_city.population = std::stoi(population_str);

            // Проверка, существует ли страна с таким ID
            if (country_id < 0 || country_id >= countries.size())
            {
                return false; // Страна с таким ID не существует
            }

            // Поиск города с таким именем в векторе cities страны
            auto& cities = countries[country_id].cities;
            for (auto& existing_city : cities)
            {
                if (existing_city.name == new_city.name)
                {
                    existing_city = new_city; // Обновление города
                    return true;
                }
            }

            // Если города с таким именем не найдено, добавляем новый
            cities.push_back(new_city);
            return true;
        }

        bool deserialize_building(std::vector<country_data>& countries, const std::string& building_data)
        {
            std::istringstream building_stream(building_data);

            // Десериализация данных здания
            building new_building;
            std::string country_id_str, name, x_str, y_str, building_type_str, progress_str, endurance_str, size_converted_str;

            if (!(std::getline(building_stream, country_id_str, ',') &&
                std::getline(building_stream, name, ',') &&
                std::getline(building_stream, x_str, ',') &&
                std::getline(building_stream, y_str, ',') &&
                std::getline(building_stream, building_type_str, ',') &&
                std::getline(building_stream, progress_str, ',') &&
                std::getline(building_stream, endurance_str, ',') &&
                std::getline(building_stream, size_converted_str, ',')))
            {
                return false; // Невозможно разобрать данные
            }

            int country_id = std::stoi(country_id_str);
            new_building.name = name;
            new_building.pos.x = std::stof(x_str);
            new_building.pos.y = std::stof(y_str);
            new_building.building_type = std::stoi(building_type_str);
            new_building.progress_of_building = std::stoi(progress_str);
            new_building.endurance = std::stoi(endurance_str);
            new_building.size_converted_to_map = std::stoi(size_converted_str);

            // Проверка, существует ли страна с таким ID
            if (country_id < 0 || country_id >= countries.size())
            {
                return false; // Страна с таким ID не существует
            }

            // Поиск здания с таким именем и позицией в векторе buildings страны
            auto& buildings = countries[country_id].buildings;
            for (auto& existing_building : buildings)
            {
                if (existing_building.name == new_building.name && existing_building.pos.x == new_building.pos.x && existing_building.pos.y == new_building.pos.y)
                {
                    existing_building = new_building; // Обновление здания
                    return true;
                }
            }

            // Если здание с таким именем и позицией не найдено, добавляем новое
            buildings.push_back(new_building);
            return true;
        }

        std::vector< my_room > deserialize_rooms(const std::string& data)
        {
            std::vector< my_room > rooms;

            std::istringstream iss(data);

            std::string token;

            while (std::getline(iss, token, ';'))
            {
                my_room r;
                std::istringstream player_stream(token);

                std::string field;

                std::getline(player_stream, field, ','); r.name      = field;
                std::getline(player_stream, field, ','); r.unique_id = std::stoi(field);

                rooms.push_back(r);
            }
            return rooms;
        }


        // Функция для сериализации вектора объектов nuclear_strike_target в строку
        std::string serialize_targets(const std::vector<nuclear_strike_target>& targets) {
            std::ostringstream oss;
            for (const auto& target : targets) 
            {
                oss << target.unique_id << ","
                    << target.GETTER_country_id << ","
                    << target.GETTER_city_id << ","
                    << target.GETTER_building_id << ","
                    << target.GETTER_rocket << ","
                    << target.SENDER_country_id << ","
                    << target.step_of_bomb << ","
                    << target.SENDER_building_id << ","
                    << target.SENDER_unit << ","
                    << target.GETTER_unit << ";";
            }
            return oss.str();
        }

        // Функция для десериализации вектора объектов nuclear_strike_target из строки
        bool deserialize_targets(std::vector<nuclear_strike_target>& targets, const std::string& data) {
            std::istringstream iss(data);
            std::string segment;

            std::vector<nuclear_strike_target> new_targets;

            // Разделение строки на сегменты по `;`
            while (std::getline(iss, segment, ';')) {
                if (segment.empty()) continue;

                nuclear_strike_target target;
                std::istringstream segment_stream(segment);
                std::string field;

                if (std::getline(segment_stream, field, ',')) target.unique_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.GETTER_country_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.GETTER_city_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.GETTER_building_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.GETTER_rocket = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.SENDER_country_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.step_of_bomb = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.SENDER_building_id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.SENDER_unit = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.GETTER_unit = std::stoi(field);
                else return false;

                new_targets.push_back(target);
            }

            // Проверка наличия элементов в оригинальном векторе и их обновление или удаление
            auto it = targets.begin();
            while (it != targets.end()) {
                auto found = std::find_if(new_targets.begin(), new_targets.end(), [&it](const nuclear_strike_target& t) {
                    return 
                        t.unique_id == it->unique_id &&
                        t.GETTER_country_id == it->GETTER_country_id &&
                        t.GETTER_city_id == it->GETTER_city_id &&
                        t.GETTER_rocket == it->GETTER_rocket &&
                        t.GETTER_building_id == it->GETTER_building_id;
                    });

                if (found != new_targets.end()) {
                    it->SENDER_country_id = found->SENDER_country_id;
                    it->SENDER_building_id = found->SENDER_building_id;
                    it->step_of_bomb = found->step_of_bomb;
                    ++it;
                }
                else {
                    it = targets.erase(it); // Удаление элемента, если он не найден в новых данных
                }
            }

            // Добавление новых элементов, которых не было в оригинальном векторе
            for (const auto& target : new_targets) {
                auto found = std::find_if(targets.begin(), targets.end(), [&target](const nuclear_strike_target& t) {
                    return 
                        t.unique_id == target.unique_id &&
                        t.GETTER_country_id == target.GETTER_country_id &&
                        t.GETTER_city_id == target.GETTER_city_id &&
                        t.GETTER_rocket == target.GETTER_rocket &&
                        t.GETTER_building_id == target.GETTER_building_id;
                    });

                if (found == targets.end()) {
                    targets.push_back(target); // Добавление нового элемента
                }
            }

            return true;
        }

        // Функция для сериализации вектора объектов nuclear_strike_target в строку
        std::string serialize_bomb_step(const std::vector<nuclear_strike_target>& targets, int id) 
        {
            std::ostringstream oss;

            oss << id << ","
                << targets[id].step_of_bomb << ";";
            
            return oss.str();
        }

        bool deserialize_bomb_step(std::vector<nuclear_strike_target>& targets, const std::string& data) 
        {
            std::istringstream iss(data);
            std::string segment;

            nuclear_strike_target target;

            int id = 0;

            // Разделение строки на сегменты по `;`
            while (std::getline(iss, segment, ';')) {
                if (segment.empty()) continue;

                std::istringstream segment_stream(segment);
                std::string field;

                if (std::getline(segment_stream, field, ','))  id = std::stoi(field);
                else return false;
                if (std::getline(segment_stream, field, ',')) target.step_of_bomb = std::stoi(field);
                else return false;

            }
            
            if (targets[id].unique_id != 0)
            {
                targets[id].step_of_bomb = target.step_of_bomb;
            }

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

std::vector<std::string> got;
class ChatServer
{
public:

    ChatServer(boost::asio::io_context& io_context, const std::string& ip, short port, const std::string& nickname)
        : acceptor_(io_context, tcp::endpoint(boost::asio::ip::make_address(ip), port)), nickname_(nickname)
    {
        g_menu.players.push_back(player{ nickname, int(g_menu.players.size()), 0 });
        std::string serialized_data = "#CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
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
        std::string serialized_data = "#CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id );
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

                std::istream is(buffer.get());
                std::string buffer_message;
                std::getline(is, buffer_message);

                std::cout << "Полученные данные: [" << buffer_message << "]" << std::endl;

                // Если строка не начинается с #, ищем первый #
                size_t first_hash = buffer_message.find('#');
                if (first_hash != std::string::npos) {
                    buffer_message = buffer_message.substr(first_hash);
                }

                // Теперь строка точно начинается с #
                std::cout << "После очистки: [" << buffer_message << "]" << std::endl;

                std::vector<std::string> all_messages;
                std::istringstream socket_stream(buffer_message);
                std::string field;

                // Разделение строки по #
                while (std::getline(socket_stream, field, '#')) {
                    if (!field.empty()) {
                        all_messages.push_back(field);
                    }
                }

                // Если сообщений нет, добавляем всё как есть
                if (all_messages.empty()) {
                    all_messages.push_back(buffer_message);
                }


                for (int i = 0; i < all_messages.size(); i++)
                {
                    std::string message = all_messages[i];
                    if (server_client_space::IsRequest(message, "CLASS.PLAYERS:"))
                    {
                        std::string players_data = message.erase(0, 14);
                        server_client_space::server_client_menu_information::deserialize_players(g_menu.players, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_CITY:"))
                    {
                        std::string players_data = message.erase(0, 22);
                        server_client_space::server_client_menu_information::deserialize_city(g_map.countries, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_BUILDING:"))
                    {
                        std::string players_data = message.erase(0, 26);
                        server_client_space::server_client_menu_information::deserialize_building(g_map.countries, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_NUCLEAR_TARGETS:"))
                    {
                        std::string players_data = message.erase(0, 33);
                        server_client_space::server_client_menu_information::deserialize_targets(g_map.air_strike_targets, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.PLAYERS_VECTOR:"))
                    {
                        std::string players_data = message.erase(0, 21);
                        g_menu.players = server_client_space::server_client_menu_information::deserialize_vector_players(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.BOMBSTEP:")) //
                    {
                        std::string players_data = message.erase(0, 19);
                        server_client_space::server_client_menu_information::deserialize_bomb_step(g_map.air_strike_targets, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.UNIT:")) //
                    {
                        std::string players_data = message.erase(0, 11);
                        server_client_space::server_client_menu_information::deserialize_unit(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.UN_HEALTH:")) //
                    {
                        std::string players_data = message.erase(0, 16);
                        server_client_space::server_client_menu_information::deserialize_unit_health(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "USER.JOIN:"))
                    {
                        std::string        nickname = message.erase(0, 10);
                        add_client(socket, nickname);
                    }
                    else if (server_client_space::IsRequest(message, "U.P:"))
                    {
                        std::string players_data = message.erase(0, 4);
                        server_client_space::server_client_menu_information::deserialize_unit_pos(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "PLAYER.READY.LOBBY:"))
                    {
                        int id = std::stoi(message.erase(0, 19));
                        g_menu.players[id].ready_to_play = true;

                        std::string serialized_data = "#CLASS.PLAYERS" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
                        send_message(serialized_data);
                    }
                    else if (server_client_space::IsRequest(message, "PLAYER.NOTREADY.LOBBY:"))
                    {
                        int id = std::stoi(message.erase(0, 22));
                        g_menu.players[id].ready_to_play = false;

                        std::string serialized_data = "#CLASS.PLAYERS" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
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
        std::string serialized_data = "#CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
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

            std::string serialized_data = "#CLASS.PLAYERS_VECTOR:" + server_client_space::server_client_menu_information::serialize_vector_players(g_menu.players);
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

                    std::string message_for_nick = "#USER.JOIN:" + nickname_;
                    //send_message(message_for_nick);

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
        std::string serialized_data = "#CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
        send_message(serialized_data);
    }

private:

    void do_read()
    {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(socket_, *buffer, "\n", [this, buffer](boost::system::error_code ec, std::size_t) {
            if (!ec)
            {
                std::istream is(buffer.get());
                std::string buffer_message;
                std::getline(is, buffer_message);

                std::cout << "Полученные данные: [" << buffer_message << "]" << std::endl;

                // Если строка не начинается с #, ищем первый #
                size_t first_hash = buffer_message.find('#');
                if (first_hash != std::string::npos) {
                    buffer_message = buffer_message.substr(first_hash);
                }

                // Теперь строка точно начинается с #
                std::cout << "После очистки: [" << buffer_message << "]" << std::endl;

                std::vector<std::string> all_messages;
                std::istringstream socket_stream(buffer_message);
                std::string field;

                // Разделение строки по #
                while (std::getline(socket_stream, field, '#')) {
                    if (!field.empty()) {
                        all_messages.push_back(field);
                    }
                }

                // Если сообщений нет, добавляем всё как есть
                if (all_messages.empty()) {
                    all_messages.push_back(buffer_message);
                }

                for (int i = 0; i < all_messages.size(); i++)
                {
                    std::string message = all_messages[i];

                    if (server_client_space::IsRequest(message, "ROOMS:")) {
                        std::string players_data = message.erase(0, 6);
                        server_client_space::rooms = server_client_space::server_client_menu_information::deserialize_rooms(players_data);
                    }
                    if (server_client_space::IsRequest(message, "CLASS.PLAYERS:")) {
                        std::string players_data = message.erase(0, 14);
                        server_client_space::server_client_menu_information::deserialize_players(g_menu.players, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_CITY:"))
                    {
                        std::string players_data = message.erase(0, 22);
                        server_client_space::server_client_menu_information::deserialize_city(g_map.countries, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_BUILDING:"))
                    {
                        std::string players_data = message.erase(0, 26);
                        server_client_space::server_client_menu_information::deserialize_building(g_map.countries, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "GAME_CYCLE:TICK_UPDATE:"))
                    {
                        message.erase(0, 23);
                        g_map.global_tick = std::stoi(message);
                    }
                    else if (server_client_space::IsRequest(message, "GAME_CYCLE:GLOBAL_EVENT_UPDATE:"))
                    {
                        message.erase(0, 31);
                        g_map.game_events = std::stoi(message);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.BOMBSTEP:")) //
                    {
                        std::string players_data = message.erase(0, 19);
                        server_client_space::server_client_menu_information::deserialize_bomb_step(g_map.air_strike_targets, players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.UNIT:"))
                    {
                        std::string players_data = message.erase(0, 11);
                        server_client_space::server_client_menu_information::deserialize_unit(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.UN_HEALTH:")) //
                    {
                        std::string players_data = message.erase(0, 16);
                        server_client_space::server_client_menu_information::deserialize_unit_health(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "U.P:"))
                    {
                        std::string players_data = message.erase(0, 4);
                        server_client_space::server_client_menu_information::deserialize_unit_pos(players_data);
                    }
                    else if (server_client_space::IsRequest(message, "CLASS.MAP.UPDATE_NUCLEAR_TARGETS:"))
                    {
                        std::string players_data = message.erase(0, 33);
                        server_client_space::server_client_menu_information::deserialize_targets(g_map.air_strike_targets, players_data);
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
                    else if (server_client_space::IsRequest(message, "CHAT:"))
                    {
                        std::string players_data = message.erase(0, 5);
                        server_client_space::server_client_menu_information::add_message(players_data);
                    }
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

void socket_control::server_send_tick()
{
    sync_socket += ("#GAME_CYCLE:TICK_UPDATE:" + std::to_string(g_map.global_tick));
}

void socket_control::server_send_event()
{
    sync_socket += ("#GAME_CYCLE:GLOBAL_EVENT_UPDATE:" + std::to_string(g_map.game_events));
}

void socket_control::client_send_player_class(int id)
{
    std::string serialized_data = "#CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
    sync_socket += serialized_data;
}

void socket_control::server_send_player_class(int id)
{
    std::string serialized_data = "#CLASS.PLAYERS:" + server_client_space::server_client_menu_information::serialize_players(g_menu.players, id);
    sync_socket += serialized_data;
}

void socket_control::client_send_city(int city_id, int country_id)
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_CITY:" + server_client_space::server_client_menu_information::serialize_city(city_id, country_id);
    sync_socket += serialized_data;
}
void socket_control::server_send_city(int city_id, int country_id)
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_CITY:" + server_client_space::server_client_menu_information::serialize_city(city_id, country_id);
    sync_socket += serialized_data;
}

void socket_control::client_send_building(int building_id, int country_id)
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_BUILDING:" + server_client_space::server_client_menu_information::serialize_building(building_id, country_id);
    sync_socket += serialized_data;
}
void socket_control::server_send_building(int building_id, int country_id)
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_BUILDING:" + server_client_space::server_client_menu_information::serialize_building(building_id, country_id);
    sync_socket += serialized_data;
}

void socket_control::client_send_nuclear_targets()
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_NUCLEAR_TARGETS:" + server_client_space::server_client_menu_information::serialize_targets(g_map.air_strike_targets);
    sync_socket += serialized_data;
}
void socket_control::server_send_nuclear_targets()
{
    std::string serialized_data = "#CLASS.MAP.UPDATE_NUCLEAR_TARGETS:" + server_client_space::server_client_menu_information::serialize_targets(g_map.air_strike_targets);
    sync_socket += serialized_data;
}

void socket_control::server_update_bomb_step(int unique_id)
{
    std::string serialized_data = "#CLASS.MAP.BOMBSTEP:" + server_client_space::server_client_menu_information::serialize_bomb_step(g_map.air_strike_targets, unique_id);
    sync_socket += serialized_data;
}

void socket_control::server_send_unit(int id)
{
    std::string serialized_data = "#CLASS.UNIT:" + server_client_space::server_client_menu_information::serialize_unit(id);
    sync_socket += serialized_data;
}
void socket_control::client_send_unit(int id)
{
    std::string serialized_data = "#CLASS.UNIT:" + server_client_space::server_client_menu_information::serialize_unit(id);
    sync_socket += serialized_data;
}

void socket_control::server_send_unit_health(int id)
{
    std::string serialized_data = "#CLASS.UN_HEALTH:" + server_client_space::server_client_menu_information::serialize_unit_health(id);
    sync_socket += serialized_data;
}
void socket_control::client_send_unit_health(int id)
{
    std::string serialized_data = "#CLASS.UN_HEALTH:" + server_client_space::server_client_menu_information::serialize_unit_health(id);
    sync_socket += serialized_data;
}

void socket_control::server_send_unit_pos(int region)
{
    std::string serialized_data = "#U.P:" + server_client_space::server_client_menu_information::serialize_unit_pos(region);
    sync_socket += serialized_data;
}
void socket_control::client_send_unit_pos(int region)
{
    std::string serialized_data = "#U.P:" + server_client_space::server_client_menu_information::serialize_unit_pos(region);
    sync_socket += serialized_data;
}

void socket_control::server_process_client_sync()
{
    if (!sync_socket.empty())
        server->send_message(sync_socket);

    sync_socket = "";
}

void socket_control::client_process_client_sync()
{
    if (!sync_socket.empty())
        client->send_message(sync_socket);

    sync_socket = "";
}

void menu::render(window_profiling window) 
{
    if (!game_sound_system::fmod_init)
    {
        game_sound_system::fmod_init = true;
    }

    ImGui::GetStyle().Colors[ImGuiCol_WindowBg]         = ImColor(0, 0, 0, 0); 
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Button]           = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]     = ImColor(35, 35, 35);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]    = ImColor(15, 15, 15);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg]          = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]   = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]    = ImColor(0, 0, 0);
    ImGui::GetStyle().Colors[ImGuiCol_Border]           = ImColor(79, 255, 69, 70);
    ImGui::GetStyle().Colors[ImGuiCol_Separator]        = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_ChildBg]          = ImColor(5, 5, 5, 0);
    ImGui::GetStyle().Colors[ImGuiCol_CheckMark]        = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab]    = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 200);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImColor(79, 255, 69, 130);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImColor(79, 255, 69, 200);

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
        ERROR_MENU,
        SETTINGS_MENU
    };

    static bool connected_to_central_server;

    if (!connected_to_central_server)
    {
        std::thread(run_client, "127.0.0.1", 1287, "User1").detach();
        connected_to_central_server = true;
    }

    //game global scenes
    switch (game_scenes_params::global_game_scene_tab)
    {

        case game_scenes_params::global_game_tabs::menu:
        {

            for (int i = 0; i < got.size(); i++)
            {
                ImGui::GetForegroundDrawList()->AddText(ImVec2(100, 10 + 15 * i), ImColor(255, 255, 255), got[i].c_str());
            }

            ImGui::GetBackgroundDrawList()->AddImage(
                (ImTextureID)g_window.g_pTextureView,
                ImVec2(0, 0),
                ImVec2(screen_x, screen_y),
                ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 210)
            );

            //main menu window setup
            ImGui::SetNextWindowPos(ImVec2(30, screen_y / 2 - 400 / 2));
            ImGui::SetNextWindowSize(ImVec2(430, 590));

            //menu
            static bool r = true;
            ImGui::Begin("Defcon 2.0", &r, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);
            //main menu code
            switch (game_scenes_params::main_menu_tabs)
            {
                case  MAIN_MENU: {

                    ImGui::Text("Nickname");
                    ImGui::NewLine();
                    ImGui::InputText("  ", nickname, 128);
                    ImGui::NewLine();

                    if (ImGui::Button("Create Server", ImVec2(380, 35))) {
                        game_scenes_params::main_menu_tabs = 1;
                    }
                    if (ImGui::Button("Join Server", ImVec2(380, 35))) {
                        game_scenes_params::main_menu_tabs = 3;
                    }
                    if (ImGui::Button("Settings", ImVec2(380, 35))) {
                        game_scenes_params::main_menu_tabs = SETTINGS_MENU;
                    }
                    if (ImGui::Button("Exit", ImVec2(380, 35))) {
                        PostQuitMessage(0);
                    }
                    break;
                }
                case  SERVER_CREATE: {

                    static char server_name [128] = "My Server";

                    ImGui::InputText("Server name", server_name, 128);

                    if (ImGui::Button("Create Server", ImVec2(380, 35)))
                    {
                        //game_scenes_params::main_menu_tabs = 2;
                        server_client_space::server_client_menu_information::server_nickname = nickname;
                        client->send_message
                        (std::string(
                            "c.s:create_room:" + std::string(server_name)

                        )
                        );
                        game_scenes_params::main_menu_tabs = 2;
                    }

                    if (ImGui::Button("Back"))
                    {
                        game_scenes_params::main_menu_tabs = MAIN_MENU;
                    }
                    break;
                }
                case  SERVER_MENU:
                {
                    //ImGui::Text((std::string("Server is running on IP ") + server_ip).c_str());

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
                        client->send_message("CHAT:" + server_client_space::server_client_menu_information::server_nickname + ": " + std::string(message));
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

                        client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

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

                            client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

                        }

                        if (!ready_for_game)
                        {
                            players[server_client_space::server_client_menu_information::
                                find_player_by_nickname
                                (server_client_space::server_client_menu_information::server_nickname).id].ready_to_play = false;

                            client->send_and_update_player_class(server_client_space::server_client_menu_information::find_player_by_nickname(server_client_space::server_client_menu_information::server_nickname).id);

                        }

                        last_ready_for_game = ready_for_game;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(313);
                    if (ImGui::Button("Disconnect"))
                    {
                        client->send_message("c.s:close_room");
                        io_context->stop();
                        game_scenes_params::main_menu_tabs = 0;
                    }

                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 - 400 / 2) - 400, screen_y / 2 - 400 / 2));
                    ImGui::SetNextWindowSize(ImVec2(400, 106));

                    ImGui::Begin("Server game settings");
                    {
                        static int last_selected_game_mode = 0;
                        const char* game_modes_selection[] = { "Default (30 mins)", "Long (45 mins)" };
                        ImGui::Combo("Game mode", &selected_game_mode, game_modes_selection, IM_ARRAYSIZE(game_modes_selection));
                        if (last_selected_game_mode != selected_game_mode) {
                            client->send_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            server_client_space::server_client_menu_information::add_message("Server : game mode changed to " + std::string(game_modes_selection[selected_game_mode]) + "/blue/");
                            last_selected_game_mode = selected_game_mode;
                        }

                        if (all_players_are_ready)
                        if (ImGui::Button("Start Game!", ImVec2(380, 35)))
                        {
                            g_socket_control.player_role = g_socket_control.player_role_enum::SERVER;
                            game_scenes_params::global_game_scene_tab = game_scenes_params::game_process;
                            client->send_message("#SERVER:GAME_START");
                        }
                    }
                    ImGui::End();


                    ImGui::SetNextWindowPos(ImVec2((screen_x / 2 + 1200) - 500, screen_y / 2 - 400 / 2));
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

                    static int selected_room = -1;

                    ImGui::BeginListBox("Servers", ImVec2(380, 100));
                    {
                        for (int i = 0; i < server_client_space::rooms.size(); ++i)
                        {
                            bool is_selected = (selected_room == i); 
                            if (ImGui::Selectable(server_client_space::rooms[i].name.c_str(), is_selected))
                            {
                                selected_room = i;
                            }
                        }
                    }
                    ImGui::EndListBox();


                    if (ImGui::Button("Update", ImVec2(380, 35)))
                    {
                        client->send_message("c.s:get_rooms");
                    }

                    if (ImGui::Button("Join Server", ImVec2(380, 35)))
                    {
                        game_scenes_params::main_menu_tabs = 4;
                    }

                    if (ImGui::Button("Back"))
                    {
                        game_scenes_params::main_menu_tabs = MAIN_MENU;
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
                    break;
                }
                case SETTINGS_MENU:
                {
                    static int selected_mode_of_resolution;
                    static int last_selected_mode_of_resolution = 0;
                    const char* game_modes_selection[] = { "Auto (Screen Size)", "1280x720 (HD Ready)", "1366x768 (HD)", "1920x1080 (Full HD)", "2560x1440 (QHD)", "3840x2160 (4K)" };
                    ImGui::Combo("Resolution", &selected_mode_of_resolution, game_modes_selection, IM_ARRAYSIZE(game_modes_selection));
                    if (selected_mode_of_resolution != last_selected_mode_of_resolution)
                    {
                        last_selected_mode_of_resolution = selected_mode_of_resolution;
                        std::vector<std::pair<int, int>> resolutions = {  
                            {1280, 720},
                            {1366, 768},
                            {1920, 1080},
                            {2560, 1440},
                            {3840, 2160},
                        };

                        static int width, height;

                        if (selected_mode_of_resolution == 0)
                        {
                            HDC hDCScreen = GetDC(NULL);

                            int width = GetDeviceCaps(hDCScreen, HORZRES);
                            int height = GetDeviceCaps(hDCScreen, VERTRES);

                            ReleaseDC(NULL, hDCScreen);
                        }
                        else
                        {
                            width  = resolutions[selected_mode_of_resolution - 1].first;
                            height = resolutions[selected_mode_of_resolution - 1].second;
                        }

 
                    }

                    if (ImGui::Button("Back"))
                        game_scenes_params::main_menu_tabs = 0;

                    break;
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
