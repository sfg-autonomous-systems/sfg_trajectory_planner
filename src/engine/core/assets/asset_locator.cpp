#include "sfg_trajectory_planner/engine/core/assets/asset_locator.hpp"

#include <fstream>
#include <sstream>
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
        auto material_node = YAML::LoadFile(resolve_arg(filepath).string());
        auto vertex_shader_path = material_node["vertex_shader"].as<std::string>();
        auto fragment_shader_path = material_node["fragment_shader"].as<std::string>();
        auto shader = load_asset<gfx::Shader>(vertex_shader_path.c_str(), fragment_shader_path.c_str());
        auto material = std::make_shared<gfx::Material>(shader);

        if (material_node["uniforms"])
        {
            for (const auto &uniform_node : material_node["uniforms"])
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

    std::shared_ptr<gfx::Shader> AssetLocator::load_shader(const std::filesystem::path &vertex_source_filepath, const std::filesystem::path &fragment_source_filepath) const
    {
        std::string vertex_source;
        std::string fragment_source;

        try
        {
            using Iterator = std::istreambuf_iterator<char>;

            std::ifstream vertex_source_file(resolve_arg(vertex_source_filepath));
            std::ifstream fragment_source_file(resolve_arg(fragment_source_filepath));
            vertex_source = std::string((Iterator(vertex_source_file)), Iterator());
            fragment_source = std::string((Iterator(fragment_source_file)), Iterator());
        }
        catch (const std::exception &exception)
        {
            throw std::runtime_error(std::string("Failed to read shader source files: ") + exception.what());
        }
        auto shader = std::make_shared<gfx::Shader>(vertex_source.c_str(), fragment_source.c_str());
        return shader;
    }

    std::shared_ptr<gfx::TextFont> AssetLocator::load_text_font(const std::filesystem::path &filepath, float height) const
    {
        std::ifstream file(resolve_arg(filepath), std::ios::binary | std::ios::ate);

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
        auto text_font = std::make_shared<gfx::TextFont>(data.data(), height);
        return text_font;
    }
}