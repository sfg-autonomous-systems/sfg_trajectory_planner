#include "sfg_trajectory_planner/engine/editor/history/destroy_object_action.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    DestroyObjectAction::DestroyObjectAction(core::Scene &scene, core::SceneObject *object)
        : m_scene(scene),
          m_parent_uuid(object && object->get_parent() ? object->get_parent()->get_uuid() : uuids::uuid{}),
          m_object_uuid(object ? object->get_uuid() : uuids::uuid{})
    {
        if (object)
        {
            core::serialization::YamlSerializer serializer;
            m_scene.serialize_object(object, &serializer);
            m_serialized_data = serializer.to_bytes();
        }
    }

    void DestroyObjectAction::redo()
    {
        auto object = m_scene.find_object_by_uuid(m_object_uuid);

        if (!object)
        {
            return;
        }
        m_scene.destroy_object(object);
    }

    void DestroyObjectAction::undo()
    {
        auto object = m_scene.find_object_by_uuid(m_object_uuid);

        if (object)
        {
            return;
        }
        core::serialization::YamlSerializer serializer;
        serializer.from_bytes(m_serialized_data);
        m_scene.deserialize_object(m_scene.find_object_by_uuid(m_parent_uuid), &serializer);
    }
}