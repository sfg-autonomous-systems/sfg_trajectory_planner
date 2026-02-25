#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>
// ImGuizmo needs to be included after imgui.
#include <imgui/ImGuizmo.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/core/scene.hpp"
#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"
#include "sfg_utils/cpp_utils.hpp"

namespace sfg_trajectory_planner::engine::core
{
    SceneObject::SceneObject(SceneObject::ConstructionKey, const Scene &scene, uuids::uuid uuid)
        : m_scene(scene),
          m_name("New Object"),
          m_uuid(std::move(uuid)),
          m_visible(true),
          m_transform_ls(glm::mat4(1.0f)),
          m_parent(nullptr)
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
        serializer->serialize("visible", m_visible);

        serializer->begin_group("transform");
        m_transform_ls.serialize(serializer);
        serializer->end_group();
    }

    void SceneObject::deserialize(serialization::AbstractSerializer *serializer)
    {
        // We only need to deserialize the name and visiblity because type and UUID are used
        // by the scene class to instantiate the correct class with the appropriate UUID.
        m_name = serializer->deserialize<std::string>("name");
        m_visible = serializer->deserialize<bool>("visible");

        serializer->begin_group("transform");
        m_transform_ls.deserialize(serializer);
        serializer->end_group();
    }

    void SceneObject::render_object(gfx::Renderer &)
    {
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

    bool SceneObject::is_visible() const
    {
        return m_visible;
    }

    Transform &SceneObject::get_transform_ls()
    {
        return m_transform_ls;
    }

    glm::mat4 SceneObject::get_ls_to_ws_matrix() const
    {
        if (m_parent)
        {
            return m_parent->get_ls_to_ws_matrix() * m_transform_ls.get_matrix();
        }
        return m_transform_ls.get_matrix();
    }

    glm::mat4 SceneObject::get_ws_to_ls_matrix() const
    {
        return glm::inverse(get_ls_to_ws_matrix());
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

    bool SceneObject::is_ancestor_of(const SceneObject *object) const
    {
        if (object == nullptr)
        {
            return false;
        }

        for (auto ancestor = object->get_parent(); ancestor != nullptr; ancestor = ancestor->get_parent())
        {
            if (ancestor == this)
            {
                return true;
            }
        }
        return false;
    }

    bool SceneObject::is_descendant_of(const SceneObject *object) const
    {
        return object ? object->is_ancestor_of(this) : false;
    }

    void SceneObject::set_name(std::string name)
    {
        m_name = std::move(name);
    }

    void SceneObject::set_visible(bool visible)
    {
        m_visible = visible;
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
}