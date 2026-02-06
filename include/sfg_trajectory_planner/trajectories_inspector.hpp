#pragma once

#include <imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/trajectory.hpp"

namespace sfg_trajectory_planner
{
    class TrajectoriesInspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        TrajectoriesInspector(std::vector<Trajectory> &trajectories);
        void render_internal() override;

    private:
        void render_trajectory(size_t trajectory_index);
        void render_trajectory_point(size_t trajectory_index, size_t point_index);

        std::vector<Trajectory> &m_trajectories;
    };
}