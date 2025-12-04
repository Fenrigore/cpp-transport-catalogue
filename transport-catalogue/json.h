#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <iterator>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() = default;
        Node(int value) : var_(value) {}
        Node(double value) : var_(value) {}
        Node(const char* value) : var_(std::string(value)) {}
        Node(std::string value) : var_(std::move(value)) {}
        Node(bool value) : var_(value) {}
        Node(Array value) : var_(std::move(value)) {}
        Node(Dict value) : var_(std::move(value)) {}
        Node(std::nullptr_t) : var_(nullptr) {}

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const { return var_; }

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value var_{};
        static bool CompareDouble(double a, double b);
    };

    struct NodeVisitor {
        std::ostream& out;
        void operator()(std::nullptr_t) const {
            out << "null";
        }

        void operator()(int number) const {
            out << number;
        }

        void operator()(double number) const {
            out << number;
        }

        void operator()(const std::string& text) const {
            out << '"';
            for (char c : text) {
                switch (c) {
                case '\\': out << "\\\\"; break;
                case '\"': out << "\\\""; break;
                case '\n': out << "\\n"; break;
                case '\r': out << "\\r"; break;
                case '\t': out << "\\t"; break;
                default: out << c; break;
                }
            }
            out << '"';
        }

        void operator()(bool b) const {
            out << (b ? "true" : "false");
        }

        void operator()(const Array& array) const {
            out << "[";
            bool first = true;
            for (const auto& item : array) {
                if (!first) out << ", ";
                first = false;
                std::visit(NodeVisitor{ out }, item.GetValue());
            }
            out << "]";
        }

        void operator()(const Dict& map) const {
            out << "{";
            bool first = true;
            for (const auto& [key, value] : map) {
                if (!first) out << ", ";
                first = false;
                out << '"' << key << "\": ";
                std::visit(NodeVisitor{ out }, value.GetValue());
            }
            out << "}";
        }
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
    private:
        Node root_;
    public:
        bool operator == (const Document& other) const {
            return root_ == other.root_;
        }
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);

}  // namespace json