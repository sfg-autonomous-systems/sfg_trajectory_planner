#pragma once

namespace sfg_trajectory_planner::engine::core::gfx
{
    class IRenderable
    {
    public:
        IRenderable() = default;
        virtual ~IRenderable() = default;
        virtual void render() = 0;

    protected:
        IRenderable(const IRenderable &) = default;
        IRenderable &operator=(const IRenderable &) = default;
        IRenderable(IRenderable &&) = default;
        IRenderable &operator=(IRenderable &&) = default;
    };
}