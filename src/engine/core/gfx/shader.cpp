#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>

namespace sfg_trajectory_planner::engine::core::gfx
{
    Shader::Shader(const char *vertex_source, const char *fragment_source)
    {
        auto success = 0;
        char info_log[512];

        auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Vertex shader compilation failed: ") + info_log);
        }

        auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Fragment shader compilation failed: ") + info_log);
        }

        m_id = glCreateProgram();
        glAttachShader(m_id, vertex_shader);
        glAttachShader(m_id, fragment_shader);
        glLinkProgram(m_id);
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Shader program linking failed: ") + info_log);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        m_ls_to_ws_uniform_location = glGetUniformLocation(m_id, "u_LsToWsMatrix");
        m_ws_to_cs_uniform_location = glGetUniformLocation(m_id, "u_WsToCsMatrix");
        m_ls_to_cs_uniform_location = glGetUniformLocation(m_id, "u_LsToCsMatrix");
    }

    Shader::Shader(Shader &&other) noexcept
        : m_id(other.m_id),
          m_ls_to_ws_uniform_location(other.m_ls_to_ws_uniform_location),
          m_ws_to_cs_uniform_location(other.m_ws_to_cs_uniform_location),
          m_ls_to_cs_uniform_location(other.m_ls_to_cs_uniform_location)
    {
        other.m_id = 0;
        other.m_ls_to_ws_uniform_location = 0;
        other.m_ws_to_cs_uniform_location = 0;
        other.m_ls_to_cs_uniform_location = 0;
    }

    Shader &Shader::operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            if (m_id)
            {
                glDeleteProgram(m_id);
            }

            m_id = other.m_id;
            m_ls_to_ws_uniform_location = other.m_ls_to_ws_uniform_location;
            m_ws_to_cs_uniform_location = other.m_ws_to_cs_uniform_location;
            m_ls_to_cs_uniform_location = other.m_ls_to_cs_uniform_location;

            other.m_id = 0;
            other.m_ls_to_ws_uniform_location = 0;
            other.m_ws_to_cs_uniform_location = 0;
            other.m_ls_to_cs_uniform_location = 0;
        }
        return *this;
    }

    Shader::~Shader()
    {
        if (m_id)
        {
            glDeleteProgram(m_id);
        }
    }

    void Shader::bind() const
    {
        glUseProgram(m_id);
    }

    void Shader::unbind() const
    {
        glUseProgram(0);
    }

    void Shader::set_ls_to_ws_matrix(const glm::mat4 &matrix) const
    {
        glUniformMatrix4fv(m_ls_to_ws_uniform_location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::set_ws_to_cs_matrix(const glm::mat4 &matrix) const
    {
        glUniformMatrix4fv(m_ws_to_cs_uniform_location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::set_ls_to_cs_matrix(const glm::mat4 &matrix) const
    {
        glUniformMatrix4fv(m_ls_to_cs_uniform_location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    GLuint Shader::get_id() const
    {
        return m_id;
    }

    GLint Shader::get_uniform_location(const std::string &name) const
    {
        if (m_uniform_location_cache.find(name) != m_uniform_location_cache.end())
        {
            return m_uniform_location_cache[name];
        }
        auto location = glGetUniformLocation(m_id, name.c_str());
        m_uniform_location_cache[name] = location;
        return location;
    }
}