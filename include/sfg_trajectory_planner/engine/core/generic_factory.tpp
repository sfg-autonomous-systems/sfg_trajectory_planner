#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"

#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core
{
    template <typename BaseType, typename... Args>
    std::vector<typename GenericFactory<BaseType, Args...>::RegisteredTypeInfo> GenericFactory<BaseType, Args...>::get_registered_types() const
    {
        std::vector<RegisteredTypeInfo> types;

        for (const auto &[type, registered_type] : m_registered_types)
        {
            types.push_back(registered_type.m_info);
        }
        return types;
    }

    template <typename BaseType, typename... Args>
    template <typename KeyType, typename DerivedType>
    void GenericFactory<BaseType, Args...>::register_type(const std::string &display_name)
    {
        register_type<KeyType, DerivedType>(
            display_name,
            [](Args... args)
            {
                return std::make_unique<DerivedType>(std::forward<Args>(args)...);
            });
    }

    template <typename BaseType, typename... Args>
    template <typename KeyType, typename DerivedType>
    void GenericFactory<BaseType, Args...>::register_type(CreatorFunction creator)
    {
        register_type<KeyType, DerivedType>("", creator);
    }

    template <typename BaseType, typename... Args>
    template <typename KeyType, typename DerivedType>
    void GenericFactory<BaseType, Args...>::register_type(const std::string &display_name, CreatorFunction creator)
    {
        auto type = SceneObject::get_type<KeyType>();

        if (m_registered_types.find(type) != m_registered_types.end())
        {
            return;
        }
        m_registered_types[type] = {type, !display_name.empty() ? display_name : type, creator};
    }

    template <typename BaseType, typename... Args>
    template <typename KeyType, typename DerivedType>
    std::unique_ptr<DerivedType> GenericFactory<BaseType, Args...>::create_type(Args... args) const
    {
        return std::static_pointer_cast<DerivedType>(create_type(SceneObject::get_type<KeyType>(), std::forward<Args>(args)...));
    }

    template <typename BaseType, typename... Args>
    std::unique_ptr<BaseType> GenericFactory<BaseType, Args...>::create_type(const std::string &type, Args... args) const
    {
        auto iterator = m_registered_types.find(type);

        if (iterator == m_registered_types.end())
        {
            return nullptr;
        }
        return iterator->second.m_creator(std::forward<Args>(args)...);
    }
}