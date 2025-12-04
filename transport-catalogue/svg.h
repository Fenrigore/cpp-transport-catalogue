#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b) : red{ r }, green{ g }, blue{ b } {}
        uint8_t red{};
        uint8_t green{};
        uint8_t blue{};
    };

    struct Rgba : public Rgb {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op) : Rgb(r, g, b), opacity{ op } {}
        double opacity = 1;
    };

    using Color = std::variant < std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none" };

    struct ColorStreamVisitor {
        std::ostream& out;
        void operator()(std::monostate) const {
            out << "none";
        }
        void operator()(const std::string& str) const {
            out << str;
        }
        void operator() (const Rgb& rgb) const {
            //"rgb("s + rgb.red + ","s + g + ","s + b + ")"s
            out << "rgb(" <<
                static_cast<int>(rgb.red) << "," <<
                static_cast<int>(rgb.green) << "," <<
                static_cast<int>(rgb.blue) << ")";
        }
        void operator()(const Rgba& rgba) const {
            out << "rgba(" <<
                static_cast<int>(rgba.red) << "," <<
                static_cast<int>(rgba.green) << "," <<
                static_cast<int>(rgba.blue) << "," <<
                rgba.opacity << ")";
        }
    };

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap cap);

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin join);

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {}
        double x = 0;
        double y = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        //Установить цвет заливки ("none" - без заливки)
        Owner& SetFillColor(Color fill_color) {
            fill_color_ = fill_color;
            return AsOwner();
        }
        //Установить цвет контура ("none" - без контура)
        Owner& SetStrokeColor(Color stroke_color) {
            stroke_color_ = stroke_color;
            return AsOwner();
        }
        //Установить ширину контура (double)
        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }
        //установить вид обводки https://developer.mozilla.org/ru/docs/Web/SVG/Tutorials/SVG_from_scratch/Fills_and_strokes/svg_stroke_linecap_example.png
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }
        //соединение обводки https://developer.mozilla.org/ru/docs/Web/SVG/Tutorials/SVG_from_scratch/Fills_and_strokes/svg_stroke_linejoin_example.png
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }
    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
            if (fill_color_ || stroke_color_ || width_ || line_cap_ || line_join_) {
                out << " "sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
        
        std::optional<Color> fill_color_{};
        std::optional<Color> stroke_color_{};
        std::optional<double> width_{};
        std::optional<StrokeLineCap> line_cap_{};
        std::optional<StrokeLineJoin> line_join_{};
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_{};

    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        std::string EscapingSpecialCharacters(std::string text) const;

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_{};
        Point offset_{};
        uint32_t font_size_ = 1;
        std::string font_family_{};
        std::string font_weight_{};
        std::string text_{};
    };

    class ObjectContainer {
    public:
        //Тут написал шаблонный метод для добавления объекта, который 
        //использует виртуальный интерфейсный сетод AddPtr
        template <typename T>
        void Add(T object) {
            AddPtr(std::make_unique<T>(std::move(object)));
        }

    private:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        //полей никаких нет, ибо это типа интерфейс
    };

    class Document : public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:

        std::vector<std::unique_ptr<Object>> objects_;
    };


    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

}  // namespace svg