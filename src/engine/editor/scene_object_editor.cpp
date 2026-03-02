#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <rclcpp/logging.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"
#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/history/record_object_action.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    SceneObjectEditor<void>::SceneObjectEditor(EditorContext &editor_context) : m_editor_context(editor_context)
    {
    }

    void SceneObjectEditor<void>::render_editor(core::gfx::Renderer &renderer)
    {
        auto *object = target();

        render_transform_editor(
            renderer,
            object->get_transform_ls(),
            object->get_parent() ? object->get_parent()->get_ls_to_ws_matrix() : glm::mat4(1.0f));
    }

    void SceneObjectEditor<void>::render_inspector()
    {
        auto *object = target();
        auto dirty = false;
        auto record_object = false;
        auto name = object->get_name();
        auto type = object->get_type();
        auto uuid = uuids::to_string(object->get_uuid());

        dirty |= ImGui::InputText("Name", &name);
        record_object |= ImGui::IsItemActivated();

        ImGui::BeginDisabled();
        ImGui::InputText("Type", &type);
        ImGui::InputText("UUID", &uuid);
        ImGui::EndDisabled();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            render_transform_inspector(object->get_transform_ls());
        }

        if (record_object)
        {
            m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(object));
        }

        if (dirty)
        {
            object->set_name(name);
        }
    }

    core::SceneObject *SceneObjectEditor<void>::target() const
    {
        return m_editor_context.m_selection_context.get_selected();
    }

    std::optional<core::Transform> SceneObjectEditor<void>::render_transform_editor(core::gfx::Renderer &renderer, const core::Transform &transform_ls, const glm::mat4 &ls_to_ws_matrix)
    {
        std::optional<core::Transform> modified_transform_ls = transform_ls;

        if (render_transform_editor(renderer, modified_transform_ls.value(), ls_to_ws_matrix))
        {
            return modified_transform_ls;
        }
        return std::nullopt;
    }

    bool SceneObjectEditor<void>::render_transform_editor(core::gfx::Renderer &renderer, core::Transform &transform_ls, const glm::mat4 &ls_to_ws_matrix)
    {
        auto dirty = false;
        auto record_object = false;
        auto ws_matrix = ls_to_ws_matrix * transform_ls.get_matrix();

        auto was_using = ImGuizmo::IsUsingAny();
        dirty |= renderer.add_gizmo(ws_matrix, m_editor_context.m_selection_context.get_gizmo_operation(), m_editor_context.m_selection_context.get_gizmo_mode(), &transform_ls);
        record_object |= !was_using && ImGuizmo::IsUsingAny();

        if (record_object)
        {
            m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(target()));
        }

        if (dirty)
        {
            transform_ls.set_matrix(glm::inverse(ls_to_ws_matrix) * ws_matrix);
        }
        return dirty;
    }

    std::optional<core::Transform> SceneObjectEditor<void>::render_transform_inspector(const core::Transform &transform, bool can_translate, bool can_rotate, bool can_scale, bool render_labels)
    {
        std::optional<core::Transform> modified_transform = transform;

        if (render_transform_inspector(modified_transform.value(), can_translate, can_rotate, can_scale, render_labels))
        {
            return modified_transform;
        }
        return std::nullopt;
    }

    bool SceneObjectEditor<void>::render_transform_inspector(core::Transform &transform, bool can_translate, bool can_rotate, bool can_scale, bool render_labels)
    {
        auto dirty = false;
        auto record_object = false;

        if (!render_labels)
        {
            ImGui::PushItemWidth(-1.0f);
        }

        auto translation = transform.get_translation();
        auto euler_angles = transform.get_euler_angles();
        auto scale = transform.get_scale();

        ImGui::BeginDisabled(!can_translate);
        dirty |= ImGui::DragFloat3(render_labels ? "Position [m]" : "##position", glm::value_ptr(translation), 0.01f);
        record_object |= ImGui::IsItemActivated();
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!can_rotate);
        dirty |= ImGui::DragFloat3(render_labels ? "Rotation [deg]" : "##rotation", glm::value_ptr(euler_angles), 0.01f);
        record_object |= ImGui::IsItemActivated();
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!can_scale);
        dirty |= ImGui::DragFloat3(render_labels ? "Scale" : "##scale", glm::value_ptr(scale), 0.01f);
        record_object |= ImGui::IsItemActivated();
        ImGui::EndDisabled();

        if (!render_labels)
        {
            ImGui::PopItemWidth();
        }

        if (record_object)
        {
            m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(target()));
        }

        if (dirty)
        {
            transform.set_translation(translation);
            transform.set_euler_angles(euler_angles);
            transform.set_scale(scale);
        }
        return dirty;
    }
}