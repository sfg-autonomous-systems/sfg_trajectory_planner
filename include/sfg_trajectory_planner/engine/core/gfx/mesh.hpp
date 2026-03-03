#pragma once

#include "sfg_trajectory_planner/engine/core/gfx/renderable.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    template <typename VertexType>
    class Mesh : public IRenderable
    {
    public:
        Mesh(GLenum topology = GL_TRIANGLES);
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;
        Mesh(Mesh &&) noexcept;
        Mesh &operator=(Mesh &&) noexcept;
        ~Mesh();

        void render() override;
        void clear();

        GLenum get_topology() const;
        bool empty() const;
        size_t vertex_count() const;

        void set_vertices(std::vector<VertexType> vertices);
        void add_vertices(const std::vector<VertexType> &vertices);

        void set_indices(std::vector<std::uint32_t> indices);
        void add_indices(const std::vector<std::uint32_t> &indices);

    private:
        void bind() const;
        void unbind() const;
        void upload() const;

        const GLenum m_topology;
        std::vector<VertexType> m_vertices;
        std::vector<std::uint32_t> m_indices;

        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;

        mutable std::uint32_t m_vbo_size = 100;
        mutable std::uint32_t m_ebo_size = 100;
        mutable bool m_dirty = true;
    };
}

#include "sfg_trajectory_planner/engine/core/gfx/mesh.tpp"