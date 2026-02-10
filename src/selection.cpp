#include "sfg_trajectory_planner/selection.hpp"

namespace sfg_trajectory_planner
{
    SceneObject *Selection::s_selected_object = nullptr;

    void Selection::select_object(SceneObject *object)
    {
        s_selected_object = object;
    }

    SceneObject *Selection::get_selected_object()
    {
        return s_selected_object;
    }
}