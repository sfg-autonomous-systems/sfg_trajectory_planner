#pragma once

#include <filesystem>
#include <glad/glad.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Shader
    {
    public:
        Shader(std::filesystem::path vertex_source_filepath, std::filesystem::path fragment_source_filepath);
        Shader(const char *vertex_source, const char *fragment_source);
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&) noexcept;
        Shader &operator=(Shader &&) noexcept;
        ~Shader();

        void bind() const;
        void unbind() const;
        GLuint get_id() const;

    private:
        GLuint m_id = 0;
    };
}