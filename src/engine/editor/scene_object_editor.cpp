#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "imgui/misc/cpp/imgui_stdlib.h"
#include "sfg_imgui_vendor/push_id_guard.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    SceneObjectEditor::SceneObjectEditor(SelectionContext &selection_context) : m_selection_context(selection_context) {}

    bool SceneObjectEditor::render_editor(engine::core::gfx::Renderer &renderer)
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

    bool SceneObjectEditor::render_transform_editor(engine::core::gfx::Renderer &renderer, engine::core::Transform &transform, const glm::mat4 &parent_transform)
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

    bool SceneObjectEditor::render_transform_inspector(engine::core::Transform &transform, bool render_translation, bool render_rotation, bool render_scale, bool render_labels)
    {
        auto changed = false;

        if (!render_labels)
        {
            ImGui::PushItemWidth(-1.0f);
        }

        auto translation = transform.get_translation();

        if (render_translation && ImGui::DragFloat3(render_labels ? "Position [m]" : "##position", glm::value_ptr(translation), 0.1f))
        {
            transform.set_translation(translation);
            changed = true;
        }
        ImGui::SetItemTooltip("Position [m]");

        auto euler_angles = transform.get_euler_angles();

        if (render_rotation && ImGui::DragFloat3(render_labels ? "Rotation [deg]" : "##rotation", glm::value_ptr(euler_angles), 0.1f))
        {
            transform.set_euler_angles(euler_angles);
            changed = true;
        }
        ImGui::SetItemTooltip("Rotation [deg]");

        auto scale = transform.get_scale();

        if (render_scale && ImGui::DragFloat3(render_labels ? "Scale" : "##scale", glm::value_ptr(scale), 0.1f))
        {
            transform.set_scale(scale);
            changed = true;
        }
        ImGui::SetItemTooltip("Scale");

        if (!render_labels)
        {
            ImGui::PopItemWidth();
        }
        return changed;
    }
}