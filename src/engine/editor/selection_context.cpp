#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    SelectionContext::SelectionContext(const core::Scene &scene, const SceneObjectEditorFactory &editor_factory) : m_scene(scene), m_editor_factory(editor_factory) {}

    SelectionContext::~SelectionContext() = default;

    core::SceneObject *SelectionContext::get_selected() const
    {
        return m_selected_object_uuid.is_nil() ? nullptr : m_scene.find_object_by_uuid<core::SceneObject>(m_selected_object_uuid);
    }

    ImGuizmo::OPERATION SelectionContext::get_gizmo_operation() const
    {
        return m_gizmo_operation;
    }

    ImGuizmo::MODE SelectionContext::get_gizmo_mode() const
    {
        return m_gizmo_mode;
    }

    SceneObjectEditor *SelectionContext::get_selected_editor() const
    {
        return get_selected() == nullptr ? nullptr : m_selected_object_editor.get();
    }

    void SelectionContext::set_selected(core::SceneObject *object)
    {
        auto uuid = object ? object->get_uuid() : uuids::uuid{};

        if (uuid == m_selected_object_uuid)
        {
            return;
        }
        m_selected_object_uuid = uuid;

        if (m_selected_object_uuid.is_nil())
        {
            m_selected_object_editor.reset();
        }
        else
        {
            m_selected_object_editor = m_editor_factory.create_object(object->get_type(), *this);
        }
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