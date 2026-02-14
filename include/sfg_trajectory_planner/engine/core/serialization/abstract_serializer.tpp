#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::engine::core::serialization
{
    template <typename ValueType>
    ValueType AbstractSerializer::deserialize(const std::string &key)
    {
        ValueType value;
        deserialize(key, value);
        return value;
    }
}