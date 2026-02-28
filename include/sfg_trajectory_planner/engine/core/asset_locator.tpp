#include "sfg_trajectory_planner/engine/core/asset_locator.hpp"

#include <sstream>
#include <type_traits>

namespace sfg_trajectory_planner::engine::core
{

    template <typename AssetType, typename... Args>
    std::shared_ptr<AssetType> AssetLocator::load_asset(Args &&...args) const
    {
        std::string key;
        std::shared_ptr<AssetType> cached_asset;

        if (try_load_from_cache(cached_asset, key, args...))
        {
            return cached_asset;
        }

        std::shared_ptr<AssetType> asset;

        if constexpr (std::is_same_v<AssetType, gfx::Material>)
        {
            asset = load_material(resolve_arg(std::forward<Args>(args))...);
        }
        else if constexpr (std::is_same_v<AssetType, gfx::Shader>)
        {
            asset = load_shader(resolve_arg(std::forward<Args>(args))...);
        }
        else if constexpr (std::is_same_v<AssetType, gfx::TextFont>)
        {
            asset = load_text_font(resolve_arg(std::forward<Args>(args))...);
        }
        else
        {
            asset = std::make_shared<AssetType>(resolve_arg(std::forward<Args>(args))...);
        }
        m_asset_cache[key] = asset;
        return asset;
    }

    template <typename AssetType, typename... Args>
    std::string AssetLocator::get_cache_key(const Args &...args) const
    {
        std::stringstream stream;
        stream << typeid(AssetType).name();
        ((stream << '|' << args), ...);
        return stream.str();
    }

    template <typename AssetType, typename... Args>
    bool AssetLocator::try_load_from_cache(std::shared_ptr<AssetType> &cached_asset, std::string &out_key, const Args &...args) const
    {
        out_key = get_cache_key<AssetType>(args...);
        auto iterator = m_asset_cache.find(out_key);

        if (iterator != m_asset_cache.end())
        {
            if (auto asset = iterator->second.lock())
            {
                cached_asset = std::static_pointer_cast<AssetType>(asset);
                return true;
            }
            else
            {
                m_asset_cache.erase(iterator);
            }
        }
        return false;
    }

    template <typename ArgType>
    decltype(auto) AssetLocator::resolve_arg(ArgType &&arg) const
    {
        if constexpr (std::is_same_v<std::decay_t<ArgType>, std::filesystem::path>)
        {
            return m_asset_directory / arg;
        }
        else
        {
            return std::forward<ArgType>(arg);
        }
    }
}