#include "map_renderer.h"
#include <string>
#include <sstream>

void render::Renderer::AddBus(const domain::Bus* bus){
	buses_.insert(bus);
}

void render::Renderer::SetSettings(RenderSettings settings){
    render_settings_ = std::move(settings);
}

void render::Renderer::PrintMap(std::ostream& out){
    FillContainers();
    const double WIDTH = render_settings_.GetWidth();
    const double HEIGHT = render_settings_.GetHeight();
    const double PADDING = render_settings_.GetPadding();
    const render::SphereProjector projector{ all_coordinates_.begin(), all_coordinates_.end(), WIDTH, HEIGHT, PADDING };
    PrintLines(projector);
    PrintBusNames(projector);
    PrintStopPoints(projector);
    PrintStopNames(projector);
    doc_.Render(out);
}

std::string render::Renderer::GetMap()
{
    FillContainers();
    const double WIDTH = render_settings_.GetWidth();
    const double HEIGHT = render_settings_.GetHeight();
    const double PADDING = render_settings_.GetPadding();
    const render::SphereProjector projector{ all_coordinates_.begin(), all_coordinates_.end(), WIDTH, HEIGHT, PADDING };
    PrintLines(projector);
    PrintBusNames(projector);
    PrintStopPoints(projector);
    PrintStopNames(projector);
    std::ostringstream svg_stream;
    doc_.Render(svg_stream);
    return svg_stream.str();
}

void render::Renderer::PrintLines(const SphereProjector& proj){
    //перевести их в экранные координаты

    for (const auto& rout : routes_coordinates_) {
        svg::Polyline rout_lines{};
        for (const auto& coord : rout) {
            rout_lines.AddPoint(proj(*coord));
        }
        rout_lines.SetFillColor("none").
            SetStrokeWidth(10.).
            SetStrokeColor(render_settings_.GetNextColorPalette()).
            SetFillColor("none").
            SetStrokeWidth(render_settings_.GetLineWidth()).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        doc_.Add(std::move(rout_lines));
    }
}

void render::Renderer::PrintBusNames(const SphereProjector& proj){
    render_settings_.ResetColorPaletteIndex();
    for (const auto& bus : buses_) {
        if (bus->stops.size() == 0) {
            continue;
        }
        size_t stop_index{};
        const std::string& name_of_bus_route = bus->route;
        geo::Coordinates stop_coord = bus->stops[stop_index]->coordinates;
        //тут сделать методы печати подложки и текста
        svg::Color cur_color = render_settings_.GetNextColorPalette();
        doc_.Add(CreateBusText(stop_coord, name_of_bus_route, "underlayer", proj, cur_color));
        doc_.Add(CreateBusText(stop_coord, name_of_bus_route, "text", proj, cur_color));
        if (!bus->is_circle) {
            stop_index = bus->stops.size() / 2;
            if (bus->stops[0]->name == bus->stops[stop_index]->name) {
                continue;
            }
            doc_.Add(CreateBusText(bus->stops[stop_index]->coordinates, name_of_bus_route, "underlayer", proj, cur_color));
            doc_.Add(CreateBusText(bus->stops[stop_index]->coordinates, name_of_bus_route, "text", proj, cur_color));
        }
    }
}

void render::Renderer::PrintStopPoints(const SphereProjector& proj){
    for (const auto& stop : stops_) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(proj(stop->coordinates)).
            SetRadius(render_settings_.GetStopRadius()).
            SetFillColor("white");
        doc_.Add(stop_circle);
    }
}

void render::Renderer::PrintStopNames(const SphereProjector& proj){
    for (const auto& stop : stops_) {
        doc_.Add(CreateStopText(stop->coordinates, stop->name, "underlayer", proj));
        doc_.Add(CreateStopText(stop->coordinates, stop->name, "text", proj));
    }
}

void render::Renderer::FillContainers(){
    //нужно пройти по автобусам и взять инфу об остановкам
    for (const domain::Bus* bus : buses_) {
        //вектор для заполнения указателями на кооринаты текущего пути
        std::vector<const geo::Coordinates*> local_coords{};
        //достаю список остановок
        const auto stops = bus->stops;
        if (stops.size() > 0) {
            for (const auto& stop : stops) {
                stops_.insert(stop);
                //прохожу по ним и сохраняю остановки в дэку (не реалокаций)
                all_coordinates_.push_back(stop->coordinates);
                //передаю указатель на только что добавленные координаты в список 
                //координат текущего пути
                local_coords.push_back(&(all_coordinates_.back()));
            }
            //добавляю в список 
            routes_coordinates_.push_back(std::move(local_coords));
        }
    }
}

svg::Text render::Renderer::CreateBusText(geo::Coordinates coords,
                                       const std::string& text,
                                       std::string text_or_underlayer,
                                       const SphereProjector& proj,
                                       svg::Color color){
    svg::Text output_text{};
    //задаю одинаковые натройки текста
    if (text_or_underlayer == "underlayer") {
        output_text.SetFillColor(render_settings_.GetUnderlayerColor()).
                    SetStrokeColor(render_settings_.GetUnderlayerColor()).
                    SetStrokeWidth(render_settings_.GetUnderlayerWidth()).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
    else if (text_or_underlayer == "text") {
        output_text.SetFillColor(color);
    }
    output_text.SetPosition(proj(coords)).
        SetOffset({ render_settings_.GetBusLabelOffset().dx,
                    render_settings_.GetBusLabelOffset().dy }).
        SetFontSize(render_settings_.GetBusLabelFontSize()).
        SetData(text).
        SetFontFamily("Verdana").
        SetFontWeight("bold");
    return output_text;
}

svg::Text render::Renderer::CreateStopText(geo::Coordinates coords, 
    const std::string& text, 
    std::string text_or_underlayer, 
    const SphereProjector& proj, 
    svg::Color color){
    svg::Text output_text{};


    if (text_or_underlayer == "underlayer") {
        output_text.SetFillColor(render_settings_.GetUnderlayerColor()).
                    SetStrokeColor(render_settings_.GetUnderlayerColor()).
                    SetStrokeWidth(render_settings_.GetUnderlayerWidth()).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
    else if (text_or_underlayer == "text") {
        output_text.SetFillColor(color);
    }
    output_text.SetPosition(proj(coords)).
        SetOffset({ render_settings_.GetStopLabelOffset().dx,
                    render_settings_.GetStopLabelOffset().dy }).
        SetFontSize(render_settings_.GetStopLabelFontSize()).
        SetFontFamily("Verdana").
        SetData(text);
    return output_text;
}

bool render::IsZero(double value){
	return std::abs(value) < render::EPSILON;
}

render::RenderSettings& render::RenderSettings::SetWidth(double width) {
    width_ = std::clamp(width, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetHeight(double height) {
    height_ = std::clamp(height, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetPadding(double padding) {
    padding_ = std::clamp(padding, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetLineWidth(double line_width) {
    line_width_ = std::clamp(line_width, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetStopRadius(double stop_radius) {
    stop_radius_ = std::clamp(stop_radius, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetBusLabelFontSize(double size) {
    bus_label_font_size_ = std::clamp(size, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetBusLabelOffset(double dx, double dy) {
    bus_label_offset_.dx = std::clamp(dx, min_clamp_, max_clamp_);
    bus_label_offset_.dy = std::clamp(dy, min_clamp_, max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetStopLabelFontSize(double size) {
    stop_label_font_size_ = std::clamp(size, 0., max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetStopLabelOffset(double dx, double dy) {
    stop_label_offset_.dx = std::clamp(dx, min_clamp_, max_clamp_);
    stop_label_offset_.dy = std::clamp(dy, min_clamp_, max_clamp_);
    return *this;
}

render::RenderSettings& render::RenderSettings::SetUnderlayerColor(svg::Color color) {
    underlayer_color_ = color;
    return *this;
}

render::RenderSettings& render::RenderSettings::SetUnderlayerWidth(double width) {
    underlayer_width_ = std::clamp(width, 0., max_clamp_);
    return *this;
}

void render::RenderSettings::AddColorToPalette(svg::Color color) {
    color_palette_.push_back(std::move(color));
}

double render::RenderSettings::GetWidth() const noexcept {
    return width_;
}

double render::RenderSettings::GetHeight() const noexcept {
    return height_;
}

double render::RenderSettings::GetPadding() const noexcept {
    return padding_;
}

double render::RenderSettings::GetLineWidth() const noexcept {
    return line_width_;
}

double render::RenderSettings::GetStopRadius() const noexcept {
    return stop_radius_;
}

double render::RenderSettings::GetBusLabelFontSize() const noexcept {
    return bus_label_font_size_;
}

render::Offset render::RenderSettings::GetBusLabelOffset() const noexcept {
    return bus_label_offset_;
}

double render::RenderSettings::GetStopLabelFontSize() const noexcept {
    return stop_label_font_size_;
}

render::Offset render::RenderSettings::GetStopLabelOffset() const noexcept {
    return stop_label_offset_;
}

svg::Color render::RenderSettings::GetUnderlayerColor() const noexcept {
    return underlayer_color_;
}

double render::RenderSettings::GetUnderlayerWidth() const noexcept {
    return underlayer_width_;
}

svg::Color render::RenderSettings::GetNextColorPalette() {
    if (color_palette_.empty()) {
        return svg::Color{};
    }

    svg::Color color = color_palette_.at(color_palette_index_);
    ++color_palette_index_;
    color_palette_index_ = color_palette_index_ >= color_palette_.size() ? 0 : color_palette_index_;
    return color;
}

void render::RenderSettings::ResetColorPaletteIndex() noexcept {
    color_palette_index_ = 0;
}