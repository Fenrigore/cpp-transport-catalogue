#pragma once

#include "json.h"
#include <vector>
#include <string>

namespace json {

    class Builder;
    class ContextAfterEnd;
    class KeyContext;
    class DictItemContext;
    class ArrayItemContext;

    class BaseContext {
    public:
        BaseContext(Builder& builder);
    protected:
        Builder& builder_;
    };

    //Обрабатывает завершение контейнера и Build
    class ContextAfterEnd : BaseContext {
    public:
        using BaseContext::BaseContext;
    public:
        // Методы для Array (продолжение добавления элементов)
        ContextAfterEnd Value(Node node_value = nullptr);
        ContextAfterEnd StartDict();
        ContextAfterEnd StartArray();
        ContextAfterEnd EndArray(); // Закрытие вложенного массива

        // Методы для Dict (продолжение добавления ключей)
        KeyContext Key(std::string key);
        ContextAfterEnd EndDict(); // Закрытие вложенного словаря

        // Метод для завершения
        Node Build();
    };


    //После Key, ожидание значения
    class KeyContext : BaseContext {
        using BaseContext::BaseContext;
    public:
        DictItemContext Value(Node node_value = nullptr);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    //Внутри массива
    class ArrayItemContext :  BaseContext {
        using BaseContext::BaseContext;
    public:
        ArrayItemContext Value(Node node_value = nullptr);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        ContextAfterEnd EndArray();
    };

    //Внутри словаря
    class DictItemContext :  BaseContext {
        using BaseContext::BaseContext;
    public:
        KeyContext Key(std::string key);
        ContextAfterEnd EndDict();
    };


    //Начальное состояние и хранилище логики
    class Builder {

    public:
        Builder() = default;

        // Разрешенные начальные методы
        ContextAfterEnd Value(Node node_value = nullptr);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

        // Запрещенные начальные методы
        Node Build() = delete;
        KeyContext Key(std::string key) = delete;
        ContextAfterEnd EndDict() = delete;
        ContextAfterEnd EndArray() = delete;

    private:
        Node root_{ nullptr };
        std::vector<Node*> nodes_stack_{};
        bool key_is_waiting_for_value_{ false };
        std::string last_key_{};
        bool value_was_setted_{};

        // Приватные Impl методы (логика)
        Builder& ValueImpl(Node node_value = nullptr);
        Builder& KeyImpl(std::string key);
        Builder& StartDictImpl();
        Builder& StartArrayImpl();
        Builder& EndDictImpl();
        Builder& EndArrayImpl();
        Node BuildImpl();

        // Дружественные классы для доступа к приватным Impl методам
        friend class BaseContext;
        friend class ContextAfterEnd;
        friend class DictItemContext;
        friend class KeyContext;
        friend class ArrayItemContext;

    };

}  // namespace json