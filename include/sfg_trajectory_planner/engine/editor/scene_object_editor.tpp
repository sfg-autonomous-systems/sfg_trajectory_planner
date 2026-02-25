#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    template <typename ObjectType>
    ObjectType *SceneObjectEditor<ObjectType>::target() const
    {
        return dynamic_cast<ObjectType *>(SceneObjectEditor<void>::target());
    }
}