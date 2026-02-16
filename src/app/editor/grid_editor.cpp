#include "sfg_trajectory_planner/app/editor/grid_editor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_trajectory_planner/app/core/grid.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::app::editor
{
    GridEditor::GridEditor(const engine::editor::SelectionContext &selection_context) : SceneObjectEditor(selection_context) {}

    bool GridEditor::render_inspector()
    {
        auto changed = SceneObjectEditor::render_inspector();

        if (!ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return changed;
        }

        auto grid = dynamic_cast<core::Grid *>(m_selection_context.get_selected());
        auto grid_size = grid->get_grid_size();

        if (ImGui::DragFloat2("Grid Size [m]", glm::value_ptr(grid_size), 0.1f))
        {
            grid->set_grid_size(grid_size);
            changed = true;
        }

        auto grid_spacing = grid->get_grid_spacing();

        if (ImGui::DragFloat("Grid Spacing [m]", &grid_spacing, 0.1f))
        {
            grid->set_grid_spacing(grid_spacing);
            changed = true;
        }

        auto color = grid->get_color();

        if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
        {
            grid->set_color(color);
            changed = true;
        }
        return changed;
    }
}