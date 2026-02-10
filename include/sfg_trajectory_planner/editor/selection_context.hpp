#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner::editor
{
    class SelectionContext
    {
    public:
        SelectionContext();
        void select_object(core::SceneObject *object);
        core::SceneObject *get_selected_object();

    private:
        core::SceneObject *m_selected_object;
    };
}