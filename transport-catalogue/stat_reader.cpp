#include "stat_reader.h"
#include <sstream>
#include <string>
#include <tuple>
#include <iomanip>

void sreader::ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    auto space_pos = request.find(' ');
    //если не нашелся пробел значит введен неправильный запрос
    if (space_pos == std::string_view::npos) {
        return;
    }
    std::string_view type_of_request(request.substr(0, space_pos));
    std::string_view requested_name(request.substr(space_pos + 1));
    // обратобка запроса
    if (type_of_request == "Bus") {
        
        std::tuple<size_t, size_t, double> info = transport_catalogue.GetBusInfo(requested_name);
        auto [r, u, l] = info;

        if (r == 0) {
            //Bus 751: not found
            output << "Bus " << requested_name << ": not found" << std::endl;
        }
        else {
            //Bus X: R stops on route, U unique stops, L route length 
            output << "Bus " << requested_name << ": " << r
                << " stops on route, " << u << " unique stops, "
                << std::setprecision(6) << l << " route length" << std::endl;
        }
    }
    else if (type_of_request == "Stop") {
        //вывести список маршрутов, у которых в stops_ есть эта остановка.
        bool is_stop_found = transport_catalogue.FindStop(requested_name);
        if (is_stop_found) {
            std::set<std::string_view> founded_buses = transport_catalogue.GetBusesByStop(requested_name);
            if (founded_buses.size() == 0) {
                //Stop X: no buses
                output << "Stop " << requested_name << ": no buses" << std::endl;
            }
            else {
                //Stop X: buses bus1 bus2 ... busN 
                output << "Stop " << requested_name << ": buses";
                for (const auto& bus_name : founded_buses) {
                    output << " " << bus_name;
                }
                output << std::endl;
            }
        }
        else {
            //Stop X: not found
            output << "Stop " << requested_name << ": not found" << std::endl;
        }
    }
}


