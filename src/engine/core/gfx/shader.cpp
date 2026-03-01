#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sfg_trajectory_planner::engine::core::gfx
{
    Shader::Shader(const char *shader_source)
    {

        // Read the shader source line for line. The shader contents should be of the form:
        // ... shared shader code for all stages ...
        // #pragma stage vertex
        // ... vertex shader source code ...
        // #pragma stage fragment
        // ... fragment shader source code ...

        // Resolve #include directives in the shader source using stb_include.
        std::string shared_source;
        std::string vertex_source;
        std::string fragment_source;

        auto current_stage = ShaderStage::None;
        std::istringstream shader_source_stream(shader_source);
        std::string line;

        while (std::getline(shader_source_stream, line))
        {
            if (line.find("#pragma stage vertex") != std::string::npos)
            {
                current_stage = ShaderStage::Vertex;
            }
            else if (line.find("#pragma stage fragment") != std::string::npos)
            {
                current_stage = ShaderStage::Fragment;
            }
            else
            {
                switch (current_stage)
                {
                    case ShaderStage::None:
                        shared_source += line + "\n";
                        break;
                    case ShaderStage::Vertex:
                        vertex_source += line + "\n";
                        break;
                    case ShaderStage::Fragment:
                        fragment_source += line + "\n";
                        break;
                    default:
                        break;
                }
            }
        }

        auto success = 0;
        char info_log[512];

        if (vertex_source.empty())
        {
            throw std::runtime_error("Shader source must contain vertex shader stage.");
        }

        vertex_source = shared_source + vertex_source;
        auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        auto vertex_source_c_str = vertex_source.c_str();
        glShaderSource(vertex_shader, 1, &vertex_source_c_str, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Vertex shader compilation failed: ") + info_log);
        }

        if (fragment_source.empty())
        {
            throw std::runtime_error("Shader source must contain fragment shader stage.");
        }

        fragment_source = shared_source + fragment_source;
        auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        auto fragment_source_c_str = fragment_source.c_str();
        glShaderSource(fragment_shader, 1, &fragment_source_c_str, nullptr);
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