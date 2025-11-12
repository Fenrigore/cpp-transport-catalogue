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

	struct BusInfo {
		BusInfo(size_t stops_count, size_t unique_stops, double geo_distance_in, int route_distance_in)
			:count{ stops_count },
			unique{ unique_stops },
			geo_distance{ geo_distance_in },
			route_distance{ route_distance_in } {
			if (geo_distance_in > 0 && route_distance_in > 0) {
				curvature = route_distance_in / geo_distance_in;
			}
		}
		size_t count{};
		size_t unique{};
		double geo_distance{};
		int route_distance{};
		double curvature{};
	};

	//тут всё обконстантил, т.к. и в unordered_map всё переконстантил
	template <typename T>
	struct DistanceHasher {
		size_t operator()(const std::pair<const T* const, const T* const >& stops)  const {
			return std::hash<const T*>{}(stops.first) + std::hash<const T*>{}(stops.second) * 47;
		}
	};

	class TransportCatalogue {
	private:
		int ComputeRouteDistance(std::string_view from, std::string_view to) const ;
	public:
		void AddStop(std::string_view name, geo::Coordinates coordinates);
		void AddBus(std::string_view route, const std::vector<std::string_view>& stops);
		const Bus* FindBus(std::string_view route) const;
		const Stop* FindStop(std::string_view name)const;
		const std::set<std::string_view>* FindRoutes(std::string_view name) const;
		BusInfo GetBusInfo(std::string_view route) const;
		void AddDistance(std::string_view from, std::string_view to, int distance);

	private:
		std::deque<Stop> stops_{};
		std::unordered_map< std::string_view, Stop*> stop_indexes_by_name_{};
		std::deque<Bus> buses_{};
		std::unordered_map<std::string_view, Bus*> bus_indexes_by_name_{};
		std::unordered_map< std::string_view, std::set<std::string_view>> routes_containing_stop_{};
		//а тут всё переконстантил потому, что ни указатель, ни значение, на которое он указывает, менять не буду и нельзя вообше
		//Плюс из FindStop получаю константный указатель
		std::unordered_map<std::pair<const Stop* const, const Stop* const>, int, DistanceHasher<Stop>> distances_between_stops_{};
	};
}