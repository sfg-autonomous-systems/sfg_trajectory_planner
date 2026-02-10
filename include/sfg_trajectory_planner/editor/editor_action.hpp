#pragma once

#include <functional>
#include <string>

namespace sfg_trajectory_planner::editor
{
    struct EditorAction
    {
        std::string m_name;
        std::function<void()> m_action;
    };
}