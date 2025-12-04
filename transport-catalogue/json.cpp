#include "json.h"
#include <stdexcept>
#include <cmath>
#include <limits>
#include <cctype>
#include <sstream>

using namespace std;

namespace json {
    namespace {
        Node LoadNode(istream& input);
        void SkipWhitespace(istream& input) {
            while (!input.eof() && isspace(input.peek())) {
                input.get();
            }
        }

        string LoadEscapeString(istream& input) {
            string result;
            while (true) {
                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in string");
                }
                char c = input.get();
                if (c == '"') {
                    break;
                }
                if (c == '\\') {
                    if (input.eof()) {
                        throw ParsingError("Unexpected end of input after escape character");
                    }
                    c = input.get();
                    switch (c) {
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    default:
                        throw ParsingError("Invalid escape sequence");
                    }
                }
                else {
                    result += c;
                }
            }
            return result;
        }

        Node LoadString(istream& input) {
            string result = LoadEscapeString(input);
            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            string number_str;
            bool is_double = false;

            if (input.peek() == '-') {
                number_str += input.get();
            }

            // Целая часть
            if (input.eof() || !isdigit(input.peek())) {
                throw ParsingError("Invalid number format");
            }
            while (!input.eof() && isdigit(input.peek())) {
                number_str += input.get();
            }

            // Дробная часть
            if (!input.eof() && input.peek() == '.') {
                is_double = true;
                number_str += input.get();
                if (input.eof() || !isdigit(input.peek())) {
                    throw ParsingError("Invalid number format after decimal point");
                }
                while (!input.eof() && isdigit(input.peek())) {
                    number_str += input.get();
                }
            }

            // Экспоненциальная часть
            if (!input.eof() && (input.peek() == 'e' || input.peek() == 'E')) {
                is_double = true;
                number_str += input.get();
                if (!input.eof() && (input.peek() == '+' || input.peek() == '-')) {
                    number_str += input.get();
                }
                if (input.eof() || !isdigit(input.peek())) {
                    throw ParsingError("Invalid number format in exponent");
                }
                while (!input.eof() && isdigit(input.peek())) {
                    number_str += input.get();
                }
            }

            try {
                if (is_double) {
                    size_t pos;
                    double value = stod(number_str, &pos);
                    if (pos != number_str.size()) {
                        throw ParsingError("Invalid number format");
                    }
                    return Node(value);
                }
                else {
                    size_t pos;
                    long long value = stoll(number_str, &pos);
                    if (pos != number_str.size()) {
                        throw ParsingError("Invalid number format");
                    }
                    // Проверяем, помещается ли в int
                    if (value > numeric_limits<int>::max() || value < numeric_limits<int>::min()) {
                        return Node(static_cast<double>(value));
                    }
                    return Node(static_cast<int>(value));
                }
            }
            catch (const std::out_of_range&) {
                throw ParsingError("Number out of range");
            }
            catch (const std::invalid_argument&) {
                throw ParsingError("Invalid number format");
            }
        }

        Node LoadArray(istream& input) {
            Array result;

            input.get(); // consume '['
            SkipWhitespace(input);

            if (input.eof()) {
                throw ParsingError("Unexpected end of input in array");
            }

            if (input.peek() == ']') {
                input.get();
                return Node(move(result));
            }

            while (true) {
                SkipWhitespace(input);
                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in array");
                }
                result.push_back(LoadNode(input));
                SkipWhitespace(input);

                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in array");
                }

                char c = input.get();
                if (c == ']') {
                    break;
                }
                else if (c != ',') {
                    throw ParsingError("Expected ',' or ']' in array");
                }
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;

            input.get(); // consume '{'
            SkipWhitespace(input);

            if (input.eof()) {
                throw ParsingError("Unexpected end of input in dictionary");
            }

            if (input.peek() == '}') {
                input.get();
                return Node(move(result));
            }

            while (true) {
                SkipWhitespace(input);

                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in dictionary");
                }

                // Ключ должен быть строкой
                if (input.peek() != '"') {
                    throw ParsingError("Expected '\"' for dictionary key");
                }
                input.get(); // consume '"'
                string key = LoadEscapeString(input);

                SkipWhitespace(input);

                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in dictionary");
                }

                // Двоеточие
                if (input.get() != ':') {
                    throw ParsingError("Expected ':' after dictionary key");
                }

                SkipWhitespace(input);
                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in dictionary");
                }
                result[move(key)] = LoadNode(input);
                SkipWhitespace(input);

                if (input.eof()) {
                    throw ParsingError("Unexpected end of input in dictionary");
                }

                char c = input.get();
                if (c == '}') {
                    break;
                }
                else if (c != ',') {
                    throw ParsingError("Expected ',' or '}' in dictionary");
                }
            }

            return Node(move(result));
        }

        Node LoadLiteral(istream& input) {
            string literal;
            while (!input.eof() && isalpha(input.peek())) {
                literal += input.get();
            }

            if (literal == "true") {
                return Node(true);
            }
            else if (literal == "false") {
                return Node(false);
            }
            else if (literal == "null") {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Unknown literal: " + literal);
            }
        }

        Node LoadNode(istream& input) {
            SkipWhitespace(input);

            if (input.eof()) {
                throw ParsingError("Unexpected end of input");
            }

            char c = input.peek();

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                input.get(); // consume '"'
                return LoadString(input);
            }
            else if (c == '-' || isdigit(c)) {
                return LoadNumber(input);
            }
            else if (isalpha(c)) {
                return LoadLiteral(input);
            }
            else {
                throw ParsingError("Unexpected character: " + string(1, c));
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return holds_alternative<int>(var_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(var_) || holds_alternative<int>(var_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(var_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(var_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(var_);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(var_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(var_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(var_);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(var_);
        }
        throw logic_error("Wrong type of variable");
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(var_);
        }
        throw logic_error("Wrong type of variable");
    }

    double Node::AsDouble() const {
        if (IsDouble()) {
            return IsPureDouble() ? get<double>(var_) : static_cast<double>(get<int>(var_));
        }
        throw logic_error("Wrong type of variable");
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return get<string>(var_);
        }
        throw logic_error("Wrong type of variable");
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(var_);
        }
        throw logic_error("Wrong type of variable");
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(var_);
        }
        throw logic_error("Wrong type of variable");
    }

    bool Node::operator==(const Node& other) const {
        if (var_.index() != other.var_.index()) {
            return false;
        }
        if (IsPureDouble() && other.IsPureDouble()) {
            return CompareDouble(AsDouble(), other.AsDouble());
        }
        return var_ == other.var_;
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }

    bool Node::CompareDouble(double a, double b) {
        return fabs(a - b) < numeric_limits<double>::epsilon() * 10;
    }

    Document::Document(Node root) : root_(move(root)) {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, ostream& output) {
        visit(NodeVisitor{ output }, doc.GetRoot().GetValue());
    }

}  // namespace json