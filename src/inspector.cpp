#include "sfg_trajectory_planner/inspector.hpp"

#include "sfg_trajectory_planner/selection.hpp"

namespace sfg_trajectory_planner
{
    void Inspector::render_internal()
    {
        auto selected_object = Selection::get_selected_object();

        if (!selected_object)
        {
            return;
        }
        selected_object->render_inspector();
    }
}