#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorStreamVisitor{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap cap) {
        switch (cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin join) {
        switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }
        return out;

    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        //push_back без перемещения потому что point супер-простая структура
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        //<polyline points="0,100 50,25 50,75 100,0" />
        out << "<polyline points=\""sv;

        for (size_t i = 0; i < points_.size(); ++i) {
            out << (i > 0 ? " "sv : ""sv) << points_[i].x << ","sv << points_[i].y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_ = data;
        return *this;
    }

    std::string Text::EscapingSpecialCharacters(std::string text)const {
        for (size_t pos = text.length(); pos-- > 0; ) {
            switch (text[pos]) {
            case '"':
                text.replace(pos, 1, "&quot;");
                break;
            case '\'':
                text.replace(pos, 1, "&apos;");
                break;
            case '<':
                text.replace(pos, 1, "&lt;");
                break;
            case '>':
                text.replace(pos, 1, "&gt;");
                break;
            case '&':
                text.replace(pos, 1, "&amp;");
                break;
            }
        }
        return text;
    }


    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        //    <text x="20" y="35" dx="10" dy="5"
        //    font-family="Verdana"
        //    font-size="16"
        //    font-weight="bold"
        //    fill="blue">
        //    My
        //    </text>
        out << " <text"sv;
        RenderAttrs(context.out);
        out << "x=\""sv << position_.x << "\" "sv;
        out << "y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" "sv;
        out << "dy=\""sv << offset_.y << "\" "sv;

        out << "font-size=\""sv << font_size_ << "\" "sv;
        if (font_family_.size() != 0) {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        //Вот тут надо подумать. Или не надо, если всё заработает. ¯\_(ツ)_/¯
       // RenderAttrs(context.out);

        if (font_weight_.size() != 0) {
            out << "font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << EscapingSpecialCharacters(text_) << "</text>"sv;

    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        //первая строка	<?xml version="1.0" encoding="UTF-8" ?>
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        //вторая строка	<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        //все объекты
        RenderContext context{ out };
        for (const auto& obj : objects_) {
            obj.get()->Render(context);
        }
        //затем конец
        out << "</svg>\n";
    }



}  // namespace svg