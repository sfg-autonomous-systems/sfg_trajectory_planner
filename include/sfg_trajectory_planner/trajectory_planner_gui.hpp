#pragma once

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner
{
    class TrajectoryPlannerGui : public sfg_imgui_vendor::GuiElement
    {
    public:
        TrajectoryPlannerGui();
        ~TrajectoryPlannerGui();
        TrajectoryPlannerGui(const TrajectoryPlannerGui &) = delete;
        TrajectoryPlannerGui &operator=(const TrajectoryPlannerGui &) = delete;
        TrajectoryPlannerGui(TrajectoryPlannerGui &&) = delete;
        TrajectoryPlannerGui &operator=(TrajectoryPlannerGui &&) = delete;

    protected:
        void render_internal() override;
    };
}