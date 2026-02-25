#pragma once

#include <imgui/imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
    class SceneObject;
}

namespace sfg_trajectory_planner::engine::editor
{
    class EditorContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    class SceneHierarchy : public sfg_imgui_vendor::GuiElement
    {
    public:
        SceneHierarchy(core::Scene &scene, EditorContext &editor_context);
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
        EditorContext &m_editor_context;
        ReparentRequest m_reparent_request;
    };
}