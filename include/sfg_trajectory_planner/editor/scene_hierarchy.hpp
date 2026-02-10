#pragma once

#include <imgui/imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/core/scene.hpp"
#include "sfg_trajectory_planner/editor/editor_action.hpp"
#include "sfg_trajectory_planner/editor/selection_context.hpp"

namespace sfg_trajectory_planner::editor
{
    class SceneHierarchy : public sfg_imgui_vendor::GuiElement
    {
    public:
        SceneHierarchy(core::Scene &scene, SelectionContext &selection_context);
        void render_internal() override;
        void add_create_object_action(const EditorAction &action);

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
        std::vector<EditorAction> m_create_object_actions;
    };
}