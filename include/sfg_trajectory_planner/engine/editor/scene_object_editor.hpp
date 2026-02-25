#pragma once

#include <optional>

#include "sfg_trajectory_planner/engine/core/transform.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class SceneObject;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Renderer;
}

namespace sfg_trajectory_planner::engine::editor
{
    class EditorContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    template <typename ObjectType = void>
    class SceneObjectEditor;

    template <>
    class SceneObjectEditor<void>
    {
    public:
        SceneObjectEditor(EditorContext &editor_context);
        virtual ~SceneObjectEditor() = default;

        virtual void render_editor(core::gfx::Renderer &renderer);
        virtual void render_inspector();

    protected:
        SceneObjectEditor(const SceneObjectEditor &) = default;
        SceneObjectEditor &operator=(const SceneObjectEditor &) = default;
        SceneObjectEditor(SceneObjectEditor &&) = default;
        SceneObjectEditor &operator=(SceneObjectEditor &&) = default;

        core::SceneObject *target() const;

        std::optional<core::Transform> render_transform_editor(core::gfx::Renderer &renderer, const core::Transform &transform_ls, const glm::mat4 &ls_to_ws_matrix = glm::mat4(1.0f));
        bool render_transform_editor(core::gfx::Renderer &renderer, core::Transform &transform_ls, const glm::mat4 &ls_to_ws_matrix = glm::mat4(1.0f));
        std::optional<core::Transform> render_transform_inspector(const core::Transform &transform, bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);
        bool render_transform_inspector(core::Transform &transform, bool render_translation = true, bool render_rotation = true, bool render_scale = true, bool render_labels = true);

        EditorContext &m_editor_context;
    };

    template <typename ObjectType>
    class SceneObjectEditor : public SceneObjectEditor<void>
    {
    public:
        using SceneObjectEditor<void>::SceneObjectEditor;

    protected:
        ObjectType *target() const;
    };
}

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.tpp"