#pragma once
#include "domain.h"
#include <vector>
#include <memory> 
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

using TransportGraph = graph::DirectedWeightedGraph<double>;
using DistanceRouter = graph::Router<double>;
namespace router {
	class TransportRouter {
	public:
		TransportRouter(const catalogue::TransportCatalogue& catalogue);

		inline static const double kilo = 1000.0;
		inline static const double minutes_per_hour = 60.0;

		//Я не могу перенести получение настроек в конструктор. 
		//Настройки принимаются по json запросу намного позже создания экземпляра класса
		void SetRouteSettings(domain::RouteSettings settings);
		std::optional<std::vector<domain::Item>> ComputeItems(std::string_view from, std::string_view to);
	private:
		std::vector<domain::Item> items_{};
		domain::RouteSettings settings_{};
		const catalogue::TransportCatalogue& catalogue_;
		std::unique_ptr<TransportGraph> graph_{};
		std::unique_ptr<DistanceRouter> router_{};
		size_t previous_stops_count_{};
		std::unordered_map<const domain::Stop*, graph::VertexId> stops_id_{};
		std::unordered_map<graph::VertexId, const domain::Stop*> id_stops_{};
	};
} //namespace router
