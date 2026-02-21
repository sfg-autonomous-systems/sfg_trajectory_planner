#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    SceneObjectEditor::SceneObjectEditor(const SelectionContext &selection_context) : m_selection_context(selection_context)
    {
    }

    bool SceneObjectEditor::render_editor(core::gfx::Renderer &renderer)
    {
        auto *selected_object = m_selection_context.get_selected();

        return render_transform_editor(
            renderer,
            selected_object->get_transform(),
            selected_object->get_parent() ? selected_object->get_parent()->get_object_to_world_matrix() : glm::mat4(1.0f));
    }

    bool SceneObjectEditor::render_inspector()
    {
        auto changed = false;
        auto *selected_object = m_selection_context.get_selected();

        sfg_imgui_vendor::PushIdGuard guard(this);
        auto name = selected_object->get_name();

        if (ImGui::InputText("Name", &name))
        {
            selected_object->set_name(name);
            changed = true;
        }

        auto type = selected_object->get_type();
        ImGui::BeginDisabled();
        ImGui::InputText("Type", &type);

        auto uuid = uuids::to_string(selected_object->get_uuid());
        ImGui::InputText("UUID", &uuid);
        ImGui::EndDisabled();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= render_transform_inspector(selected_object->get_transform());
        }
        return changed;
    }

    std::optional<core::Transform> SceneObjectEditor::render_transform_editor(core::gfx::Renderer &renderer, const core::Transform &transform, const glm::mat4 &parent_transform)
    {
        std::optional<core::Transform> modified_transform = transform;

        if (render_transform_editor(renderer, modified_transform.value(), parent_transform))
        {
            return modified_transform;
        }
        return std::nullopt;
    }

    std::optional<core::Transform> SceneObjectEditor::render_transform_inspector(const core::Transform &transform, bool can_translate, bool can_rotate, bool can_scale, bool render_labels)
    {
        std::optional<core::Transform> modified_transform = transform;

        if (render_transform_inspector(modified_transform.value(), can_translate, can_rotate, can_scale, render_labels))
        {
            return modified_transform;
        }
        return std::nullopt;
    }

    bool SceneObjectEditor::render_transform_editor(core::gfx::Renderer &renderer, core::Transform &transform, const glm::mat4 &parent_transform)
    {
        auto changed = false;
        auto object_to_world_matrix = parent_transform * transform.get_matrix();

        if (renderer.add_gizmo(object_to_world_matrix, m_selection_context.get_gizmo_operation(), m_selection_context.get_gizmo_mode(), &transform))
        {
            transform.set_matrix(glm::inverse(parent_transform) * object_to_world_matrix);
            changed = true;
        }
        return changed;
    }

    bool SceneObjectEditor::render_transform_inspector(core::Transform &transform, bool can_translate, bool can_rotate, bool can_scale, bool render_labels)
    {
        auto changed = false;

        if (!render_labels)
        {
            ImGui::PushItemWidth(-1.0f);
        }

        ImGui::BeginDisabled(!can_translate);
        auto translation = transform.get_translation();

        if (ImGui::DragFloat3(render_labels ? "Position [m]" : "##position", glm::value_ptr(translation), 0.01f))
        {
            transform.set_translation(translation);
            changed = true;
        }
        ImGui::SetItemTooltip("Position [m]");
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!can_rotate);
        auto euler_angles = transform.get_euler_angles();

        if (ImGui::DragFloat3(render_labels ? "Rotation [deg]" : "##rotation", glm::value_ptr(euler_angles), 0.01f))
        {
            transform.set_euler_angles(euler_angles);
            changed = true;
        }
        ImGui::SetItemTooltip("Rotation [deg]");
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!can_scale);
        auto scale = transform.get_scale();

        if (ImGui::DragFloat3(render_labels ? "Scale" : "##scale", glm::value_ptr(scale), 0.01f))
        {
            transform.set_scale(scale);
            changed = true;
        }
        ImGui::SetItemTooltip("Scale");
        ImGui::EndDisabled();

        if (!render_labels)
        {
            ImGui::PopItemWidth();
        }
        return changed;
    }
}