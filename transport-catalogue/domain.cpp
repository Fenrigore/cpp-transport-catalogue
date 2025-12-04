#include "domain.h"
namespace domain {
	Stop::Stop(std::string stop_name, geo::Coordinates stop_coordinates)
		:name{ stop_name }, coordinates{ stop_coordinates } {
	}

	Bus::Bus(std::string bus_route, std::vector<domain::Stop*> bus_stops, bool circle)
		:route{ bus_route }, stops{ bus_stops }, is_circle{ circle } {
	}

	BusInfo::BusInfo(size_t stops_count, size_t unique_stops, double geo_distance_in, int route_distance_in)
		:count{ stops_count },
		unique{ unique_stops },
		geo_distance{ geo_distance_in },
		route_distance{ route_distance_in } {
		if (geo_distance_in > 0 && route_distance_in > 0) {
			curvature = route_distance_in / geo_distance_in;
		}
	}
}
