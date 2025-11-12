#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "geo.h"
#include "transport_catalogue.h"


namespace ireader {
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command{};      // Название команды
        std::string id{};           // id маршрута или остановки
        std::string description{};  // Параметры команды
    };

    struct DirtyDistanceCommand {
        //тут хранится название стартовой остановки
        std::string start_stop_name;
        //здесь будет храниться D1m to stop1, D2m to stop2, ...
        std::string all_distances;
    };

    struct StopPairDistance {
        std::string_view from;
        std::string_view to;
        int distance;
    };

    class InputReader {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        void Read(std::istream& input, catalogue::TransportCatalogue& catalogue);
        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(catalogue::TransportCatalogue& catalogue) const;

        void ClearDistanceData();

    private:
        std::vector<CommandDescription> create_stops_commands_;
        std::vector<CommandDescription> create_buses_commands_;
        std::vector<DirtyDistanceCommand> create_distance_commands_;
        std::vector<StopPairDistance> distance_data_;
    };
}