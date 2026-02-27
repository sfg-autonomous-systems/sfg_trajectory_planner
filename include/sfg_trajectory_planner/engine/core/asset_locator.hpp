#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace sfg_trajectory_planner::engine::core
{
    class AssetLocator
    {
    public:
        explicit AssetLocator(std::filesystem::path asset_directory);

        template <typename AssetType, typename... Args>
        std::shared_ptr<AssetType> load_asset(Args &&...args) const;

    private:
        template <typename AssetType, typename... Args>
        std::string get_cache_key(const Args &...args) const;

        template <typename ArgType>
        decltype(auto) resolve_arg(ArgType &&arg) const;

        std::filesystem::path m_asset_directory;
        mutable std::unordered_map<std::string, std::weak_ptr<void>> m_asset_cache;
    };
}

#include "sfg_trajectory_planner/engine/core/asset_locator.tpp"