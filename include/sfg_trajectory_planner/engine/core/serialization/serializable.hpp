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

        virtual void serialize(AbstractSerializer *serializer) const = 0;
        virtual void deserialize(AbstractSerializer *serializer) = 0;

    protected:
        ISerializable(const ISerializable &) = default;
        ISerializable &operator=(const ISerializable &) = default;
        ISerializable(ISerializable &&) = default;
        ISerializable &operator=(ISerializable &&) = default;
    };
}