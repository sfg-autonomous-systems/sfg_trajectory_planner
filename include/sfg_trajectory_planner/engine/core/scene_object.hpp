#pragma once

#include <glm/glm.hpp>
#include <string>
#include <uuid.h>
#include <vector>

#include "sfg_trajectory_planner/engine/core/serialization/serializable.hpp"
#include "sfg_trajectory_planner/engine/core/transform.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Renderer;
}

namespace sfg_trajectory_planner::engine::core
{

    class SceneObject : public serialization::ISerializable
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

        void serialize(serialization::AbstractSerializer *serializer) const override;
        void deserialize(serialization::AbstractSerializer *serializer) override;

        virtual void render_object(gfx::Renderer &renderer);

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
        Scene &m_scene;
        std::string m_name;
        const uuids::uuid m_uuid;
        Transform m_transform;
        SceneObject *m_parent;
        std::vector<SceneObject *> m_children;
        bool m_visible;
    };
}

#include "sfg_trajectory_planner/engine/core/scene_object.tpp"