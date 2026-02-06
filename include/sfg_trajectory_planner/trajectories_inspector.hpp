#pragma once

#include <imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/trajectory.hpp"

namespace sfg_trajectory_planner
{
    class TrajectoriesInspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        void render_internal() override;
    };
}