#pragma once

#include <imgui/imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneHierarchy : public sfg_imgui_vendor::GuiElement
    {
    public:
        SceneHierarchy(engine::core::Scene &scene, SelectionContext &selection_context);
        void render_internal() override;

    private:
        struct ReparentRequest
        {
            engine::core::SceneObject *m_parent;
            engine::core::SceneObject *m_child;
        };

        void render_object(engine::core::SceneObject &object);
        void render_object_separator(engine::core::SceneObject *parent);

        engine::core::Scene &m_scene;
        SelectionContext &m_selection_context;
        ReparentRequest m_reparent_request;
    };
}