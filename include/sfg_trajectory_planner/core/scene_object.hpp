#pragma once

#include <glm/glm.hpp>
#include <string>
#include <uuid.h>
#include <vector>

#include "sfg_trajectory_planner/core/transform.hpp"

namespace sfg_trajectory_planner::core::gfx
{
    class Renderer;
}

namespace sfg_trajectory_planner::core::serialization
{
    class AbstractSerializer;
}

namespace sfg_trajectory_planner::core
{
    class Scene;

    class SceneObject
    {
        friend class Scene;

    public:
        class ConstructionKey
        {
            friend class Scene;

        private:
            ConstructionKey() = default;
        };

        template <typename ObjectType>
        static std::string get_type();

        SceneObject(ConstructionKey key, Scene &scene, uuids::uuid uuid);
        virtual ~SceneObject();
        SceneObject(const SceneObject &) = delete;
        SceneObject &operator=(const SceneObject &) = delete;
        SceneObject(SceneObject &&) = delete;
        SceneObject &operator=(SceneObject &&) = delete;

        virtual void serialize(serialization::AbstractSerializer *serializer) const;
        virtual void deserialize(serialization::AbstractSerializer *serializer);

        virtual void render_object(gfx::Renderer &renderer);
        void render_inspector();

        std::string get_name() const;
        std::string get_type() const;
        uuids::uuid get_uuid() const;

        Transform &get_transform();
        glm::mat4 get_object_to_world_matrix() const;
        glm::mat4 get_world_to_object_matrix() const;
        SceneObject *get_parent() const;
        const std::vector<SceneObject *> &get_children() const;
        bool is_visible() const;

        void set_name(std::string name);
        void set_parent(SceneObject *parent);
        void set_visible(bool visible);

    protected:
        virtual void render_inspector_internal();

        Scene &m_scene;
        std::string m_name;
        const uuids::uuid m_uuid;
        Transform m_transform;
        SceneObject *m_parent;
        std::vector<SceneObject *> m_children;
        bool m_visible;
    };
}

#include "sfg_trajectory_planner/core/scene_object.tpp"