#pragma once

#include "geo.h"
#include "domain.h"
#include <vector>
#include "svg.h"
#include <iostream>
#include <fstream>
#include <string_view>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <cstddef>
#include <deque>
#include <set>


namespace render {
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct Offset {
        double dx{};
        double dy{};
    };

    class RenderSettings {
    public:
        //установка
        RenderSettings& SetWidth(double width);
        RenderSettings& SetHeight(double height);
        RenderSettings& SetPadding(double padding);
        RenderSettings& SetLineWidth(double line_width);
        RenderSettings& SetStopRadius(double stop_radius);
        RenderSettings& SetBusLabelFontSize(double size);
        RenderSettings& SetBusLabelOffset(double dx, double dy);
        RenderSettings& SetStopLabelFontSize(double size);
        RenderSettings& SetStopLabelOffset(double dx, double dy);
        RenderSettings& SetUnderlayerColor(svg::Color color);
        RenderSettings& SetUnderlayerWidth(double width);
        void AddColorToPalette(svg::Color color);

        //взять
        double GetWidth() const noexcept;
        double GetHeight() const noexcept;
        double GetPadding() const noexcept;
        double GetLineWidth() const noexcept;
        double GetStopRadius() const noexcept;
        double GetBusLabelFontSize() const noexcept;
        Offset GetBusLabelOffset() const noexcept;
        double GetStopLabelFontSize() const noexcept;
        Offset GetStopLabelOffset() const noexcept;
        svg::Color GetUnderlayerColor() const noexcept;
        double GetUnderlayerWidth() const noexcept;
        svg::Color GetNextColorPalette();
        void ResetColorPaletteIndex() noexcept;

    private:
        double width_{};
        double height_{};
        double padding_{};
        double line_width_{};
        double stop_radius_{};
        double bus_label_font_size_{};
        Offset bus_label_offset_{};
        double stop_label_font_size_{};
        Offset stop_label_offset_{};
        svg::Color underlayer_color_{};
        double underlayer_width_{};
        std::vector<svg::Color> color_palette_{};
        size_t color_palette_index_{};

        static constexpr double max_clamp_ = 100000.0;
        static constexpr double min_clamp_ = -100000.0;
    };

    struct BusSetComparator {
        bool operator() (const domain::Bus* a, const domain::Bus* b) const {
            return a->route < b->route;
        }
    };

    struct StopsSetComparator {
        bool operator() (const domain::Stop* a, const domain::Stop* b) const {
            return a->name < b->name;
        }
    };

	class Renderer {
	public:
		void AddBus(const domain::Bus* bus);
	    
        void SetSettings(RenderSettings settings);

        void PrintMap(std::ostream& out);
        std::string GetMap();

	private:

        void PrintLines(const SphereProjector& proj);
        void PrintBusNames(const SphereProjector& proj);
        void PrintStopPoints(const SphereProjector& proj);
        void PrintStopNames(const SphereProjector& proj);
        void FillContainers();
        svg::Text CreateBusText(geo::Coordinates coords,
            const std::string& text, 
            std::string text_or_underlayer, 
            const SphereProjector& proj,
            svg::Color color = "black");

        svg::Text CreateStopText(geo::Coordinates coords,
            const std::string& text,
            std::string text_or_underlayer,
            const SphereProjector& proj,
            svg::Color color = "black");

        std::vector<std::vector<const geo::Coordinates*>> routes_coordinates_{};
        std::deque<geo::Coordinates> all_coordinates_{};
		svg::Document doc_{};
        std::set<const domain::Bus*, BusSetComparator> buses_{};
        std::set<const domain::Stop*, StopsSetComparator> stops_{};
        RenderSettings render_settings_{};
	};
}