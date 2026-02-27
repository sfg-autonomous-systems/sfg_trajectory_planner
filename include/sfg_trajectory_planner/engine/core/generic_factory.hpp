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
        using CreatorFunction = std::function<std::unique_ptr<BaseType>(Args... args)>;

        struct RegisteredTypeInfo
        {
            std::string m_type;
            std::string m_display_name;
        };

        std::vector<RegisteredTypeInfo> get_registered_types() const;

        template <typename KeyType, typename DerivedType>
        void register_type(const std::string &display_name = "");
        template <typename KeyType, typename DerivedType>
        void register_type(CreatorFunction creator);
        template <typename KeyType, typename DerivedType>
        void register_type(const std::string &display_name, CreatorFunction creator);

        template <typename KeyType, typename DerivedType>
        std::unique_ptr<DerivedType> create_type(Args... args) const;
        std::unique_ptr<BaseType> create_type(const std::string &type, Args... args) const;

    private:
        struct RegisteredType
        {
        public:
            RegisteredTypeInfo m_info;
            CreatorFunction m_creator;
        };

        std::unordered_map<std::string, RegisteredType> m_registered_types;
    };
}

#include "sfg_trajectory_planner/engine/core/generic_factory.tpp"