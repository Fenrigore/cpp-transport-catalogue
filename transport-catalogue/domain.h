#pragma once
#include <string>
#include <vector>
#include "geo.h"
#include <stdexcept>

namespace domain {
	struct Stop
	{
		Stop(std::string stop_name, geo::Coordinates stop_coordinates);
		std::string name{}; //имя остановки
		geo::Coordinates coordinates{};
	};

	struct Bus {
		Bus(std::string bus_route, std::vector<domain::Stop*> bus_stops, bool circle);
			
		std::string route{};
		std::vector<domain::Stop*> stops{};
		bool is_circle{};
	};

	struct BusInfo {
		BusInfo(size_t stops_count, size_t unique_stops, double geo_distance_in, int route_distance_in);
			
		size_t count{};
		size_t unique{};
		double geo_distance{};
		int route_distance{};
		double curvature{};
	};

}