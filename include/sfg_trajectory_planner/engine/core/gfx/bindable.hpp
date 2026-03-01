#pragma once

namespace sfg_trajectory_planner::engine::core::gfx
{
    class IBindable
    {
    public:
        IBindable() = default;
        virtual ~IBindable() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

    protected:
        IBindable(const IBindable &) = default;
        IBindable &operator=(const IBindable &) = default;
        IBindable(IBindable &&) = default;
        IBindable &operator=(IBindable &&) = default;
    };
}