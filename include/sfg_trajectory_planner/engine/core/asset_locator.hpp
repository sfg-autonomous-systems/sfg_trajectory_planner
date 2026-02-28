#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Material;
    class Shader;
    class TextFont;
}

namespace sfg_trajectory_planner::engine::core
{
    class AssetLocator
    {
    public:
        explicit AssetLocator(std::filesystem::path asset_directory);

        template <typename AssetType, typename... Args>
        std::shared_ptr<AssetType> load_asset(Args &&...args) const;

    private:
        std::shared_ptr<gfx::Material> load_material(const std::filesystem::path &filepath) const;
        std::shared_ptr<gfx::Shader> load_shader(const std::filesystem::path &vertex_source_filepath, const std::filesystem::path &fragment_source_filepath) const;
        std::shared_ptr<gfx::TextFont> load_text_font(const std::filesystem::path &filepath, float height) const;

        template <typename AssetType, typename... Args>
        std::string get_cache_key(const Args &...args) const;

        template <typename AssetType, typename... Args>
        bool try_load_from_cache(std::shared_ptr<AssetType> &out_asset, std::string &out_key, const Args &...args) const;

        template <typename ArgType>
        decltype(auto) resolve_arg(ArgType &&arg) const;

        std::filesystem::path m_asset_directory;
        mutable std::unordered_map<std::string, std::weak_ptr<void>> m_asset_cache;
    };
}

#include "sfg_trajectory_planner/engine/core/asset_locator.tpp"