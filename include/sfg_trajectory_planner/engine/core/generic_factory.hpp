#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sfg_trajectory_planner::engine::core
{
    template <typename BaseType, typename... Args>
    class GenericFactory
    {
    public:
        struct RegisteredTypeInfo
        {
            std::string m_type;
            std::string m_display_name;
        };

        std::vector<RegisteredTypeInfo> get_registered_types() const;

        template <typename KeyType, typename DerivedType>
        void register_type(const std::string &display_name = "");

        template <typename KeyType, typename DerivedType>
        void register_type(std::function<std::unique_ptr<BaseType>(Args... args)> creator, const std::string &display_name = "");

        template <typename KeyType, typename DerivedType>
        std::unique_ptr<DerivedType> create_object(Args... args) const;
        std::unique_ptr<BaseType> create_object(const std::string &type, Args... args) const;

    private:
        struct RegisteredType
        {
        public:
            RegisteredTypeInfo m_info;
            std::function<std::unique_ptr<BaseType>(Args... args)> m_creator;
        };

        std::unordered_map<std::string, RegisteredType> m_registered_types;
    };
}

#include "sfg_trajectory_planner/engine/core/generic_factory.tpp"