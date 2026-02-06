#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner
{
    class TransformInspector : public sfg_imgui_vendor::GuiElement
    {
    public:
        TransformInspector();

        void render_internal() override;

        void set_translation(const glm::vec3 &translation);
        void set_rotation(const glm::vec3 &rotation);
        void set_scale(const glm::vec3 &scale);

        glm::vec3 get_translation() const;
        glm::vec3 get_rotation() const;
        glm::vec3 get_scale() const;
        glm::mat4 get_transform_matrix() const;

    private:
        glm::vec3 m_translation;
        glm::vec3 m_rotation;
        glm::vec3 m_scale;
    };
}