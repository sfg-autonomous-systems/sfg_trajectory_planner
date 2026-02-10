#include "sfg_trajectory_planner/editor/inspector.hpp"

namespace sfg_trajectory_planner::editor
{
    Inspector::Inspector(SelectionContext &selection_context) : m_selection_context(selection_context) {}

    void Inspector::render_internal()
    {
        auto selected_object = m_selection_context.get_selected_object();

        if (!selected_object)
        {
            return;
        }
        selected_object->render_inspector();
    }
}