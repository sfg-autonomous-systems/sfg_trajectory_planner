#pragma once

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class EditorContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    class Inspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        Inspector(const EditorContext &editor_context);
        void render_internal() override;

    private:
        const EditorContext &m_editor_context;
    };
}