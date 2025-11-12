#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
namespace redact{
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return { lat, lng };
    }

/**
 * Удаляет пробелы в начале и конце строки
 */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
    * Парсит маршрут.
    * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
    * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
    */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }
}


ireader::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }
    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void ireader::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        //Если Stop, то надо проверить кол-во запятых. Если запятая одна, то делаем то же самое, если запятых больше
        //обрезаю description до второй запятой, левую часть кладу в 
        if (command_description.command == "Stop") {

            size_t count_of_commas = std::count(command_description.description.begin(),
                command_description.description.end(), ',');
            //если запятых больше 1, значит в запросе есть дистанции
            if (count_of_commas > 1) {
                //копирую текст из description. Если забрать его через move, а в дальнейшем выбросится исключение,
                //то всё потеряется
                std::string temp_string = command_description.description;
                size_t comma = 0;
                auto second_comma_it = std::find_if(temp_string.begin(), temp_string.end(), [&comma](const char& ch) {
                        if (ch == ',') { //если символ это запятая, прибавляю счетчик 
                            if (++comma == 2) {//если счетчик равен 2 после прибавления, то говорю что это оно
                                return true;
                            }
                        }
                        return false;   //если предыдущие не отработали то говорю что идём дальше
                    });
                //возвращаю в description только координаты
                command_description.description = std::string(temp_string.begin(), second_comma_it);
                //добавляю в create_distance_commands_ структуру Distance с именем command_description.id 
                // и текстом после второй запятой
                create_distance_commands_.push_back({command_description.id, 
                    std::string(second_comma_it + 1, temp_string.end()) });
            }
                //Разобрался с запятыми и теперь добавляю остановку в очередь создания остановок
                create_stops_commands_.push_back(std::move(command_description));
        }
        else {
            create_buses_commands_.push_back(std::move(command_description));
        }
    }
}

void ireader::InputReader::Read(std::istream& input, catalogue::TransportCatalogue& catalogue){
    int base_request_count;
    input >> base_request_count >> std::ws;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseLine(line);
    }
    ClearDistanceData();
    ApplyCommands(catalogue);
}

void ireader::InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    for (const auto& com : create_stops_commands_) { //для каждой сырой команды в векторе команд создания остановки
        catalogue.AddStop(redact::Trim(com.id), redact::ParseCoordinates(com.description)); //передаю чистое имя и координаты
    }
    for (const auto& com : create_buses_commands_) { //для каждой сырой команды в векторе команд создания маршрута
        catalogue.AddBus(redact::Trim(com.id), redact::ParseRoute(com.description)); //передаю очищенное имя и разделенные остановки
    }
    for (const auto& data : distance_data_) {
        catalogue.AddDistance(data.from, data.to, data.distance);
    }

}

void ireader::InputReader::ClearDistanceData() {
    //7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
    //между расстоянием и названием остановки всегда находится "m to"
    std::string separator_for_distance = "m to";
    //для каждой грязной команды создания дистанций
    for (const auto& dirty : create_distance_commands_) {
        //разделяю все блоки дистанций в вектор
        auto separate_distance_commands = redact::Split(dirty.all_distances, ',');
        for (auto to_separate : separate_distance_commands) {
            //Нахожу первый символ разделителя
            auto separator = to_separate.find(separator_for_distance);
            //из string_view нельзя (или не знаю как) преобразовать и int, зато из string можно
            std::string meters(to_separate.substr(0, separator));
            int distance = std::stoi(meters);
            //адрес начинается с позиции разделителя + размера разделителя
            std::string_view adress(to_separate.substr(separator + separator_for_distance.size()));
            
            distance_data_.push_back({ redact::Trim(dirty.start_stop_name), redact::Trim(adress) , distance });
        }

    }
}