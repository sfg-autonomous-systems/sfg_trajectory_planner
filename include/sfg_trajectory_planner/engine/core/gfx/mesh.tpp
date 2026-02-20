#include "sfg_trajectory_planner/engine/core/gfx/mesh.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    template <typename VertexType>
    Mesh<VertexType>::Mesh(Topology topology)
        : m_topology(topology)
    {
        // Generate vertex array object and vertex buffer object.
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        // Generate element buffer object.
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        // Initialize vertex buffer with initial size, but no data.
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vbo_size * sizeof(VertexType), nullptr, GL_STATIC_DRAW);

        // Initialize element buffer with initial size, but no data.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_ebo_size * sizeof(std::uint32_t), nullptr, GL_STATIC_DRAW);

        // The concrete vertex type is expected to set up the vertex attributes itself, since different vertex types might have different vertex attributes.
        VertexType::set_vertex_attributes();

        glBindVertexArray(0);
    }

    template <typename VertexType>
    Mesh<VertexType>::~Mesh()
    {
        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
        }

        if (m_vbo)
        {
            glDeleteBuffers(1, &m_vbo);
        }

        if (m_ebo)
        {
            glDeleteBuffers(1, &m_ebo);
        }
    }

    template <typename VertexType>
    Mesh<VertexType>::Mesh(Mesh &&other) noexcept
        : m_vertices(std::move(other.m_vertices)),
          m_indices(std::move(other.m_indices)),
          m_vao(other.m_vao),
          m_vbo(other.m_vbo),
          m_ebo(other.m_ebo),
          m_vbo_size(other.m_vbo_size),
          m_ebo_size(other.m_ebo_size),
          m_dirty(other.m_dirty)
    {
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
    }

    template <typename VertexType>
    Mesh<VertexType> &Mesh<VertexType>::operator=(Mesh &&other) noexcept
    {
        if (this != &other)
        {
            if (m_vao)
            {
                glDeleteVertexArrays(1, &m_vao);
            }

            if (m_vbo)
            {
                glDeleteBuffers(1, &m_vbo);
            }

            if (m_ebo)
            {
                glDeleteBuffers(1, &m_ebo);
            }

            m_vertices = std::move(other.m_vertices);
            m_indices = std::move(other.m_indices);
            m_vao = other.m_vao;
            m_vbo = other.m_vbo;
            m_ebo = other.m_ebo;
            m_vbo_size = other.m_vbo_size;
            m_ebo_size = other.m_ebo_size;
            m_dirty = other.m_dirty;

            other.m_vao = 0;
            other.m_vbo = 0;
            other.m_ebo = 0;
        }
        return *this;
    }

    template <typename VertexType>
    void Mesh<VertexType>::render()
    {
        bind();

        if (m_indices.empty())
        {
            glDrawArrays(m_topology == Topology::Triangles ? GL_TRIANGLES : GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));
        }
        else
        {
            glDrawElements(m_topology == Topology::Triangles ? GL_TRIANGLES : GL_LINES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, static_cast<void *>(0));
        }
        unbind();
    }

    template <typename VertexType>
    void Mesh<VertexType>::clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_dirty = true;
    }

    template <typename VertexType>
    typename Mesh<VertexType>::Topology Mesh<VertexType>::get_topology() const
    {
        return m_topology;
    }

    template <typename VertexType>
    bool Mesh<VertexType>::empty() const
    {
        return m_vertices.empty();
    }

    template <typename VertexType>
    size_t Mesh<VertexType>::vertex_count() const
    {
        return m_vertices.size();
    }

    template <typename VertexType>
    void Mesh<VertexType>::set_vertices(std::initializer_list<VertexType> vertices)
    {
        m_vertices = vertices;
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::set_vertices(std::vector<VertexType> vertices)
    {
        m_vertices = std::move(vertices);
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::add_vertices(std::initializer_list<VertexType> vertices)
    {
        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::add_vertices(const std::vector<VertexType> &vertices)
    {
        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::set_indices(std::initializer_list<std::uint32_t> indices)
    {
        m_indices = indices;
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::set_indices(std::vector<std::uint32_t> indices)
    {
        m_indices = std::move(indices);
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::add_indices(std::initializer_list<std::uint32_t> indices)
    {
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::add_indices(const std::vector<std::uint32_t> &indices)
    {
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());
        m_dirty = true;
    }

    template <typename VertexType>
    void Mesh<VertexType>::bind()
    {
        if (m_dirty)
        {
            upload();
            m_dirty = false;
        }
        glBindVertexArray(m_vao);
    }

    template <typename VertexType>
    void Mesh<VertexType>::unbind()
    {
        glBindVertexArray(0);
    }

    template <typename VertexType>
    void Mesh<VertexType>::upload()
    {
        if (m_vertices.size() > m_vbo_size)
        {
            m_vbo_size = m_vertices.size() * 2;
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, m_vbo_size * sizeof(VertexType), m_vertices.data(), GL_DYNAMIC_DRAW);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(VertexType), m_vertices.data());
        }

        if (m_indices.size() > m_ebo_size)
        {
            m_ebo_size = m_indices.size() * 2;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_ebo_size * sizeof(std::uint32_t), m_indices.data(), GL_DYNAMIC_DRAW);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_indices.size() * sizeof(std::uint32_t), m_indices.data());
        }
    }
}