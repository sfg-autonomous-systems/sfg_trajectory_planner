#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::core::serialization
{
    template <typename ValueType>
    ValueType AbstractSerializer::deserialize(const std::string &key)
    {
        ValueType value;
        deserialize(key, value);
        return value;
    }
}