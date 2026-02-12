#include "sfg_trajectory_planner/core/scene_object.hpp"

#include "sfg_utils/cpp_utils.hpp"

namespace sfg_trajectory_planner::core
{
    template <typename ObjectType>
    std::string SceneObject::get_type()
    {
        return sfg_utils::cpp_utils::get_type<ObjectType>();
    }
}
