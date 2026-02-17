#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"

namespace sfg_trajectory_planner::engine::core::serialization
{
    template <typename ElementType>
    void YamlSerializer::serialize_vector(const std::string &key, const std::vector<ElementType> &value)
    {
        YAML::Node node;

        for (const auto &item : value)
        {
            node.push_back(item);
        }
        node.SetStyle(YAML::EmitterStyle::Flow);
        current_node()[key] = node;
    }
}