#pragma once

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner
{
    class Inspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        void render_internal() override;
    };
}