#pragma once

#include <imgui/imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"
#include "sfg_trajectory_planner/scene.hpp"

namespace sfg_trajectory_planner
{
    class SceneHierarchy : public sfg_imgui_vendor::GuiElement
    {
    public:
        SceneHierarchy(Scene &scene);
        void render_internal() override;

    private:
        struct ReparentRequest
        {
            SceneObject *m_parent;
            SceneObject *m_child;
        };

        void render_object(SceneObject &object);
        void render_object_separator(SceneObject *parent);

        Scene &m_scene;
        ReparentRequest m_reparent_request;
    };
}