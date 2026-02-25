#pragma once

#include <uuid.h>

#include "sfg_trajectory_planner/engine/editor/history/action.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
}

namespace sfg_trajectory_planner::engine::editor::history
{
    class RecordObjectAction : public IAction
    {
    public:
        RecordObjectAction(const core::Scene &scene, uuids::uuid object_uuid);
        void redo() override;
        void undo() override;

    private:
        const core::Scene &m_scene;
        uuids::uuid m_object_uuid;
        std::vector<std::uint8_t> m_serialized_data;
    };
}