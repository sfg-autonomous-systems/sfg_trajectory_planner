#pragma once

#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner::editor
{
    class SelectionContext
    {
    public:
        SelectionContext(core::Scene &scene);

        core::SceneObject *get_selected();
        ImGuizmo::OPERATION get_gizmo_operation();
        ImGuizmo::MODE get_gizmo_mode();

        void set_selected(core::SceneObject *object);
        void set_selected(uuids::uuid uuid);
        void set_gizmo_operation(ImGuizmo::OPERATION operation);
        void set_gizmo_mode(ImGuizmo::MODE mode);

    private:
        core::Scene &m_scene;

        uuids::uuid m_selected_object_uuid;
        ImGuizmo::MODE m_gizmo_mode = ImGuizmo::LOCAL;
        ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::TRANSLATE;
    };
}