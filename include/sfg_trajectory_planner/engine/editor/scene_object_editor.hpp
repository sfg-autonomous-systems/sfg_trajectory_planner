#pragma once

#include <optional>

#include "sfg_trajectory_planner/engine/core/transform.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Renderer;
}

namespace sfg_trajectory_planner::engine::editor
{
    class SelectionContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditor
    {
    public:
        SceneObjectEditor(const SelectionContext &selection_context);
        virtual ~SceneObjectEditor() = default;
        SceneObjectEditor(const SceneObjectEditor &) = delete;
        SceneObjectEditor &operator=(const SceneObjectEditor &) = delete;
        SceneObjectEditor(SceneObjectEditor &&) = delete;
        SceneObjectEditor &operator=(SceneObjectEditor &&) = delete;

        virtual bool render_editor(core::gfx::Renderer &renderer);
        virtual bool render_inspector();

    protected:
        std::optional<core::Transform> render_transform_editor(core::gfx::Renderer &renderer, const core::Transform &transform, const glm::mat4 &parent_transform = glm::mat4(1.0f));
        std::optional<core::Transform> render_transform_inspector(const core::Transform &transform, bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);

        bool render_transform_editor(core::gfx::Renderer &renderer, core::Transform &transform, const glm::mat4 &parent_transform = glm::mat4(1.0f));
        bool render_transform_inspector(core::Transform &transform, bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);

        const SelectionContext &m_selection_context;
    };
}