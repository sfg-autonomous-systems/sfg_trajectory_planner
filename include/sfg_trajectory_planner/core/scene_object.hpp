#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/core/transform.hpp"

#define STRINGIFY(x) #x

namespace sfg_trajectory_planner::core
{
    class Scene;

    class SceneObjectKey
    {
        friend class Scene;

    private:
        SceneObjectKey() = default;
    };

    class SceneObject
    {
        friend class Scene;

    public:
        SceneObject(SceneObjectKey key, Scene &scene, std::string type = STRINGIFY(sfg_trajectory_planner::core::SceneObject));
        virtual ~SceneObject();
        virtual void render_object(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, const glm::vec4 &viewport);
        virtual void render_inspector();

        std::string get_name() const;
        std::string get_type() const;
        glm::mat4 get_local_transform() const;
        glm::mat4 get_global_transform() const;
        SceneObject *get_parent() const;
        const std::vector<SceneObject *> &get_children() const;
        bool is_visible() const;

        void set_name(std::string name);
        void set_local_transform(glm::mat4 transform);
        void set_parent(SceneObject *parent);
        void set_visible(bool visible);

    protected:
        Scene &m_scene;

        std::string m_name;
        const std::string m_type;
        Transform m_local_transform;
        SceneObject *m_parent;
        std::vector<SceneObject *> m_children;
        bool m_visible;
    };
}

#undef STRINGIFY