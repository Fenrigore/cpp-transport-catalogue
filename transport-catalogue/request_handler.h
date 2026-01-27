#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
namespace handler {
    class RequestHandler {
    public:
        explicit RequestHandler(catalogue::TransportCatalogue& catalogue
            , render::Renderer& renderer
        , router::TransportRouter& router);
        //для каталога
        void NewStop(std::string_view name, geo::Coordinates coordinates);
        void NewBus(std::string_view route, const std::vector<std::string_view>& stops, bool is_circle);
        void AddStopsDistance(std::string_view from, std::string_view to, int distance);
        const domain::Stop* GetStop(std::string_view name)const;
        domain::BusInfo GetBus(std::string_view route) const;
        const std::set<std::string_view>* FindBusesByStops(std::string_view name) const;
        //Настройки
        void SetRenderSettings(render::RenderSettings settings);
        void SetRouteSettings(domain::RouteSettings settings);
        //получение карты
        std::string GetMapStr() const;
        //получение кратчайшего маршрута
        bool NeedItems(const std::string& from, const std::string& to);
        std::vector<domain::Item> GetItems() const;

    private:
        catalogue::TransportCatalogue& catalogue_;
        render::Renderer& renderer_;
        router::TransportRouter& transport_router_;
    };
}
