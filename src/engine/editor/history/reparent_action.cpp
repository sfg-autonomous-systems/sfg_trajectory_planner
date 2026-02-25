#include "sfg_trajectory_planner/engine/editor/history/reparent_action.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    ReparentAction::ReparentAction(const core::Scene &scene, uuids::uuid child_uuid, uuids::uuid old_parent_uuid, uuids::uuid new_parent_uuid)
        : m_scene(scene),
          m_child_uuid(child_uuid),
          m_old_parent_uuid(old_parent_uuid),
          m_new_parent_uuid(new_parent_uuid)
    {
    }

    void ReparentAction::redo()
    {
        auto child = m_scene.find_object_by_uuid(m_child_uuid);
        auto new_parent = m_scene.find_object_by_uuid(m_new_parent_uuid);

        if (child && new_parent)
        {
            child->set_parent(new_parent);
        }
    }

    void ReparentAction::undo()
    {
        auto child = m_scene.find_object_by_uuid(m_child_uuid);
        auto old_parent = m_scene.find_object_by_uuid(m_old_parent_uuid);

        if (child && old_parent)
        {
            child->set_parent(old_parent);
        }
    }
}