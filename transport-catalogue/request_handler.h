#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
namespace handler {
    class RequestHandler {
    public:
        explicit RequestHandler(catalogue::TransportCatalogue& catalogue, render::Renderer& renderer);
        //для каталога
        void NewStop(std::string_view name, geo::Coordinates coordinates);
        void NewBus(std::string_view route, const std::vector<std::string_view>& stops, bool is_circle);
        void AddStopsDistance(std::string_view from, std::string_view to, int distance);
        const domain::Stop* GetStop(std::string_view name)const;
        domain::BusInfo GetBus(std::string_view route) const;
        const std::set<std::string_view>* FindBusesByStops(std::string_view name) const;

        void SetRenderSettings(render::RenderSettings settings);
        std::string GetMapStr() const;
    private:
        catalogue::TransportCatalogue& catalogue_;
        render::Renderer& renderer_;
    };
}
