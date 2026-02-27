#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    EditorContext::EditorContext(core::Scene &scene, const SceneObjectEditorFactory &editor_factory)
        : m_selection_context(
              scene,
              [this, &editor_factory](core::SceneObject *object)
              {
                  m_editor = object ? editor_factory.create_type(object->get_type(), *this) : nullptr;
              })
    {
    }
}