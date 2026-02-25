#pragma once

#include <uuid.h>

#include "sfg_trajectory_planner/engine/editor/history/action.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
}

namespace sfg_trajectory_planner::engine::editor::history
{
    class ReparentAction : public IAction
    {
    public:
        ReparentAction(const core::Scene &scene, uuids::uuid child_uuid, uuids::uuid old_parent_uuid, uuids::uuid new_parent_uuid);
        void redo() override;
        void undo() override;

    private:
        const core::Scene &m_scene;
        uuids::uuid m_child_uuid;
        uuids::uuid m_old_parent_uuid;
        uuids::uuid m_new_parent_uuid;
    };
}