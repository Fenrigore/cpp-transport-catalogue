#include "transport_router.h"
#include <stdexcept>
#include <iterator>
#include <algorithm>


#include <iostream>

// Вставьте сюда решение из предыдущего спринта
//using TransportGraph = graph::DirectedWeightedGraph<double>;

router::TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue)
	: catalogue_{ catalogue } {}

void router::TransportRouter::SetRouteSettings(domain::RouteSettings settings){
	settings_ = std::move(settings);
}

bool router::TransportRouter::ComputeItems(const std::string& from, const std::string& to){
	items_.clear();
	//надо запросить оба контейнера расстояний из каталога

	//получаю контейнер остановок
	const std::deque<domain::Stop>& stops = catalogue_.GetAllStops();

	// если размер контейнера в предыдущий раз был другой, то обновить всё
	if (previous_stops_count_ != stops.size()) {
		previous_stops_count_ = stops.size();
		
		//заполняю карты остановка-id и id-остановка
		for (size_t i = 0; i < stops.size(); ++i) {
			stops_id_.insert({ &stops[i], i });
			id_stops_.insert({ i, &stops[i] });
		}

		//Обновляю граф
		graph_ = std::make_unique<TransportGraph>(stops_id_.size());

		//получаю список всех маршрутов
		std::vector<const domain::Bus*> all_buses = catalogue_.GetAllBuses();
		//прохожу по каждому 
		for (const auto& bus : all_buses) {

			for (size_t first_stop = 0; first_stop < bus->stops.size(); ++first_stop) {
				//сразу записываем ожидание автобуса на начальной остановке
				double route_duration = settings_.bus_wait_time;

				//тут сначала от остановки А(i) считаем последовательно расстояния до всех пар (j, j+1)
				// где j не меньше i. То есть от остановки А считаем расстояние до А, затем до Б и так далее
				// Когда посчитаем все расстояния от остановки А до конечной будем считать от следующей (Б) остановки
				// до конечной. Как посчитали расстояние сразу создаем ребро {i,j, расстояние i->j}
				// 

				for (size_t last_stop = first_stop; last_stop < bus->stops.size()-1; ++last_stop) {
					//тут bus->stops.size()-1 потому что я буду искать расстояние от last_stop до last_stop+1

					//получаю расстояние j->j+1, рассчитываю время и записываю из в i->j+1 время поездки
					int route_distance = catalogue_.GetRouteDistance(bus->stops[last_stop], bus->stops[last_stop + 1]);
					double ride_time = (route_distance * minutes_per_hour) / (settings_.bus_velocity * kilo);
					route_duration += ride_time;

					//затем для i->j+1 остановок делаю грань графа

					graph::Edge<double> edge{ stops_id_[bus->stops[first_stop]]
						, stops_id_[bus->stops[last_stop + 1]]
						, route_duration, bus->route, last_stop - first_stop + 1 };

					//и записываю ее в граф
					graph_->AddEdge(edge);
				}
			}
		}

		//обновляю роутер
		router_ = std::make_unique<DistanceRouter>(*graph_);

	}
	//получаю указатели на остановки 
	const domain::Stop* from_stop = catalogue_.FindStop(from);
	const domain::Stop* to_stop = catalogue_.FindStop(to);

	//если остановки нет, то маршрут не построить
	if (!from_stop || !to_stop) {
		return false;
	}

	//по указателям получаю id остановок
	graph::VertexId from_id = stops_id_.at(from_stop);
	graph::VertexId to_id = stops_id_.at(to_stop);
	//и ищу кратчайший маршрут между ними
	auto route_result = router_->BuildRoute(from_id, to_id);
	
	//если значение не получено
	if (!route_result.has_value()) {
		//сообщаю о неудаче
		return false;
	}
	//получаю RouteInfo, оно хранит индекс граней и общее время движения.
	graph::Router<double>::RouteInfo route_info = route_result.value();

	//тут берем ребро, по первой вершине пишем wait item, 
	//затем по обеим вершинам находим автобус, сохраняем имя для item,
	//рассчитываем сколько между ними остановок для рассчета span_count
	// и собсно делаем итем bus 

	for (size_t i = 0; i < route_info.edges.size(); ++i) {
		//получаю грань
		const graph::Edge<double> edge = graph_->GetEdge(route_info.edges[i]);

		//по грани получаю названия остановок и общее время пути
		std::string first_stop_name = id_stops_[edge.from]->name;
		std::string last_stop_name = id_stops_[edge.to]->name;
		double bus_travel_time = edge.weight - settings_.bus_wait_time;
		std::string bus_name = edge.bus_route;
		size_t span_count = edge.span_count;

		//создаю итем ожидания и посадки на автобус рям в векторе
		items_.emplace_back(domain::Item{ domain::ItemType::Wait
			, first_stop_name
			, static_cast<double>(settings_.bus_wait_time )});

		//Оформляю bus item


		items_.emplace_back(domain::Item{ domain::ItemType::Bus
			, bus_name
			, static_cast<double>(bus_travel_time)
			, span_count });
		
	}

	return true;
}

std::vector<domain::Item> router::TransportRouter::GetItems() const noexcept{
	return std::move(items_);
}
