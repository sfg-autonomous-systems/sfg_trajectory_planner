#pragma once

#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SelectionContext
    {
    public:
        SelectionContext(const engine::core::Scene &scene, const SceneObjectEditorFactory &editor_factory);
        ~SelectionContext();

        engine::core::SceneObject *get_selected() const;
        ImGuizmo::OPERATION get_gizmo_operation() const;
        ImGuizmo::MODE get_gizmo_mode() const;
        SceneObjectEditor *get_selected_editor() const;

        void set_selected(engine::core::SceneObject *object);
        void set_gizmo_operation(ImGuizmo::OPERATION operation);
        void set_gizmo_mode(ImGuizmo::MODE mode);

    private:
        const engine::core::Scene &m_scene;
        const SceneObjectEditorFactory &m_editor_factory;

        uuids::uuid m_selected_object_uuid;
        ImGuizmo::MODE m_gizmo_mode = ImGuizmo::LOCAL;
        ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::TRANSLATE;
        std::unique_ptr<SceneObjectEditor> m_selected_object_editor;
    };
}