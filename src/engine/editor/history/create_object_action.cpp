#include "sfg_trajectory_planner/engine/editor/history/create_object_action.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    CreateObjectAction::CreateObjectAction(SelectionContext &selection_context, core::Scene &scene, std::string type, std::string name, core::SceneObject *parent)
        : m_selection_context(selection_context),
          m_scene(scene),
          m_type(std::move(type)),
          m_name(std::move(name)),
          m_parent_uuid(parent ? parent->get_uuid() : uuids::uuid{})
    {
    }

    void CreateObjectAction::redo()
    {
        auto parent = m_scene.find_object_by_uuid(m_parent_uuid);

        if (m_object_uuid.is_nil())
        {
            auto object = m_scene.create_object(m_type, m_name, parent);

            if (!object)
            {
                return;
            }
            m_object_uuid = object->get_uuid();
            core::serialization::YamlSerializer serializer;
            m_scene.serialize_object(object, &serializer);
            m_serialized_data = serializer.to_bytes();
        }
        else
        {
            core::serialization::YamlSerializer serializer;
            serializer.from_bytes(m_serialized_data);
            m_scene.deserialize_object(parent, &serializer);
        }
    }

    void CreateObjectAction::undo()
    {
        auto object = m_scene.find_object_by_uuid(m_object_uuid);

        if (!object)
        {
            return;
        }

        auto selected_object = m_selection_context.get_selected();

        if (selected_object == object || object->is_ancestor_of(selected_object))
        {
            m_selection_context.set_selected(nullptr);
        }
        m_scene.destroy_object(object);
    }
}