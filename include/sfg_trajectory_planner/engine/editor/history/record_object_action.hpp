#pragma once

#include <uuid.h>

#include "sfg_trajectory_planner/engine/editor/history/action.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
    class SceneObject;
}

namespace sfg_trajectory_planner::engine::editor::history
{
    class RecordObjectAction : public IAction
    {
    public:
        RecordObjectAction(core::SceneObject *object);
        void redo() override;
        void undo() override;

    private:
        const core::Scene &m_scene;
        uuids::uuid m_object_uuid;
        std::vector<std::uint8_t> m_serialized_data;
    };
}