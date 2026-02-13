#pragma once

#include <string>
#include <variant>
#include <vector>

namespace sfg_trajectory_planner::core::serialization
{

    class AbstractSerializer
    {
    public:
        using ValueVariant = std::variant<
            std::string, size_t, int, float, bool,
            std::vector<std::string>, std::vector<size_t>, std::vector<int>, std::vector<float>, std::vector<bool>>;

        virtual ~AbstractSerializer() = default;
        AbstractSerializer(const AbstractSerializer &) = delete;
        AbstractSerializer &operator=(const AbstractSerializer &) = delete;
        AbstractSerializer(AbstractSerializer &&) = delete;
        AbstractSerializer &operator=(AbstractSerializer &&) = delete;

        virtual void serialize(const std::string &key, const ValueVariant &value) = 0;
        virtual ValueVariant deserialize(const std::string &key) = 0;
        virtual void push_group(const std::string &name) = 0;
        virtual void pop_group() = 0;
    };
}