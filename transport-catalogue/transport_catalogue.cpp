#include "transport_catalogue.h"
#include "geo.h"
#include <unordered_set>
#include <algorithm>

#include <iostream>

int catalogue::TransportCatalogue::GetRouteDistance(const domain::Stop* from, const domain::Stop* to) const
{
	int route_distance{};
	auto route_distance_iter = distances_between_stops_.find({ from, to });
	if (route_distance_iter != distances_between_stops_.end()) {
		route_distance = route_distance_iter->second;
	}
	else {
		route_distance_iter = distances_between_stops_.find({ to, from });
		if (route_distance_iter != distances_between_stops_.end()) {
			route_distance = route_distance_iter->second;
		}
	}
	return route_distance;
}

double catalogue::TransportCatalogue::GetGeoDistance(const domain::Stop* from, const domain::Stop* to) const
{
	double geo_distance{};
	auto geo_distance_iter = geo_distances_between_stops_.find({ from, to });
	if (geo_distance_iter != geo_distances_between_stops_.end()) {
		geo_distance = geo_distance_iter->second;
	}
	else {
		geo_distance = geo_distances_between_stops_[{ from, to }]
			= geo_distances_between_stops_[{ to, from }]
			= geo::ComputeDistance(from->coordinates, to->coordinates);
	}
	return geo_distance;
}

void catalogue::TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates) {
	//добавляю в дэк сотановок новую остановку
	stops_.push_back({ std::string(name),std::move(coordinates) });
	//беру имя остановки, задаю его как ключ для хэш таблицы остановок
	//и для этого ключа передаю значение (указатель на остановку в дэке)
	stop_indexes_by_name_[stops_.back().name] = &stops_.back();
}

void catalogue::TransportCatalogue::AddBus(std::string_view route, const std::vector<std::string_view>& stops, bool is_circle_route) {
	std::vector<domain::Stop*> temp_stops{}; //создаю временный вектор указателей на остановки
	for (const std::string_view& stop : stops) { //для каждого названия остановки
		//проверяю есть ли остановка с таким названием в хэше остановок
		if (auto index_it = stop_indexes_by_name_.find(stop); index_it != stop_indexes_by_name_.end()) {
			//если есть, передаю указатель на остановку во временный вектор
			temp_stops.push_back(index_it->second);
		}
	}
	//подготовил временный вектор, остальные данные уже есть. Создаю маршрут
	domain::Bus temp_bus(std::string(route), std::move(temp_stops), is_circle_route);
	//передаю его в дэк маршрутов
	buses_.push_back(std::move(temp_bus));
	//беру имя маршрута, задаю его как ключ для хэш таблицы маршрутов
	//и для этого ключа передаю указатель на маршрут
	domain::Bus* current_bus = &buses_.back();
	bus_indexes_by_name_[buses_.back().route] = current_bus;
	//Запоминаю для всех существующих автобусов, что они содержатся в этом маршруте
	for (const auto stop : buses_.back().stops) {
		routes_containing_stop_[stop->name].insert(current_bus->route);
	}
}

const domain::Bus* catalogue::TransportCatalogue::FindBus(std::string_view route) const {
	auto it = bus_indexes_by_name_.find(route);
	if (it == bus_indexes_by_name_.end()) {
		return nullptr;
	}
	return it->second;
}

const domain::Stop* catalogue::TransportCatalogue::FindStop(std::string_view name) const {
	auto it = stop_indexes_by_name_.find(name);
	if (it == stop_indexes_by_name_.end()) {
		return nullptr;
	}
	return it->second;
}

const std::set<std::string_view>* catalogue::TransportCatalogue::FindRoutes(std::string_view name) const {
	//ищу остановку в routes_containing_stop_
	auto routes_containing_stop_it = routes_containing_stop_.find(name);
	//если не нашел, то остановки нет и передаю nullptr
	if (routes_containing_stop_it == routes_containing_stop_.end()) {
		return nullptr;
	}
	//если нашёл, то передаю ссылку на список маршрутов, в которых есть эта остановка
	return &(routes_containing_stop_it->second);
}

domain::BusInfo catalogue::TransportCatalogue::GetBusInfo(std::string_view route) const {
	//получаю автобус
	const domain::Bus* bus = FindBus(route);
	if (bus == nullptr) {
		return { 0,0,0., 0 };
	}
	//переменная для получения уникальных остановок
	std::unordered_set<std::string_view> unique;
	//и для подсчета дистанции
	double geo_distance{};
	int route_distance{};
	//добавляю каждое название остановки в unordered_set
	//в нём останутся только уникальные
	for (auto it = bus->stops.begin(); it != bus->stops.end(); ++it) {
		unique.insert((*it)->name);
		//тут считаю расстояния географические
		if (it + 1 != bus->stops.end()) {
			route_distance += GetRouteDistance(*it, *(it + 1));
			geo_distance += GetGeoDistance(*it, *(it + 1));
		}
	}
	return { bus->stops.size(),unique.size(), geo_distance,  route_distance };
}

void catalogue::TransportCatalogue::AddDistance(std::string_view from, std::string_view to, int distance) {
	const domain::Stop* const stop_a = FindStop(from);
	const domain::Stop* const stop_b = FindStop(to);
	if (stop_a && stop_b) {
		distances_between_stops_[{ stop_a, stop_b }] = distance;
	}
}

std::vector<const domain::Bus*> catalogue::TransportCatalogue::GetAllBuses() const {
	std::vector<const domain::Bus*> buses{};
	for (const auto& para : bus_indexes_by_name_) {
		buses.push_back(para.second);
	}
	return buses;
}

const std::deque<domain::Stop>& catalogue::TransportCatalogue::GetAllStops()const {
	return stops_;
}
