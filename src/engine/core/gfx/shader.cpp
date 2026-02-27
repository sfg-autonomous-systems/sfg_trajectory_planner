#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace sfg_trajectory_planner::engine::core::gfx
{
    Shader::Shader(std::filesystem::path vertex_source_filepath, std::filesystem::path fragment_source_filepath)
    {
        std::string vertex_source;
        std::string fragment_source;

        try
        {
            using Iterator = std::istreambuf_iterator<char>;

            std::ifstream vertex_source_file(vertex_source_filepath);
            std::ifstream fragment_source_file(fragment_source_filepath);
            vertex_source = std::string((Iterator(vertex_source_file)), Iterator());
            fragment_source = std::string((Iterator(fragment_source_file)), Iterator());
        }
        catch (const std::exception &exception)
        {
            throw std::runtime_error(std::string("Failed to read shader source files: ") + exception.what());
        }
        *this = Shader(vertex_source.c_str(), fragment_source.c_str());
    }

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
    }

    Shader::~Shader()
    {
        if (m_id)
        {
            glDeleteProgram(m_id);
        }
    }

    Shader::Shader(Shader &&other) noexcept : m_id(other.m_id)
    {
        other.m_id = 0;
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
            other.m_id = 0;
        }
        return *this;
    }

    void Shader::bind() const
    {
        glUseProgram(m_id);
    }

    void Shader::unbind() const
    {
        glUseProgram(0);
    }

    GLuint Shader::get_id() const
    {
        return m_id;
    }
}