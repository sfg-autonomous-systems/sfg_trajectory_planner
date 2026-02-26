#include "sfg_trajectory_planner/engine/editor/history/destroy_object_action.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    DestroyObjectAction::DestroyObjectAction(SelectionContext &selection_context, core::Scene &scene, core::SceneObject *object)
        : m_selection_context(selection_context),
          m_scene(scene),
          // It's responsibility of the caller to ensure that the object is not null.
          m_object_uuid(object->get_uuid()),
          m_parent_uuid(object->get_parent() ? object->get_parent()->get_uuid() : uuids::uuid{})
    {
        core::serialization::YamlSerializer serializer;
        m_scene.serialize_object(object, &serializer);
        m_serialized_data = serializer.to_bytes();
    }

    void DestroyObjectAction::redo()
    {
        auto object = m_scene.find_object_by_uuid(m_object_uuid);
        auto selected_object = m_selection_context.get_selected();

        // We need to check if the currently selected object is being deleted. Since the scene doesn't just destroy the object itself
        // but also all of its descendants, we also need to clear the selection if any of the descendants of the deleted object is currently selected.
        if (selected_object == object || object->is_ancestor_of(selected_object))
        {
            m_selected_object_uuid = selected_object->get_uuid();
            m_selection_context.set_selected(nullptr);
        }
        m_scene.destroy_object(object);
    }

    void DestroyObjectAction::undo()
    {
        core::serialization::YamlSerializer serializer;
        serializer.from_bytes(m_serialized_data);
        m_scene.deserialize_object(m_scene.find_object_by_uuid(m_parent_uuid), &serializer);
    }
}