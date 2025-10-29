#pragma once
#include <deque>
#include <unordered_map>
#include<string>
#include <vector>
#include <string_view>
#include <set>
#include "geo.h"

namespace catalogue {
	/// <summary>
	/// Структура остановки. Включает в себя:
	/// - название
	/// - координаты
	/// </summary>
	struct Stop
	{
		//Остановка должна создаваться только с параметрами, так как болванок не будет,
		//потому что input reader добавляет их через буфер: сначала все остановки, потом
		//все маршруты
		Stop(std::string stop_name, geo::Coordinates stop_coordinates)
			:name{ stop_name }, coordinates{ stop_coordinates } {
		}
		std::string name{}; //имя остановки
		geo::Coordinates coordinates{};
	};

	/// <summary>
	/// Структура автобусного маршрута. Содержит
	/// - название маршрута
	/// - ссылки на остановки на маршруте
	/// - является ли маршрут круговым
	/// </summary>
	struct Bus {
		Bus(std::string bus_route, std::vector<Stop*> bus_stops)
			:route{ bus_route }, stops{ bus_stops } {
		}
		std::string route{};
		std::vector<Stop*> stops{};
	};

	class TransportCatalogue {
	public:
		void AddStop(std::string_view name, geo::Coordinates coordinates);
		void AddBus(std::string_view route, const std::vector<std::string_view>& stops);
		Bus* FindBus(std::string_view route) const;
		Stop* FindStop(std::string_view name)const;
		std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;
		std::tuple<size_t, size_t, double> GetBusInfo(std::string_view route) const;


	private:
		std::deque<Stop> stops_{};
		std::unordered_map< std::string_view, Stop*> stop_indexes_by_name_{};
		std::deque<Bus> buses_{};
		std::unordered_map<std::string_view, Bus*> bus_indexes_by_name_{};
	};
}

