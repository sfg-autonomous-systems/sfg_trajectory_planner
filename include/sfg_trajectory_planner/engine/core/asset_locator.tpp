#include "sfg_trajectory_planner/engine/core/asset_locator.hpp"

#include <sstream>
#include <type_traits>

namespace sfg_trajectory_planner::engine::core
{

    template <typename AssetType, typename... Args>
    std::shared_ptr<AssetType> AssetLocator::load_asset(Args &&...args) const
    {
        std::string key = get_cache_key<AssetType>(args...);
        auto iterator = m_asset_cache.find(key);

        if (iterator != m_asset_cache.end())
        {
            if (auto cached_asset = iterator->second.lock())
            {
                return std::static_pointer_cast<AssetType>(cached_asset);
            }
        }
        auto asset = std::make_shared<AssetType>(resolve_arg(std::forward<Args>(args))...);
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