#pragma once

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditor
    {
    public:
        explicit SceneObjectEditor(SelectionContext &selection_context);
        virtual ~SceneObjectEditor() = default;
        SceneObjectEditor(const SceneObjectEditor &) = delete;
        SceneObjectEditor &operator=(const SceneObjectEditor &) = delete;
        SceneObjectEditor(SceneObjectEditor &&) = delete;
        SceneObjectEditor &operator=(SceneObjectEditor &&) = delete;

        virtual bool render_editor(engine::core::gfx::Renderer &renderer);
        virtual bool render_inspector();

    protected:
        bool render_transform_editor(engine::core::gfx::Renderer &renderer, engine::core::Transform &transform, const glm::mat4 &parent_transform = glm::mat4(1.0f));
        bool render_transform_inspector(engine::core::Transform &transform, bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);

        SelectionContext &m_selection_context;
    };
}