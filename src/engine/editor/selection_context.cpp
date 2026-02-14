#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    SelectionContext::SelectionContext(engine::core::Scene &scene) : m_scene(scene) {}

    engine::core::SceneObject *SelectionContext::get_selected()
    {
        return m_selected_object_uuid.is_nil() ? nullptr : m_scene.find_object_by_uuid<engine::core::SceneObject>(m_selected_object_uuid);
    }

    ImGuizmo::OPERATION SelectionContext::get_gizmo_operation()
    {
        return m_gizmo_operation;
    }

    ImGuizmo::MODE SelectionContext::get_gizmo_mode()
    {
        return m_gizmo_mode;
    }

    void SelectionContext::set_selected(engine::core::SceneObject *object)
    {
        m_selected_object_uuid = object ? object->get_uuid() : uuids::uuid{};
    }

    void SelectionContext::set_selected(uuids::uuid uuid)
    {
        m_selected_object_uuid = std::move(uuid);
    }

    void SelectionContext::set_gizmo_operation(ImGuizmo::OPERATION operation)
    {
        m_gizmo_operation = operation;
    }

    void SelectionContext::set_gizmo_mode(ImGuizmo::MODE mode)
    {
        m_gizmo_mode = mode;
    }
}