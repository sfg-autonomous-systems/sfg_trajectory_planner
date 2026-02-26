#pragma once

#include <functional>
#include <imgui/imgui.h>
// ImGuizmo needs to be included after imgui.
#include <imgui/ImGuizmo.h>
#include <uuid.h>

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
    class SceneObject;
}

namespace sfg_trajectory_planner::engine::editor
{
    class SelectionContext
    {
    public:
        SelectionContext(const core::Scene &scene, std::function<void(core::SceneObject *)> on_selection_changed);
        ~SelectionContext();

        core::SceneObject *get_selected() const;
        ImGuizmo::OPERATION get_gizmo_operation() const;
        ImGuizmo::MODE get_gizmo_mode() const;

        void set_selected(core::SceneObject *object);
        void set_gizmo_operation(ImGuizmo::OPERATION operation);
        void set_gizmo_mode(ImGuizmo::MODE mode);

    private:
        const core::Scene &m_scene;
        std::function<void(core::SceneObject *)> m_on_selection_changed;

        uuids::uuid m_object_uuid;
        ImGuizmo::MODE m_gizmo_mode = ImGuizmo::LOCAL;
        ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::TRANSLATE;
    };
}