#pragma once

namespace sfg_trajectory_planner::engine::editor::history
{
    class IAction
    {
    public:
        IAction() = default;
        virtual ~IAction() = default;
        virtual void redo() = 0;
        virtual void undo() = 0;

    protected:
        IAction(const IAction &) = default;
        IAction &operator=(const IAction &) = default;
        IAction(IAction &&) = default;
        IAction &operator=(IAction &&) = default;
    };
}