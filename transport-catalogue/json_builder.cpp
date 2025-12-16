#include "json_builder.h"
#include <stdexcept>
#include <variant>
#include <utility>

namespace json {

    BaseContext::BaseContext(Builder& builder) :builder_{ builder } {}

    ContextAfterEnd Builder::Value(Node node_value) {
        ValueImpl(std::move(node_value));
        return ContextAfterEnd(*this);
    }

    DictItemContext Builder::StartDict() {
        StartDictImpl();
        return DictItemContext(*this);
    }

    ArrayItemContext Builder::StartArray() {
        StartArrayImpl();
        return ArrayItemContext(*this);
    }

    Builder& Builder::ValueImpl(Node node_value) {
        //Если корень пустой
        if (nodes_stack_.empty()) {
            //просто сразу добавляю значение
            if (value_was_setted_) {
                throw std::logic_error("It already has a value");
            }
            root_ = std::move(node_value);
        }
        //если последний элемент в векторе - массив, то добавляю в него в конец
        else if (nodes_stack_.back()->IsArray()) {
            json::Array& temp = std::get<Array>(nodes_stack_.back()->GetNCValue());
            temp.push_back(std::move(node_value));
        }
        else if (nodes_stack_.back()->IsDict()) {
            if (!key_is_waiting_for_value_) {
                throw std::logic_error("Where a key?");
            }
            json::Dict& temp = std::get<Dict>(nodes_stack_.back()->GetNCValue());
            temp[std::move(last_key_)] = std::move(node_value);
            last_key_.clear();
            key_is_waiting_for_value_ = false;
        }
        value_was_setted_ = true;
        return *this;
    }

    Builder& Builder::KeyImpl(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("You tried to enter a key for a non-dict");
        }
        if (key_is_waiting_for_value_) {
            throw std::logic_error("You tried to enter a key twice");
        }
        last_key_ = std::move(key);
        key_is_waiting_for_value_ = true;
        return *this;
    }

    Builder& Builder::StartDictImpl() {
        //Ели очередь пустая
        if (nodes_stack_.empty()) {
            //и не был записан рут
            if (value_was_setted_) {
                throw std::logic_error("It already has a value");
            }
            //просто сразу добавляю массив в него и записываю в вектор
            nodes_stack_.push_back(&(root_ = Dict{}));
        }
        else if (nodes_stack_.back()->IsDict()) {
            if (!key_is_waiting_for_value_) {
                throw std::logic_error("Where a key?");
            }
            json::Dict& temp = std::get<Dict>(nodes_stack_.back()->GetNCValue());
            nodes_stack_.push_back(&(temp[std::move(last_key_)] = Dict{}));
            last_key_.clear();
            key_is_waiting_for_value_ = false;
        }
        else if (nodes_stack_.back()->IsArray()) {
            json::Array& temp = std::get<Array>(nodes_stack_.back()->GetNCValue());
            temp.push_back(Dict{});
            nodes_stack_.push_back(&(temp.back()));
        }
        else {
            throw std::logic_error("Cant start dict");
        }
        value_was_setted_ = true;
        return *this;
    }

    Builder& Builder::StartArrayImpl() {
        //Ели рут пустой
        if (nodes_stack_.empty()) {
            if (value_was_setted_) {
                throw std::logic_error("It already has a value");
            }
            //просто сразу добавляю массив в него и записываю в вектор
            nodes_stack_.push_back(&(root_ = Array{}));
        }
        //Если последний элемент, на которыый указывает указатель
        //это массив, добавляю в него еще один элемент (пустой массив) 
        //и передаю указатель на него в вектор
        else if (nodes_stack_.back()->IsArray()) {
            json::Array& temp = std::get<Array>(nodes_stack_.back()->GetNCValue());
            temp.push_back(Array{});
            nodes_stack_.push_back(&(temp.back()));
        }
        else if (nodes_stack_.back()->IsDict()) {
            if (!key_is_waiting_for_value_) {
                throw std::logic_error("Where a key?");
            }
            json::Dict& temp = std::get<Dict>(nodes_stack_.back()->GetNCValue());
            nodes_stack_.push_back(&(temp[std::move(last_key_)] = Array{}));
            last_key_.clear();
            key_is_waiting_for_value_ = false;
        }
        else {
            throw std::logic_error("Cant start array");
        }
        value_was_setted_ = true;
        return *this;
    }

    Builder& Builder::EndDictImpl() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Nothing to close");
        }
        else if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("You tried to close a non-dict using the dict method");
        }
        else if (key_is_waiting_for_value_) {
            throw std::logic_error("The key was waiting for the value to be recorded");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArrayImpl() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Nothing to close");
        }
        else if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("You tried to close a non-array using the array method");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::BuildImpl() {
        size_t size_of_stack = nodes_stack_.size();

        if (size_of_stack > 0 || key_is_waiting_for_value_) {
            throw std::logic_error("Containers not closed");
        }
        else if (size_of_stack == 0 && !value_was_setted_) {
            throw std::logic_error("Root is empty");
        }
        return std::move(root_);
    }

    DictItemContext KeyContext::Value(Node node_value) {
        builder_.ValueImpl(std::move(node_value));
        return DictItemContext(builder_);
    }

    DictItemContext KeyContext::StartDict() {
        builder_.StartDictImpl();
        return DictItemContext(builder_);
    }

    ArrayItemContext KeyContext::StartArray() {
        builder_.StartArrayImpl();
        return ArrayItemContext(builder_);
    }

    KeyContext DictItemContext::Key(std::string key) {
        builder_.KeyImpl(std::move(key));
        return KeyContext(builder_);
    }

    ContextAfterEnd DictItemContext::EndDict() {
        builder_.EndDictImpl();
        return ContextAfterEnd(builder_);
    }

    ArrayItemContext ArrayItemContext::Value(Node node_value) {
        builder_.ValueImpl(std::move(node_value));
        return ArrayItemContext(builder_);
    }

    DictItemContext ArrayItemContext::StartDict() {
        builder_.StartDictImpl();
        return DictItemContext(builder_);
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        builder_.StartArrayImpl();
        return ArrayItemContext(builder_);
    }

    ContextAfterEnd ArrayItemContext::EndArray() {
        builder_.EndArrayImpl();
        return ContextAfterEnd(builder_);
    }

    ContextAfterEnd ContextAfterEnd::Value(Node node_value) {
        builder_.ValueImpl(std::move(node_value));
        return *this;
    }

    ContextAfterEnd ContextAfterEnd::StartDict() {
        builder_.StartDictImpl();
        return *this;
    }

    ContextAfterEnd ContextAfterEnd::StartArray() {
        builder_.StartArrayImpl();
        return *this;
    }

    ContextAfterEnd ContextAfterEnd::EndDict() {
        builder_.EndDictImpl();
        return *this;
    }

    ContextAfterEnd ContextAfterEnd::EndArray() {
        builder_.EndArrayImpl();
        return *this;
    }

    KeyContext ContextAfterEnd::Key(std::string key) {
        builder_.KeyImpl(std::move(key));
        return KeyContext(builder_);
    }

    Node ContextAfterEnd::Build() {
        return builder_.BuildImpl();
    }

}  // namespace json