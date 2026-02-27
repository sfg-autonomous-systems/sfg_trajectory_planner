#include "sfg_trajectory_planner/engine/core/asset_locator.hpp"

namespace sfg_trajectory_planner::engine::core
{
    AssetLocator::AssetLocator(std::filesystem::path asset_directory)
        : m_asset_directory(asset_directory)
    {
    }
}