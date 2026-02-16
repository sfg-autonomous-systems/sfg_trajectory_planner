#pragma once

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class Inspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        Inspector(const SelectionContext &selection_context);
        void render_internal() override;

    private:
        const SelectionContext &m_selection_context;
    };
}