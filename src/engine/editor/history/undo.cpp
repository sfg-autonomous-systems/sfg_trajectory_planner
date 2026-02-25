#include "sfg_trajectory_planner/engine/editor/history/undo.hpp"

#include "sfg_trajectory_planner/engine/editor/history/action.hpp"

namespace sfg_trajectory_planner::engine::editor::history
{
    void Undo::execute(std::unique_ptr<IAction> action)
    {
        action->redo();
        m_undo_stack.push_back(std::move(action));
        m_redo_stack.clear();
    }

    void Undo::undo()
    {
        if (m_undo_stack.empty())
        {
            return;
        }

        auto action = std::move(m_undo_stack.back());
        m_undo_stack.pop_back();
        action->undo();
        m_redo_stack.push_back(std::move(action));
    }

    void Undo::redo()
    {
        if (m_redo_stack.empty())
        {
            return;
        }

        auto action = std::move(m_redo_stack.back());
        m_redo_stack.pop_back();
        action->redo();
        m_undo_stack.push_back(std::move(action));
    }
}