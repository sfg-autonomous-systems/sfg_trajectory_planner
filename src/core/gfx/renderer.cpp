#include "sfg_trajectory_planner/core/gfx/renderer.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>
#include <SDL.h>

static constexpr auto s_max_line_vertices = 100000;

static constexpr auto s_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

uniform mat4 u_ViewProjection;

out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static constexpr auto s_fragment_shader_source = R"(
#version 330 core
in vec4 v_Color;
out vec4 f_Color;

void main() {
    f_Color = v_Color;
}
)";

namespace sfg_trajectory_planner::core::gfx
{
    Renderer::Renderer()
    {
        m_line_vertices.reserve(s_max_line_vertices);
    }

    Renderer::~Renderer()
    {
        if (!SDL_WasInit(SDL_INIT_VIDEO))
        {
            return;
        }

        if (!m_initialized)
        {
            return;
        }

        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
        }

        if (m_vbo)
        {
            glDeleteBuffers(1, &m_vbo);
        }

        if (m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
        }

        if (m_color_texture)
        {
            glDeleteTextures(1, &m_color_texture);
        }

        if (m_depth_rbo)
        {
            glDeleteRenderbuffers(1, &m_depth_rbo);
        }

        if (m_shader_program)
        {
            glDeleteProgram(m_shader_program);
        }
    }

    void Renderer::set_matrices(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::ivec4 viewport)
    {
        m_view_matrix = view_matrix;
        m_projection_matrix = projection_matrix;

        if ((viewport.z != m_viewport.z || viewport.w != m_viewport.w) && viewport.z > 0 && viewport.w > 0)
        {
            resize_fbo(viewport.z, viewport.w);
            m_viewport = viewport;
        }
    }

    void Renderer::add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color)
    {
        add_line(glm::mat4(1.0f), start, end, color);
    }

    void Renderer::add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color)
    {
        if (m_line_vertices.size() + 2 > s_max_line_vertices)
        {
            throw std::runtime_error("Exceeded maximum line vertex count");
        }
        m_line_vertices.push_back({model_matrix * glm::vec4(start, 1.0f), color});
        m_line_vertices.push_back({model_matrix * glm::vec4(end, 1.0f), color});
    }

    bool Renderer::add_gizmo(glm::mat4 &model_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id)
    {
        ImGuizmo::PushID(id);
        auto manipulated = ImGuizmo::Manipulate(glm::value_ptr(m_view_matrix), glm::value_ptr(m_projection_matrix), operation, mode, glm::value_ptr(model_matrix));
        ImGuizmo::PopID();

        return manipulated;
    }

    GLuint Renderer::render()
    {
        if (!m_initialized)
        {
            initialize_lazily();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_viewport.z, m_viewport.w);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if (!m_line_vertices.empty())
        {
            // Upload data.
            auto vertex_count = m_line_vertices.size();

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), m_line_vertices.data());

            // Issue draw call.
            glUseProgram(m_shader_program);
            glm::mat4 view_projection_matrix = m_projection_matrix * m_view_matrix;
            glUniformMatrix4fv(glGetUniformLocation(m_shader_program, "u_ViewProjection"), 1, GL_FALSE, &view_projection_matrix[0][0]);

            glBindVertexArray(m_vao);
            glLineWidth(1.0f);
            glDrawArrays(GL_LINES, 0, (GLsizei)vertex_count);
            glBindVertexArray(0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_line_vertices.clear();

        return m_color_texture;
    }

    // We initialize lazily because initialization relies on a valid OpenGL context that might not yet me available
    // at the time of construction because the OpenGL context is initialized in the GUI thread, while the Renderer is constructed in the main thread.
    void Renderer::initialize_lazily()
    {
        if (m_initialized)
        {
            return;
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, s_max_line_vertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, m_position)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, m_color)));
        glBindVertexArray(0);

        auto success = 0;
        char info_log[512];

        auto m_vertex_shader_program = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vertex_shader_program, 1, &s_vertex_shader_source, nullptr);
        glCompileShader(m_vertex_shader_program);
        glGetShaderiv(m_vertex_shader_program, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(m_vertex_shader_program, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Vertex shader compilation failed: ") + info_log);
        }

        auto m_fragment_shader_program = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_fragment_shader_program, 1, &s_fragment_shader_source, nullptr);
        glCompileShader(m_fragment_shader_program);
        glGetShaderiv(m_fragment_shader_program, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(m_fragment_shader_program, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Fragment shader compilation failed: ") + info_log);
        }

        m_shader_program = glCreateProgram();
        glAttachShader(m_shader_program, m_vertex_shader_program);
        glAttachShader(m_shader_program, m_fragment_shader_program);
        glLinkProgram(m_shader_program);
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(m_shader_program, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Shader program linking failed: ") + info_log);
        }

        glDeleteShader(m_vertex_shader_program);
        glDeleteShader(m_fragment_shader_program);
        m_initialized = true;
    }

    void Renderer::resize_fbo(int width, int height)
    {
        if (m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
            glDeleteTextures(1, &m_color_texture);
            glDeleteRenderbuffers(1, &m_depth_rbo);
        }

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glGenTextures(1, &m_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

        glGenRenderbuffers(1, &m_depth_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}