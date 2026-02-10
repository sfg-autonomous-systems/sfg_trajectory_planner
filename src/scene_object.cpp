#include "sfg_trajectory_planner/scene_object.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "sfg_trajectory_planner/gfx_utils.hpp"
#include "sfg_trajectory_planner/scene.hpp"

namespace sfg_trajectory_planner
{
    SceneObject::SceneObject(SceneObjectKey, Scene &scene)
        : m_scene(scene),
          m_name("New Object"),
          m_local_transform(glm::mat4(1.0f)),
          m_parent(nullptr),
          m_visible(true)
    {
    }

    SceneObject::~SceneObject()
    {
        if (!m_parent)
        {
            return;
        }

        auto &siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    void SceneObject::render_object(const glm::mat4 &, const glm::mat4 &, const glm::vec4 &) {}

    void SceneObject::render_inspector()
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::InputText("##name", &m_name);

        if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m_local_transform, scale, orientation, translation, skew, perspective);
        glm::vec3 rotation = glm::degrees(glm::eulerAngles(orientation));
        bool update_local_transform = false;

        ImGui::Text("Position:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        update_local_transform = ImGui::DragFloat3("##position", glm::value_ptr(translation), 0.1f);

        ImGui::Text("Rotation:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        update_local_transform |= ImGui::DragFloat3("##rotation", glm::value_ptr(rotation), 0.1f);

        ImGui::Text("Scale:   ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        update_local_transform |= ImGui::DragFloat3("##scale", glm::value_ptr(scale), 0.1f);
        ImGui::Spacing();

        if (!update_local_transform)
        {
            return;
        }

        m_local_transform = glm::translate(glm::mat4(1.0f), translation) *
                            glm::mat4_cast(glm::quat(glm::radians(rotation))) *
                            glm::scale(glm::mat4(1.0f), glm::max(scale, glm::vec3(0.001f)));
    }

    std::string SceneObject::get_name() const
    {
        return m_name;
    }

    glm::mat4 SceneObject::get_local_transform() const
    {
        return m_local_transform;
    }

    glm::mat4 SceneObject::get_global_transform() const
    {
        if (m_parent)
        {
            return m_parent->get_global_transform() * m_local_transform;
        }
        return m_local_transform;
    }

    SceneObject *SceneObject::get_parent() const
    {
        if (m_parent == m_scene.get_root())
        {
            return nullptr;
        }
        return m_parent;
    }

    const std::vector<SceneObject *> &SceneObject::get_children() const
    {
        return m_children;
    }

    bool SceneObject::is_visible() const
    {
        return m_visible;
    }

    void SceneObject::set_name(std::string name)
    {
        m_name = std::move(name);
    }

    void SceneObject::set_local_transform(glm::mat4 transform)
    {
        m_local_transform = std::move(transform);
    }

    void SceneObject::set_parent(SceneObject *parent)
    {
        // If the new parent is the same as the current parent, do nothing.
        if (parent == m_parent)
        {
            return;
        }

        // We need to check if the new parent is a descendant of this object to avoid creating a cycle in the scene graph.
        for (SceneObject *ancestor = parent; ancestor != nullptr; ancestor = ancestor->get_parent())
        {
            if (ancestor == this)
            {
                return;
            }
        }

        // Delete this object from the current parent's children list.
        if (m_parent)
        {
            auto &siblings = m_parent->m_children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        // Set the new parent and add this object to the new parent's children list.
        m_parent = parent ? parent : m_scene.get_root();
        m_parent->m_children.push_back(this);
    }

    void SceneObject::set_visible(bool visible)
    {
        m_visible = visible;
    }
}