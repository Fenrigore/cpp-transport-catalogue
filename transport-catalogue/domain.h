#pragma once
#include <string>
#include <vector>
#include "geo.h"
#include <stdexcept>
#include <optional>

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
		//кол-во остановок маршрута
		size_t count{};
		//кол-во уникальных остановок маршрута
		size_t unique{};
		//геодезическое расстояние для расчета кривизны
		double geo_distance{};
		//сумма заданных расстояний между остановками маршрута
		int route_distance{};
		//кривизна маршрута
		double curvature{};
	};

	struct RouteSettings {
		RouteSettings() = default;
		RouteSettings(int speed, int waiting_time);

		int bus_velocity{};
		int bus_wait_time{};
	};

	enum class ItemType {
		Bus,
		Wait
	};

	struct Item {
		Item(ItemType type_of_item
			, std::string item_name
			, double travel_or_waiting_time
			, std::optional<int> span_count = std::nullopt);

		ItemType type{};
		std::string name{};
		double time{};
		std::optional<int> span_count_for_bus{};
	};
}