#include "sfg_trajectory_planner/app/editor/trajectory_editor.hpp"

#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/app/core/trajectory.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/history/record_object_action.hpp"

namespace sfg_trajectory_planner::app::editor
{
    static constexpr auto s_waypoints_table_flags = ImGuiTableFlags_BordersV |
                                                    ImGuiTableFlags_BordersOuterH |
                                                    ImGuiTableFlags_RowBg |
                                                    ImGuiTableFlags_SizingFixedFit;

    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";
    static constexpr auto s_modify_waypoint_constraints_popup_id = "modify_waypoint_constraints_popup";
    static constexpr auto s_modify_waypoint_constraints_button_text = "C";

    TrajectoryEditor::TrajectoryEditor(engine::editor::EditorContext &editor_context) : SceneObjectEditor(editor_context)
    {
    }

    void TrajectoryEditor::render_editor(engine::core::gfx::Renderer &renderer)
    {
        SceneObjectEditor::render_editor(renderer);
        auto trajectory = target();
        auto ls_to_ws_matrix = trajectory->get_ls_to_ws_matrix();
        auto gizmo_operation = m_editor_context.m_selection_context.get_gizmo_operation();

        if (m_selected_waypoint_index < trajectory->get_waypoint_count())
        {
            if ((gizmo_operation == ImGuizmo::OPERATION::TRANSLATE && trajectory->can_translate_waypoint(m_selected_waypoint_index)) ||
                (gizmo_operation == ImGuizmo::OPERATION::ROTATE && trajectory->can_rotate_waypoint(m_selected_waypoint_index)) ||
                (gizmo_operation == ImGuizmo::OPERATION::SCALE && trajectory->can_scale_waypoint(m_selected_waypoint_index)))
            {
                const auto &transform_ls = trajectory->get_waypoint_transform_ls(m_selected_waypoint_index);

                if (auto modified_transform_ls = render_transform_editor(renderer, transform_ls, ls_to_ws_matrix))
                {
                    trajectory->set_waypoint_transform_ls(m_selected_waypoint_index, modified_transform_ls.value());
                }
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            auto distance_from_camera_squared = std::numeric_limits<float>::max();
            glm::vec3 camera_position_ws = glm::inverse(renderer.get_camera().get_ws_to_vs_matrix())[3].xyz();

            for (size_t index = 0; index < trajectory->get_waypoint_count(); index++)
            {
                glm::vec3 waypoint_position = glm::vec3(ls_to_ws_matrix * glm::vec4(trajectory->get_waypoint_transform_ls(index).get_translation(), 1.0f));
                auto screen_position = renderer.get_camera().world_to_screen_point(waypoint_position);
                auto can_select_waypoint = !ImGuizmo::IsOver() && !ImGuizmo::IsUsingAny();
                auto is_hovering_waypoint = glm::length(screen_position.xy() - glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y)) < 10.0f && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
                auto distance_squared = glm::length2(camera_position_ws - waypoint_position);

                if (can_select_waypoint && is_hovering_waypoint && distance_squared < distance_from_camera_squared)
                {
                    m_selected_waypoint_index = index;
                    distance_from_camera_squared = distance_squared;
                }
            }

            if (distance_from_camera_squared == std::numeric_limits<float>::max() && !ImGuizmo::IsUsingAny() && ImGui::IsWindowHovered())
            {
                m_selected_waypoint_index = std::numeric_limits<size_t>::max();
            }
        }
    }

    void TrajectoryEditor::render_inspector()
    {
        SceneObjectEditor::render_inspector();

        if (!ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        if (ImGui::Button("Follow Trajectory", ImVec2(-1.0f, 0.0f)))
        {
            target()->follow_trajectory();
        }

        auto trajectory = target();
        auto dirty = false;
        auto record_object = false;
        auto add_waypoint = false;
        auto action_name = trajectory->get_action_name();
        auto frame_id = trajectory->get_frame_id();
        auto color = trajectory->get_color();

        dirty |= ImGui::InputText("Action Name", &action_name, ImGuiInputTextFlags_EnterReturnsTrue);
        record_object |= ImGui::IsItemActivated();

        dirty |= ImGui::InputText("Frame ID", &frame_id, ImGuiInputTextFlags_EnterReturnsTrue);
        record_object |= ImGui::IsItemActivated();

        dirty |= ImGui::ColorEdit3("Color", glm::value_ptr(color));
        record_object |= ImGui::IsItemActivated();

        if (ImGui::BeginTable("waypoints_table", 4, s_waypoints_table_flags))
        {
            ImGui::TableSetupColumn("Waypoint", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Time from last [s]", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Transform", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentDisable, ImGui::CalcTextSize(s_add_button_text).x + ImGui::GetStyle().ItemSpacing.x);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::TableHeader("Waypoint");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Time from last [s]");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Transform");

            ImGui::TableNextColumn();

            add_waypoint = ImGui::Button(s_add_button_text);
            dirty |= add_waypoint;
            record_object |= ImGui::IsItemActivated();
            ImGui::SetItemTooltip("Add Waypoint");

            for (size_t index = 0; index < trajectory->get_waypoint_count(); index++)
            {
                sfg_imgui_vendor::PushIdGuard id_guard(index);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Selectable("##waypoint_selectable", m_selected_waypoint_index == index))
                {
                    m_selected_waypoint_index = index;
                }

                ImGui::SameLine();
                ImGui::Text("Waypoint %zu", index);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);

                auto waypoint_dirty = false;
                auto waypoint_record_object = false;
                auto remove_waypoint = false;
                auto time_from_last = trajectory->get_waypoint_time_from_last(index);
                auto transform_ls = trajectory->get_waypoint_transform_ls(index);
                auto constraints = trajectory->get_waypoint_constraints(index);

                waypoint_dirty |= ImGui::DragFloat("##time_from_last", &time_from_last, 0.01f, 0.0f, std::numeric_limits<float>::max());
                waypoint_record_object |= ImGui::IsItemActivated();

                ImGui::TableNextColumn();

                if (render_transform_inspector(transform_ls, trajectory->can_translate_waypoint(index), trajectory->can_rotate_waypoint(index), trajectory->can_scale_waypoint(index), false))
                {
                    trajectory->set_waypoint_transform_ls(index, transform_ls);
                }

                ImGui::TableNextColumn();

                remove_waypoint = ImGui::Button(s_remove_button_text);
                waypoint_dirty |= remove_waypoint;
                waypoint_record_object |= ImGui::IsItemActivated();
                ImGui::SetItemTooltip("Remove Waypoint");

                if (ImGui::Button(s_modify_waypoint_constraints_button_text))
                {
                    ImGui::OpenPopup(s_modify_waypoint_constraints_popup_id);
                }
                ImGui::SetItemTooltip("Modify Waypoint Constraints");

                if (ImGui::BeginPopup(s_modify_waypoint_constraints_popup_id))
                {
                    using namespace magic_enum::bitwise_operators;

                    for (auto constraint : magic_enum::enum_values<core::Waypoint::Constraints>())
                    {
                        bool selected = (constraints & constraint) != core::Waypoint::Constraints::None;

                        if (ImGui::Selectable(magic_enum::enum_name(constraint).data(), selected, ImGuiSelectableFlags_DontClosePopups))
                        {
                            if (selected)
                            {
                                constraints &= ~constraint;
                            }
                            else
                            {
                                constraints |= constraint;
                            }
                            waypoint_dirty = true;
                            waypoint_record_object |= ImGui::IsItemActivated();
                        }
                    }
                    ImGui::EndPopup();
                }

                if (waypoint_record_object)
                {
                    m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(trajectory));
                }

                if (waypoint_dirty)
                {
                    trajectory->set_waypoint_time_from_last(index, time_from_last);
                    trajectory->set_waypoint_constraints(index, constraints);

                    if (remove_waypoint)
                    {
                        if (m_selected_waypoint_index == index)
                        {
                            m_selected_waypoint_index = std::numeric_limits<size_t>::max();
                        }
                        else if (m_selected_waypoint_index > index && m_selected_waypoint_index != std::numeric_limits<size_t>::max())
                        {
                            m_selected_waypoint_index--;
                        }
                        trajectory->remove_waypoint(index--);
                    }
                }
            }
            ImGui::EndTable();
        }

        if (record_object)
        {
            m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(trajectory));
        }

        if (dirty)
        {
            trajectory->set_action_name(action_name);
            trajectory->set_frame_id(frame_id);
            trajectory->set_color(color);

            if (add_waypoint)
            {
                trajectory->add_waypoint();
            }
        }
    }
}