#include "sfg_trajectory_planner/engine/editor/history/record_object_action.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    RecordObjectAction::RecordObjectAction(core::SceneObject *object)
        : m_scene(object->get_scene()),
          m_object_uuid(object ? object->get_uuid() : uuids::uuid{})
    {
        if (!object)
        {
            return;
        }

        core::serialization::YamlSerializer serializer;
        object->serialize(&serializer);
        m_serialized_data = serializer.to_bytes();
    }

    void RecordObjectAction::redo()
    {
        auto object = m_scene.find_object_by_uuid(m_object_uuid);

        if (!object)
        {
            return;
        }

        // Serialize the current state of the object before deserializing the previous state.
        core::serialization::YamlSerializer serializer;
        object->serialize(&serializer);
        auto serialized_data = serializer.to_bytes();

        // Deserialize the previous saved state of the object.
        serializer.from_bytes(m_serialized_data);
        object->deserialize(&serializer);
        m_serialized_data = std::move(serialized_data);
    }

    void RecordObjectAction::undo()
    {
        redo();
    }
}