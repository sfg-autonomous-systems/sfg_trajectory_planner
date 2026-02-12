#pragma once

#include <imgui/imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/core/scene.hpp"
#include "sfg_trajectory_planner/editor/selection_context.hpp"

namespace sfg_trajectory_planner::editor
{
    class SceneHierarchy : public sfg_imgui_vendor::GuiElement
    {
    public:
        SceneHierarchy(core::Scene &scene, SelectionContext &selection_context);
        void render_internal() override;

    private:
        struct ReparentRequest
        {
            core::SceneObject *m_parent;
            core::SceneObject *m_child;
        };

        void render_object(core::SceneObject &object);
        void render_object_separator(core::SceneObject *parent);

        core::Scene &m_scene;
        SelectionContext &m_selection_context;
        ReparentRequest m_reparent_request;
    };
}