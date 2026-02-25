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
    class CreateObjectAction : public IAction
    {
    public:
        CreateObjectAction(core::Scene &scene, std::string type, std::string name, core::SceneObject *parent);
        void redo() override;
        void undo() override;

    private:
        core::Scene &m_scene;
        std::string m_type;
        std::string m_name;
        uuids::uuid m_parent_uuid;
        uuids::uuid m_object_uuid;
        std::vector<std::uint8_t> m_serialized_data;
    };
}