#include "transport_catalogue.h"
#include "geo.h"
#include <unordered_set>
#include <algorithm>

#include <iostream>

int catalogue::TransportCatalogue::ComputeRouteDistance(std::string_view from, std::string_view to) const {
	//получаю указатели на остановки
	const Stop* first_stop = FindStop(from);
	const Stop* second_stop = FindStop(to);
	if (!first_stop || !second_stop) {
		return 0;
	}
	//ищу остановки в хэш таблице дистанций
	auto distance_iter = distances_between_stops_.find({ first_stop, second_stop });
	//если не нашлось расстояние от А до Б, то проверяю есть ли от Б до А
	if (distance_iter == distances_between_stops_.end()) {
		distance_iter = distances_between_stops_.find({ second_stop, first_stop });
		//если и такого расстояния нет, значит нет
		if (distance_iter == distances_between_stops_.end()) {
			return 0;
		}
	}
	return distance_iter->second;
}

void catalogue::TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates){
	//добавляю в дэк сотановок новую остановку
	stops_.push_back({ std::string(name),std::move(coordinates)});
	//беру имя остановки, задаю его как ключ для хэш таблицы остановок
	//и для этого ключа передаю значение (указатель на остановку в дэке)
	stop_indexes_by_name_[stops_.back().name] = &stops_.back();
}

void catalogue::TransportCatalogue::AddBus(std::string_view route, const std::vector<std::string_view>& stops){
	std::vector<Stop*> temp_stops{}; //создаю временный вектор указателей на остановки
	for (const std::string_view& stop : stops) { //для каждого названия остановки
		//проверяю есть ли остановка с таким названием в хэше остановок
		if (auto index_it = stop_indexes_by_name_.find(stop); index_it != stop_indexes_by_name_.end()) {
			//если есть, передаю указатель на остановку во временный вектор
			temp_stops.push_back(index_it->second);
		}
	}
	//подготовил временный вектор, остальные данные уже есть. Создаю маршрут
	Bus temp_bus(std::string(route), std::move(temp_stops));
	//передаю его в дэк маршрутов
	buses_.push_back(std::move(temp_bus));
	//беру имя маршрута, задаю его как ключ для хэш таблицы маршрутов
	//и для этого ключа передаю указатель на маршрут
	Bus* current_bus = &buses_.back();
	bus_indexes_by_name_[buses_.back().route] = current_bus;
	//Запоминаю для всех существующих автобусов, что они содержатся в этом маршруте
	for (const auto stop : buses_.back().stops) {
		routes_containing_stop_[stop->name].insert(current_bus->route);
	}
}

const catalogue::Bus* catalogue::TransportCatalogue::FindBus(std::string_view route) const {
	auto it = bus_indexes_by_name_.find(route);
	if (it == bus_indexes_by_name_.end()) {
		return nullptr;
	}
	return it->second;
}

const catalogue::Stop* catalogue::TransportCatalogue::FindStop(std::string_view name) const {
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
	if (routes_containing_stop_it == routes_containing_stop_.end()){
		return nullptr;
	}
	//если нашёл, то передаю ссылку на список маршрутов, в которых есть эта остановка
	return &(routes_containing_stop_it->second);
}

catalogue::BusInfo catalogue::TransportCatalogue::GetBusInfo(std::string_view route) const {
	//получаю автобус
	const Bus* bus = FindBus(route);
	if (bus == nullptr) {
		return { 0,0,0., 0};
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
			double current_distance = ComputeDistance((*it)->coordinates, (*(it + 1))->coordinates);
			geo_distance += current_distance;
			route_distance += ComputeRouteDistance((*it)->name, (*(it + 1))->name);
		}
	}
	return { bus->stops.size(),unique.size(), geo_distance,  route_distance };
}


void catalogue::TransportCatalogue::AddDistance(std::string_view from, std::string_view to, int distance) {
	const Stop* const stop_a = FindStop(from);
	const Stop* const stop_b = FindStop(to);
	if (stop_a && stop_b) {
		distances_between_stops_[{ stop_a, stop_b }] = distance;
	}
}