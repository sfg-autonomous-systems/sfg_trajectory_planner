#include "sfg_trajectory_planner/engine/editor/scene_hierarchy.hpp"

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    static constexpr auto s_scene_objects_table_flags = ImGuiTableFlags_BordersV |
                                                        ImGuiTableFlags_BordersOuterH |
                                                        ImGuiTableFlags_RowBg |
                                                        ImGuiTableFlags_SizingFixedFit;
    static constexpr auto s_base_tree_node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                                   ImGuiTreeNodeFlags_DefaultOpen |
                                                   ImGuiTreeNodeFlags_DrawLinesToNodes;

    static constexpr auto s_add_button_text = "+";
    static constexpr auto s_remove_button_text = "-";
    static constexpr auto s_create_object_popup_id = "create_object_popup";

    SceneHierarchy::SceneHierarchy(core::Scene &scene, SelectionContext &selection_context)
        : GuiElement(),
          m_scene(scene),
          m_selection_context(selection_context),
          m_reparent_request({nullptr, nullptr})
    {
    }

    void SceneHierarchy::render_internal()
    {
        if (ImGui::BeginTable("scene_objects_table", 3, s_scene_objects_table_flags))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentDisable, ImGui::GetFrameHeight());
            ImGui::TableSetupColumn("Objects", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentDisable, ImGui::CalcTextSize(s_add_button_text).x + ImGui::GetStyle().ItemSpacing.x);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::TableHeader("##visibility");

            ImGui::TableNextColumn();
            ImGui::TableHeader("Objects");

            ImGui::TableNextColumn();

            if (ImGui::Button(s_add_button_text))
            {
                ImGui::OpenPopup(s_create_object_popup_id);
            }

            if (ImGui::BeginPopup(s_create_object_popup_id))
            {
                for (const auto &type : m_scene.get_possible_types())
                {
                    if (ImGui::MenuItem(type.m_display_name.c_str()))
                    {
                        m_scene.create_object(type.m_type, type.m_display_name);
                    }
                }
                ImGui::EndPopup();
            }

            for (auto &child : m_scene.get_root()->get_children())
            {
                render_object(*child);
            }
            render_object_separator(nullptr);
            ImGui::EndTable();
        }

        if (m_reparent_request.m_child)
        {
            m_reparent_request.m_child->set_parent(m_reparent_request.m_parent);
            m_reparent_request = {nullptr, nullptr};
        }
    }

    void SceneHierarchy::render_object(core::SceneObject &object)
    {
        sfg_imgui_vendor::PushIdGuard id_guard(&object);
        ImGui::TableNextRow();

        // Visibility column.
        ImGui::TableNextColumn();
        auto visible = object.is_visible();

        if (ImGui::Checkbox("##visibility", &visible))
        {
            object.set_visible(visible);

            if (!visible && m_selection_context.get_selected() == &object)
            {
                m_selection_context.set_selected(nullptr);
            }
        }

        // Name column.
        ImGui::TableNextColumn();
        auto flags = s_base_tree_node_flags;

        if (object.get_children().empty())
        {
            flags = flags | ImGuiTreeNodeFlags_Leaf;
        }

        if (m_selection_context.get_selected() == &object)
        {
            flags = flags | ImGuiTreeNodeFlags_Selected;
        }

        auto expanded = ImGui::TreeNodeEx(object.get_name().c_str(), flags);

        if (ImGui::IsItemClicked())
        {
            m_selection_context.set_selected(&object);
        }

        if (ImGui::BeginDragDropSource())
        {
            auto dragged_object = &object;
            ImGui::SetDragDropPayload("SCENE_HIERARCHY_DRAGGED_OBJECT", &dragged_object, sizeof(dragged_object));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (auto payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_DRAGGED_OBJECT"))
            {
                auto dragged_object = *static_cast<core::SceneObject **>(payload->Data);
                m_reparent_request.m_parent = &object;
                m_reparent_request.m_child = dragged_object;
            }
            ImGui::EndDragDropTarget();
        }

        // Remove column.
        ImGui::TableNextColumn();

        if (ImGui::Button(s_remove_button_text))
        {
            if (m_selection_context.get_selected() == &object)
            {
                m_selection_context.set_selected(nullptr);
            }
            m_scene.destroy_object(&object);

            if (expanded)
            {
                ImGui::TreePop();
            }
            return;
        }

        if (expanded)
        {
            for (auto &child : object.get_children())
            {
                render_object(*child);
            }
            ImGui::TreePop();
        }
    }

    void SceneHierarchy::render_object_separator(core::SceneObject *parent)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::InvisibleButton("##drop_separator", ImVec2(-1, 8.0f));

        if (ImGui::BeginDragDropTarget())
        {
            if (auto payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_DRAGGED_OBJECT"))
            {
                auto dragged_object = *static_cast<core::SceneObject **>(payload->Data);
                m_reparent_request.m_parent = parent;
                m_reparent_request.m_child = dragged_object;
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopStyleVar();
    }
}