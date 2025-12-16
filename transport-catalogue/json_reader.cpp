#include "json_reader.h"

#include "json_builder.h"

JsonReader::JsonReader(handler::RequestHandler& handler, std::istream& input_str)
	:handler_{ handler }, input_doc_{ json::Load(input_str) } {
}

void JsonReader::ProcessRequests(std::ostream& out_str) {
	const json::Dict& root_dict = input_doc_.GetRoot().AsDict();

	ParseBaseRequests(root_dict.at("base_requests").AsArray());
	ParseRender(root_dict.at("render_settings").AsDict());
	json::Array to_out = ProcessStatRequests(root_dict.at("stat_requests").AsArray());
	if (to_out.size() > 0) {
		json::Print(json::Document(to_out), out_str);
	}
}

void JsonReader::ParseBaseRequests(const json::Array& requests) {
	for (const auto& request : requests) {
		const json::Dict& request_dict = request.AsDict();
		auto it = request_dict.find("type");
		if (it != request_dict.end()
			&& it->second.IsString()
			&& it->second.AsString() == "Stop") {
			ParseStop(request_dict);
		}
	}
	for (const auto& request : requests) {
		const json::Dict& request_dict = request.AsDict();
		auto it = request_dict.find("type");
		if (it != request_dict.end()
			&& it->second.IsString()
			&& it->second.AsString() == "Bus") {
			ParseBus(request_dict);
		}
	}
	for (const auto& distance : distances_) {
		handler_.AddStopsDistance(distance.from, distance.to, distance.distance);
	}
}

void JsonReader::ParseStop(const json::Dict& stop_dict) {
	const std::string& name = stop_dict.at("name").AsString();
	const double lat = stop_dict.at("latitude").AsDouble();
	const double lng = stop_dict.at("longitude").AsDouble();
	handler_.NewStop(name, { lat, lng });

	if (stop_dict.count("road_distances")) {
		const json::Dict& road_distances = stop_dict.at("road_distances").AsDict();
		for (const auto& [neighbor_name, distance_node] : road_distances) {
			int distance = distance_node.AsInt();
			distances_.push_back({ name, neighbor_name, distance });
		}
	}
}

void JsonReader::ParseBus(const json::Dict& bus_dict) {
	std::string name = bus_dict.at("name").AsString();
	bool is_round = bus_dict.at("is_roundtrip").AsBool();
	const json::Array& stops_array = bus_dict.at("stops").AsArray();

	std::vector<std::string_view> stops;
	for (const auto& stop_node : stops_array) {
		stops.push_back(stop_node.AsString());
	}

	if (!is_round) {
		for (size_t i = stops.size() - 1; i > 0; --i) {
			stops.push_back(stops[i - 1]);
		}
	}

	handler_.NewBus(name, stops, is_round);
}

void JsonReader::ParseRender(const json::Dict& render_dict) {
	//создаю пустой класс настроек
	render::RenderSettings render_settings{};
	//заполняю даблами
	render_settings.SetWidth(render_dict.at("width").AsDouble())
		.SetHeight(render_dict.at("height").AsDouble())
		.SetPadding(render_dict.at("padding").AsDouble())
		.SetLineWidth(render_dict.at("line_width").AsDouble())
		.SetStopRadius(render_dict.at("stop_radius").AsDouble())
		.SetBusLabelFontSize(render_dict.at("bus_label_font_size").AsDouble())
		.SetStopLabelFontSize(render_dict.at("stop_label_font_size").AsDouble())
		.SetUnderlayerWidth(render_dict.at("underlayer_width").AsDouble())
		.ResetColorPaletteIndex();

	//перехожу к заполнению более сложных полей
		//"bus_label_offset": [7.0, 15.0] хранит только 2 элемента
	double dx = render_dict.at("bus_label_offset").AsArray()[0].AsDouble();
	double dy = render_dict.at("bus_label_offset").AsArray()[1].AsDouble();

	render_settings.SetBusLabelOffset(dx, dy);

	// "stop_label_offset": [7.0, -3.0] так же хранит только 2 элемента
	dx = render_dict.at("stop_label_offset").AsArray()[0].AsDouble();
	dy = render_dict.at("stop_label_offset").AsArray()[1].AsDouble();

	render_settings.SetStopLabelOffset(dx, dy);

	//получил ноду, отправил её в метод. Он вернет цвет, если был в ноде
	render_settings.SetUnderlayerColor(GiveMeColorDude(render_dict.at("underlayer_color")));

	//и из палетты все цвета передаю в настройки
	for (const auto& color_node : render_dict.at("color_palette").AsArray()) {
		render_settings.AddColorToPalette(GiveMeColorDude(color_node));
	}

	handler_.SetRenderSettings(std::move(render_settings));
}


svg::Color JsonReader::GiveMeColorDude(json::Node node)
{
	if (node.IsArray()) {
		size_t size = node.AsArray().size();
		switch (size) {
		case 3:
			return svg::Rgb(node.AsArray()[0].AsInt(),
				node.AsArray()[1].AsInt(),
				node.AsArray()[2].AsInt());
			break;
		case 4:
			return svg::Rgba(node.AsArray()[0].AsInt(),
				node.AsArray()[1].AsInt(),
				node.AsArray()[2].AsInt(),
				node.AsArray()[3].AsDouble());
			break;
		default:
			break;
		}
	}
	else if (node.IsString()) {
		return node.AsString();
	}
	return svg::Color{};
}

json::Array JsonReader::ProcessStatRequests(const json::Array& requests)
{
	json::Builder builder{};
	auto builder_array = builder.StartArray();

	for (const auto& request_node : requests) {
		const json::Dict& request_dict = request_node.AsDict();
		std::string type{};
		auto type_it = request_dict.find("type");
		if (type_it != request_dict.end()) {
			type = type_it->second.AsString();
			json::Node response;
			if (type == "Bus") {
				response = ProcessBusRequest(request_dict);
			}
			else if (type == "Stop") {
				response = ProcessStopRequest(request_dict);
			}
			else if (type == "Map") {
				response = ProcessMapRequest(request_dict);
			}
			builder_array.Value(std::move(response));
		}
		else {
			std::cerr << "unknown type" << std::endl;
		}
	}
	return builder_array.EndArray().Build().AsArray();
}

json::Node JsonReader::ProcessBusRequest(const json::Dict& request)
{
	int id = request.at("id").AsInt();
	std::string name = request.at("name").AsString();

	domain::BusInfo bus_info = handler_.GetBus(name);

	// Если маршрут не найден
	if (bus_info.count == 0) {
		return json::Builder{}
			.StartDict()
			.Key("request_id")
			.Value(id)
			.Key("error_message")
			.Value("not found")
			.EndDict()
			.Build();
	}

	// Маршрут найден - формируем полный ответ
	return json::Builder{}
		.StartDict()
		.Key("request_id").Value(id)
		.Key("curvature").Value(bus_info.curvature)
		.Key("route_length").Value(bus_info.route_distance)
		.Key("stop_count").Value(static_cast<int>(bus_info.count))
		.Key("unique_stop_count").Value(static_cast<int>(bus_info.unique))
		.EndDict().Build();
}

json::Node JsonReader::ProcessStopRequest(const json::Dict& request)
{
	int id = request.at("id").AsInt();
	std::string name = request.at("name").AsString();

	// Проверяем, существует ли остановка
	if (!handler_.GetStop(name)) {  //метод FindStop в RequestHandler
		return json::Builder{}
			.StartDict()
			.Key("request_id").Value(id)
			.Key("error_message").Value("not found")
			.EndDict().Build();
	}

	// Получаем маршруты (может быть nullptr)
	const std::set<std::string_view>* buses = handler_.FindBusesByStops(name);

	json::Builder builder;
	builder.StartDict()
		.Key("request_id").Value(id)
		.Key("buses");

	auto builder_array = builder.StartArray();
	if (buses) {
		for (const auto& bus_name : *buses) {
			//buses_array.push_back(std::string(bus_name));
			builder_array.Value(std::string(bus_name));
		}
	}
	return builder_array.EndArray().EndDict().Build();
}

json::Node JsonReader::ProcessMapRequest(const json::Dict& request) {
	int id = request.at("id").AsInt();

	// Получаем SVG карту через RequestHandler
	std::string svg_map = handler_.GetMapStr();

	return json::Builder{}.StartDict()
		.Key("request_id").Value(id)
		.Key("map").Value(std::move(svg_map))
		.EndDict().Build();
}
