#include "sfg_trajectory_planner/editor/selection_context.hpp"

namespace sfg_trajectory_planner::editor
{
    SelectionContext::SelectionContext() : m_selected_object(nullptr) {}

    void SelectionContext::select_object(core::SceneObject *object)
    {
        m_selected_object = object;
    }

    core::SceneObject *SelectionContext::get_selected_object()
    {
        return m_selected_object;
    }
}