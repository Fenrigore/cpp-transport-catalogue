#include "request_handler.h"
#include <sstream>

handler::RequestHandler::RequestHandler(catalogue::TransportCatalogue& catalogue
	, render::Renderer& renderer
	, router::TransportRouter& router)
	: catalogue_{ catalogue }, renderer_{ renderer }, transport_router_{ router } {
}

void handler::RequestHandler::NewStop(std::string_view name, geo::Coordinates coordinates){
	catalogue_.AddStop(name, coordinates);
}

void handler::RequestHandler::NewBus(std::string_view route, const std::vector<std::string_view>& stops,bool is_circle){
	catalogue_.AddBus(route, stops, is_circle);
}

void handler::RequestHandler::AddStopsDistance(std::string_view from, std::string_view to, int distance){
	catalogue_.AddDistance(from, to, distance);
}

const domain::Stop* handler::RequestHandler::GetStop(std::string_view name) const{
	return catalogue_.FindStop(name);
}

domain::BusInfo handler::RequestHandler::GetBus(std::string_view route) const{
	return catalogue_.GetBusInfo(route);
}

const std::set<std::string_view>* handler::RequestHandler::FindBusesByStops(std::string_view name) const{
	return catalogue_.FindRoutes(name);
}

void handler::RequestHandler::SetRenderSettings(render::RenderSettings settings){
	renderer_.SetSettings(std::move(settings));
}

void handler::RequestHandler::SetRouteSettings(domain::RouteSettings settings) {
	transport_router_.SetRouteSettings(std::move(settings));
}

std::string handler::RequestHandler::GetMapStr() const {
	renderer_.FillContainers(catalogue_.GetAllBuses());
	return renderer_.GetMap();
}

std::optional<std::vector<domain::Item>> handler::RequestHandler::ComputeItems(const std::string& from, const std::string& to) {
	return transport_router_.ComputeItems(from, to);
}


