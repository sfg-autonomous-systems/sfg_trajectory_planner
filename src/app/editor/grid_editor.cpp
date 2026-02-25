#include "sfg_trajectory_planner/app/editor/grid_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_trajectory_planner/app/core/grid.hpp"
#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/history/record_object_action.hpp"

namespace sfg_trajectory_planner::app::editor
{
    GridEditor::GridEditor(engine::editor::EditorContext &editor_context) : SceneObjectEditor(editor_context)
    {
    }

    void GridEditor::render_inspector()
    {
        SceneObjectEditor::render_inspector();

        if (!ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        auto grid = target();
        auto dirty = false;
        auto record_object = false;
        auto grid_size = grid->get_grid_size();
        auto grid_spacing = grid->get_grid_spacing();
        auto color = grid->get_color();

        dirty |= ImGui::DragFloat2("Grid Size [m]", glm::value_ptr(grid_size), 0.1f);
        record_object |= ImGui::IsItemActivated();

        dirty |= ImGui::DragFloat("Grid Spacing [m]", &grid_spacing, 0.1f);
        record_object |= ImGui::IsItemActivated();

        dirty |= ImGui::ColorEdit3("Color", glm::value_ptr(color));
        record_object |= ImGui::IsItemActivated();

        if (record_object)
        {
            m_editor_context.m_undo.execute(std::make_unique<engine::editor::history::RecordObjectAction>(grid));
        }

        if (dirty)
        {
            grid->set_grid_size(grid_size);
            grid->set_grid_spacing(grid_spacing);
            grid->set_color(color);
        }
    }
}