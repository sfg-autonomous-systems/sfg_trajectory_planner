#include "sfg_trajectory_planner/engine/core/assets/asset_locator.hpp"

#include <fstream>
#include <sstream>
#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include <stb_include.h>
#include <type_traits>
#include <yaml-cpp/yaml.h>

#include "sfg_trajectory_planner/engine/core/gfx/material.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/text_font.hpp"

namespace sfg_trajectory_planner::engine::core::assets
{
    AssetLocator::AssetLocator(std::filesystem::path asset_directory)
        : m_asset_directory(std::move(asset_directory))
    {
    }

    std::shared_ptr<gfx::Material> AssetLocator::load_material(const std::filesystem::path &filepath) const
    {
        auto material_file = YAML::LoadFile(filepath.string());
        auto shader_filepath = std::filesystem::path(material_file["shader"].as<std::string>());
        auto shader = load_asset<gfx::Shader>(shader_filepath);
        auto material = std::make_shared<gfx::Material>(shader);

        if (material_file["uniforms"])
        {
            for (const auto &uniform_node : material_file["uniforms"])
            {
                auto uniform_name = uniform_node.first.as<std::string>();
                const auto &value_node = uniform_node.second;

                if (value_node.IsScalar())
                {
                    if (value_node.Tag() == "!!int")
                    {
                        material->set_uniform(uniform_name, value_node.as<int>());
                    }
                    else if (value_node.Tag() == "!!float")
                    {
                        material->set_uniform(uniform_name, value_node.as<float>());
                    }
                }
                else if (value_node.IsSequence())
                {
                    auto values = value_node.as<std::vector<float>>();

                    if (value_node.size() == 2)
                    {
                        material->set_uniform(uniform_name, glm::vec2(values[0], values[1]));
                    }
                    else if (value_node.size() == 3)
                    {
                        material->set_uniform(uniform_name, glm::vec3(values[0], values[1], values[2]));
                    }
                    else if (value_node.size() == 4)
                    {
                        material->set_uniform(uniform_name, glm::vec4(values[0], values[1], values[2], values[3]));
                    }
                    else if (value_node.size() == 16)
                    {
                        material->set_uniform(
                            uniform_name,
                            glm::mat4(
                                values[0], values[1], values[2], values[3],
                                values[4], values[5], values[6], values[7],
                                values[8], values[9], values[10], values[11],
                                values[12], values[13], values[14], values[15]));
                    }
                }
            }
        }
        return material;
    }

    std::shared_ptr<gfx::Shader> AssetLocator::load_shader(const std::filesystem::path &filepath) const
    {
        std::string shader_source;

        try
        {
            using Iterator = std::istreambuf_iterator<char>;
            std::ifstream shader_source_file(filepath);
            shader_source = std::string((Iterator(shader_source_file)), Iterator());
        }
        catch (const std::exception &exception)
        {
            throw std::runtime_error("Failed to read shader source file '" + filepath.string() + "': " + exception.what());
        }

        char error[256];
        std::string filename = filepath.filename().string();

        char *shader_source_processed = stb_include_string(
            shader_source.data(),
            nullptr,
            const_cast<char *>(m_asset_directory.c_str()),
            const_cast<char *>(filename.c_str()),
            error);

        if (shader_source_processed == nullptr)
        {
            throw std::runtime_error("Failed to preprocess shader source file '" + filepath.string() + "': " + error);
        }

        shader_source = shader_source_processed;
        free(shader_source_processed);
        return std::make_shared<gfx::Shader>(shader_source);
    }

    std::shared_ptr<gfx::TextFont> AssetLocator::load_text_font(const std::filesystem::path &filepath, float height) const
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open font file: " + filepath.string());
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<std::uint8_t> data(size);

        if (!file.read(reinterpret_cast<char *>(data.data()), size))
        {
            throw std::runtime_error("Failed to read font file: " + filepath.string());
        }
        return std::make_shared<gfx::TextFont>(data.data(), height);
    }
}