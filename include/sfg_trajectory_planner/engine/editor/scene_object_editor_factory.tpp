#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    template <typename EditorType>
    void SceneObjectEditorFactory::register_editor(const std::string &display_name)
    {
        register_type<typename EditorType::TargetObjectType, EditorType>(display_name);
    }

    template <typename EditorType>
    void SceneObjectEditorFactory::register_editor(CreatorFunction creator)
    {
        register_type<typename EditorType::TargetObjectType, EditorType>(creator);
    }
}