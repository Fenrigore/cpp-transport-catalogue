#pragma once

#include <iostream>
#include "request_handler.h"
#include "json.h"

struct DistanceInfo {
	std::string from;
	std::string to;
	int distance;
};

class JsonReader {
public:
	JsonReader(handler::RequestHandler& handler, std::istream& input_str);
	void ProcessRequests(std::ostream& out_str);
		
private:
	//для обработки запросов
	void ParseBaseRequests(const json::Array& requests);
	void ParseStop(const json::Dict& stop_dict);
	void ParseBus(const json::Dict& bus_dict);
	void ParseRender(const json::Dict& render_dict);
	svg::Color GiveMeColorDude(json::Node node);


	json::Array ProcessStatRequests(const json::Array& requests);
	json::Node ProcessBusRequest(const json::Dict& request);
	json::Node ProcessStopRequest(const json::Dict& request);
	json::Node ProcessMapRequest(const json::Dict& request);


	handler::RequestHandler& handler_;
	json::Document input_doc_;
	std::vector<DistanceInfo> distances_{};
};