#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sfg_trajectory_planner::engine::core::gfx
{
    Shader::Shader(const std::string &shader_source)
    {
        // Read the shader source line for line. The shader contents should be of the form:
        // ... shared code for all stages ...
        // #pragma stage vertex
        // ... vertex stage source code ...
        // #pragma stage fragment
        // ... fragment stage source code ...
        std::array<std::string, s_stage_count> stage_sources;
        std::array<bool, s_stage_count> stage_source_defined = {false};
        // The index of the current shader stage source being read. An index of s_stage_count indicates shared source code that should be included in all stages.
        auto current_stage_index = s_stage_count;
        std::istringstream shader_source_stream(shader_source);
        std::string line;

        while (std::getline(shader_source_stream, line))
        {
            auto stage = std::find_if(
                std::begin(s_stages),
                std::end(s_stages),
                [&](const auto &stage)
                {
                    return line.find(stage.m_keyword) == 0;
                });

            // Switch to the correct stage source index based on the encountered stage keyword.
            if (stage != std::end(s_stages))
            {
                current_stage_index = std::distance(std::begin(s_stages), stage);

                if (stage_source_defined[current_stage_index])
                {
                    throw std::runtime_error(std::string(stage->m_name) + " shader stage is defined multiple times in shader.");
                }
                stage_source_defined[current_stage_index] = true;
            }

            auto is_shared_line = current_stage_index == s_stage_count;

            for (size_t stage_index = 0; stage_index < s_stage_count; stage_index++)
            {
                auto is_current_stage_line = stage_index == current_stage_index;

                if (is_shared_line || is_current_stage_line)
                {
                    stage_sources[stage_index] += line;
                }
                stage_sources[stage_index] += "\n";
            }
        }

        auto success = 0;
        char info_log[512];
        m_id = glCreateProgram();

        for (size_t stage_index = 0; stage_index < s_stage_count; stage_index++)
        {
            if (!stage_source_defined[stage_index])
            {
                continue;
            }
            const auto &stage = s_stages[stage_index];
            const auto stage_source = stage_sources[stage_index].c_str();

            auto stage_id = glCreateShader(stage.m_shader_type);
            glShaderSource(stage_id, 1, &stage_source, nullptr);
            glCompileShader(stage_id);
            glGetShaderiv(stage_id, GL_COMPILE_STATUS, &success);

            if (!success)
            {
                glGetShaderInfoLog(stage_id, 512, nullptr, info_log);
                throw std::runtime_error(std::string(stage.m_name) + " shader stage compilation failed: " + info_log);
            }
            glAttachShader(m_id, stage_id);
            glDeleteShader(stage_id);
        }

        glLinkProgram(m_id);
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, info_log);
            throw std::runtime_error(std::string("Shader linking failed: ") + info_log);
        }
    }

    Shader::Shader(Shader &&other) noexcept
        : m_id(other.m_id),
          m_uniform_location_cache(std::move(other.m_uniform_location_cache))
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
            m_uniform_location_cache = std::move(other.m_uniform_location_cache);
            other.m_id = 0;
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

    GLuint Shader::get_id() const
    {
        return m_id;
    }

    GLint Shader::get_uniform_location(const std::string &name) const
    {
        if (auto iterator = m_uniform_location_cache.find(name); iterator != m_uniform_location_cache.end())
        {
            return iterator->second;
        }
        auto location = glGetUniformLocation(m_id, name.c_str());
        m_uniform_location_cache[name] = location;
        return location;
    }
}