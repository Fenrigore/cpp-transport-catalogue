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
        
        catalogue::BusInfo info = transport_catalogue.GetBusInfo(requested_name);

        if (info.count == 0) {
            //Bus 751: not found
            output << "Bus " << requested_name << ": not found" << std::endl;
        }
        else {
            //Bus X: R stops on route, U unique stops, L route length 
            //Bus X: R stops on route, U unique stops, L route length, C curvature
            output << "Bus " << requested_name << ": " << info.count
                << " stops on route, " << info.unique << " unique stops, "
                << info.route_distance << " route length, " <<
                info.curvature << " curvature" << std::endl;
        }
    }
    else if (type_of_request == "Stop") {
        //вывести список маршрутов, у которых в stops_ есть эта остановка.
        //получаю указатель на список маршрутов
        const std::set<std::string_view>* routes_with_this_stop = transport_catalogue.FindRoutes(requested_name);

        //если он нулевой
        if (routes_with_this_stop == nullptr) {
            //проверяю есть ли такая остановка вообще
            if (transport_catalogue.FindStop(requested_name)) {
                //если такая остановка существует, то она не принадлежит никакому маршруту
                //Stop X: no buses
                output << "Stop " << requested_name << ": no buses" << std::endl;
            }
            else {
                //либо её реально нет
                //Stop X: not found
                output << "Stop " << requested_name << ": not found" << std::endl;
            }
        }
        else {
            //если мы тут, то в списке есть маршруты. Выводим их все
                //Stop X: buses bus1 bus2 ... busN 
                output << "Stop " << requested_name << ": buses";
                for (const auto& bus_name : *routes_with_this_stop) {
                    output << " " << bus_name;
                }
                output << std::endl;
        }
    }
}

void sreader::Read(std::istream& input, std::ostream& output, catalogue::TransportCatalogue& catalogue) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}