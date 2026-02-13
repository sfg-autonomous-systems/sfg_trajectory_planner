#include "sfg_trajectory_planner/core/scene_object.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "sfg_imgui_vendor/push_id_guard.hpp"
#include "sfg_trajectory_planner/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"
#include "sfg_trajectory_planner/core/scene.hpp"
#include "sfg_utils/cpp_utils.hpp"

namespace sfg_trajectory_planner::core
{
    SceneObject::SceneObject(SceneObject::ConstructionKey, Scene &scene, uuids::uuid uuid)
        : m_scene(scene),
          m_name("New Object"),
          m_uuid(std::move(uuid)),
          m_transform(glm::mat4(1.0f)),
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

    void SceneObject::serialize(serialization::AbstractSerializer *serializer) const
    {
        serializer->serialize("name", m_name);
        serializer->serialize("type", get_type());
        serializer->serialize("uuid", uuids::to_string(m_uuid));

        serializer->push_group("transform");
        m_transform.serialize(serializer);
        serializer->pop_group();
    }

    void SceneObject::deserialize(serialization::AbstractSerializer *serializer)
    {
        // We only need to deserialize the name because type and UUID are used
        // by the scene class to instantiate the correct class with the appropriate
        // UUID.
        m_name = std::get<std::string>(serializer->deserialize("name"));

        serializer->push_group("transform");
        m_transform.deserialize(serializer);
        serializer->pop_group();
    }

    void SceneObject::render_object(gfx::Renderer &) {}

    void SceneObject::render_inspector()
    {
        sfg_imgui_vendor::PushIdGuard guard(this);
        ImGui::InputText("Name", &m_name);

        auto type = get_type();
        ImGui::BeginDisabled();
        ImGui::InputText("Type", &type);

        auto uuid = uuids::to_string(get_uuid());
        ImGui::InputText("UUID", &uuid);
        ImGui::EndDisabled();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            m_transform.render_inspector();
        }

        if (ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen))
        {
            render_inspector_internal();
        }
    }

    std::string SceneObject::get_name() const
    {
        return m_name;
    }

    std::string SceneObject::get_type() const
    {
        return sfg_utils::cpp_utils::get_type(this);
    }

    uuids::uuid SceneObject::get_uuid() const
    {
        return m_uuid;
    }

    Transform &SceneObject::get_transform()
    {
        return m_transform;
    }

    glm::mat4 SceneObject::get_object_to_world_matrix() const
    {
        if (m_parent)
        {
            return m_parent->get_object_to_world_matrix() * m_transform.get_matrix();
        }
        return m_transform.get_matrix();
    }

    glm::mat4 SceneObject::get_world_to_object_matrix() const
    {
        return glm::inverse(get_object_to_world_matrix());
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

    void SceneObject::set_parent(SceneObject *parent)
    {
        // If the new parent is the same as the current parent, do nothing.
        if (parent == m_parent)
        {
            return;
        }

        // We need to check if the new parent is a descendant of this object to avoid creating a cycle in the scene graph.
        for (auto ancestor = parent; ancestor != nullptr; ancestor = ancestor->get_parent())
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

    void SceneObject::render_inspector_internal() {}
}