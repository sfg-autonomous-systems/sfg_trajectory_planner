#include "sfg_trajectory_planner/core/scene_object.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "sfg_trajectory_planner/core/gfx/utils.hpp"
#include "sfg_trajectory_planner/core/scene.hpp"

#ifdef __GNUG__ // GCC/Clang
#include <cxxabi.h>
#include <memory>
std::string demangle(const char *name)
{
    int status = -4;
    std::unique_ptr<char, void (*)(void *)> res{
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free};
    return (status == 0) ? res.get() : name;
}
#else // MSVC
std::string demangle(const char *name) { return name; }
#endif

namespace sfg_trajectory_planner::core
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
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##name", &m_name);

        auto type = get_type();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Type:");
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##type", &type);
        ImGui::EndDisabled();

        if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        glm::vec3 scale = m_local_transform.get_scale();
        glm::quat rotation = m_local_transform.get_rotation();
        glm::vec3 rotation_in_euler_angles = glm::degrees(glm::eulerAngles(rotation));
        glm::vec3 translation = m_local_transform.get_translation();

        ImGui::Text("Position:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        if (ImGui::DragFloat3("##position", glm::value_ptr(translation), 0.1f))
        {
            m_local_transform.set_translation(translation);
        }

        ImGui::Text("Rotation:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::DragFloat3("##rotation_in_euler_angles", glm::value_ptr(rotation_in_euler_angles), 0.1f))
        {
            m_local_transform.set_rotation(glm::quat(glm::radians(rotation_in_euler_angles)));
        }

        ImGui::Text("Scale:   ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        if (ImGui::DragFloat3("##scale", glm::value_ptr(scale), 0.1f))
        {
            m_local_transform.set_scale(glm::max(scale, glm::vec3(0.001f)));
        }

        ImGui::Spacing();
    }

    std::string SceneObject::get_name() const
    {
        return m_name;
    }

    std::string SceneObject::get_type() const
    {
        return demangle(typeid(*this).name());
    }

    glm::mat4 SceneObject::get_local_transform() const
    {
        return m_local_transform.get_matrix();
    }

    glm::mat4 SceneObject::get_global_transform() const
    {
        if (m_parent)
        {
            return m_parent->get_global_transform() * m_local_transform.get_matrix();
        }
        return m_local_transform.get_matrix();
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