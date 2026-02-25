#pragma once

#include <memory>
#include <vector>

namespace sfg_trajectory_planner::engine::editor::history
{
    class IAction;
}

namespace sfg_trajectory_planner::engine::editor::history
{
    class Undo
    {
    public:
        void execute(std::unique_ptr<IAction> action);
        void undo();
        void redo();

    private:
        std::vector<std::unique_ptr<IAction>> m_undo_stack;
        std::vector<std::unique_ptr<IAction>> m_redo_stack;
    };
}