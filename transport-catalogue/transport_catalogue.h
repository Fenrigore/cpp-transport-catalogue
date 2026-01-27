#pragma once

#include <deque>
#include <unordered_map>
#include<string>
#include <vector>
#include <string_view>
#include <set>
#include "geo.h"
#include "domain.h"

namespace catalogue {

	template <typename T>
	struct DistanceHasher {
		size_t operator()(const std::pair<const T* const, const T* const >& stops)  const {
			return std::hash<const T*>{}(stops.first) + std::hash<const T*>{}(stops.second) * 47;
		}
	};

	class TransportCatalogue {
	public:
		int GetRouteDistance(const domain::Stop* from, const domain::Stop* to)const;
		double GetGeoDistance(const domain::Stop* from, const domain::Stop* to) const;
		void AddStop(std::string_view name, geo::Coordinates coordinates);
		void AddBus(std::string_view route, const std::vector<std::string_view>& stops, bool is_circle_route);
		const domain::Bus* FindBus(std::string_view route) const;
		const domain::Stop* FindStop(std::string_view name)const;
		const std::set<std::string_view>* FindRoutes(std::string_view name) const;
		domain::BusInfo GetBusInfo(std::string_view route) const;
		void AddDistance(std::string_view from, std::string_view to, int distance);
		std::vector<const domain::Bus*> GetAllBuses() const;
		const std::deque<domain::Stop>& GetAllStops() const;
	private:
		std::deque<domain::Stop> stops_{};
		std::unordered_map< std::string_view, domain::Stop*> stop_indexes_by_name_{};
		std::deque<domain::Bus> buses_{};
		std::unordered_map<std::string_view, domain::Bus*> bus_indexes_by_name_{};
		std::unordered_map< std::string_view, std::set<std::string_view>> routes_containing_stop_{};
		//а тут всё переконстантил потому, что ни указатель, ни значение, на которое он указывает, менять не буду и нельзя вообше
		//Плюс из FindStop получаю константный указатель
		mutable std::unordered_map<std::pair<const domain::Stop* const
			, const domain::Stop* const>
			, int
			, DistanceHasher<domain::Stop>> distances_between_stops_{};
		mutable std::unordered_map<std::pair<const domain::Stop* const
			, const domain::Stop* const>
			, double
			, DistanceHasher<domain::Stop>> geo_distances_between_stops_{};
	};
}