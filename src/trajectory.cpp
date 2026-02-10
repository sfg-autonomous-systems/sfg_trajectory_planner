#include "sfg_trajectory_planner/trajectory.hpp"

#include "sfg_trajectory_planner/core/gfx/utils.hpp"

#define STRINGIFY(x) #x

namespace sfg_trajectory_planner
{
    Trajectory::Trajectory(core::SceneObjectKey key, core::Scene &scene) : SceneObject(key, scene, STRINGIFY(sfg_trajectory_planner::Trajectory))
    {
    }

    void Trajectory::render_object(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, const glm::vec4 &viewport)
    {
        for (size_t child_index = 1; child_index < m_children.size(); child_index++)
        {
            core::gfx::utils::render_line(
                view_matrix,
                projection_matrix,
                viewport,
                m_children[child_index - 1]->get_global_transform()[3].xyz(),
                m_children[child_index]->get_global_transform()[3].xyz(),
                IM_COL32(255, 255, 0, 255),
                2.0f);
        }
    }

    void Trajectory::render_inspector()
    {
        SceneObject::render_inspector();
    }
}

#undef STRINGIFY