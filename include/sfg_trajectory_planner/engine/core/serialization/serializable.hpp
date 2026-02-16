#pragma once

namespace sfg_trajectory_planner::engine::core::serialization
{
    class AbstractSerializer;
}

namespace sfg_trajectory_planner::engine::core::serialization
{
    class ISerializable
    {
    public:
        ISerializable() = default;
        virtual ~ISerializable() = default;
        ISerializable(const ISerializable &) = delete;
        ISerializable &operator=(const ISerializable &) = delete;
        ISerializable(ISerializable &&) = delete;
        ISerializable &operator=(ISerializable &&) = delete;

        virtual void serialize(AbstractSerializer *serializer) const = 0;
        virtual void deserialize(AbstractSerializer *serializer) = 0;
    };
}