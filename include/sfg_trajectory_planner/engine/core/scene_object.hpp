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

        SceneObject(ConstructionKey key, const Scene &scene, uuids::uuid uuid);
        SceneObject(const SceneObject &) = delete;
        SceneObject &operator=(const SceneObject &) = delete;
        SceneObject(SceneObject &&) = delete;
        SceneObject &operator=(SceneObject &&) = delete;
        virtual ~SceneObject();

        void serialize(serialization::AbstractSerializer *serializer) const override;
        void deserialize(serialization::AbstractSerializer *serializer) override;

        virtual void render_object(gfx::Renderer &renderer);

        std::string get_name() const;
        std::string get_type() const;
        uuids::uuid get_uuid() const;
        bool is_visible() const;
        Transform &get_transform_ls();
        glm::mat4 get_ls_to_ws_matrix() const;
        glm::mat4 get_ws_to_ls_matrix() const;
        SceneObject *get_parent() const;
        const std::vector<SceneObject *> &get_children() const;
        bool is_ancestor_of(const SceneObject *object) const;
        bool is_descendant_of(const SceneObject *object) const;

        void set_name(std::string name);
        void set_visible(bool visible);
        void set_parent(SceneObject *parent);

    protected:
        const Scene &m_scene;
        std::string m_name;
        const uuids::uuid m_uuid;
        bool m_visible;
        Transform m_transform_ls;
        SceneObject *m_parent;
        std::vector<SceneObject *> m_children;
    };
}

#include "sfg_trajectory_planner/engine/core/scene_object.tpp"