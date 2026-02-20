#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sfg_trajectory_planner::engine::core::serialization
{
    class AbstractSerializer
    {
    public:
        enum class Mode
        {
            Read,
            Write
        };

        AbstractSerializer() = default;
        AbstractSerializer(const AbstractSerializer &) = delete;
        AbstractSerializer &operator=(const AbstractSerializer &) = delete;
        AbstractSerializer(AbstractSerializer &&) = delete;
        AbstractSerializer &operator=(AbstractSerializer &&) = delete;
        virtual ~AbstractSerializer() = default;

        virtual void save_to_file(const std::filesystem::path &path) = 0;
        virtual void load_from_file(const std::filesystem::path &path) = 0;

        // Serialization methods for primitive types and strings.
        virtual void serialize(const std::string &key, bool value) = 0;
        virtual void serialize(const std::string &key, size_t value) = 0;
        virtual void serialize(const std::string &key, std::int32_t value) = 0;
        virtual void serialize(const std::string &key, float value) = 0;
        virtual void serialize(const std::string &key, const std::string &value) = 0;

        // Serialization methods for vectors of primitive types and strings.
        virtual void serialize(const std::string &key, const std::vector<bool> &value) = 0;
        virtual void serialize(const std::string &key, const std::vector<size_t> &value) = 0;
        virtual void serialize(const std::string &key, const std::vector<std::int32_t> &value) = 0;
        virtual void serialize(const std::string &key, const std::vector<float> &value) = 0;
        virtual void serialize(const std::string &key, const std::vector<std::string> &value) = 0;

        // Convenience deserialization method that deduces the type from the provided reference and
        // calls the corresponding protected deserialization method.
        template <typename ValueType>
        ValueType deserialize(const std::string &key);

        // Grouping methods to allow serializers to represent hierarchical data structures.
        virtual void begin_group(const std::string &name) = 0;
        virtual void end_group() = 0;

        // Sequence methods to allow serializers to represent lists of items.
        virtual size_t begin_sequence(const std::string &name, Mode mode) = 0;
        virtual void next_item() = 0;
        virtual void end_sequence() = 0;

    protected:
        // Deserialization methods for primitive types and strings.
        virtual void deserialize(const std::string &key, bool &value) = 0;
        virtual void deserialize(const std::string &key, size_t &value) = 0;
        virtual void deserialize(const std::string &key, std::int32_t &value) = 0;
        virtual void deserialize(const std::string &key, float &value) = 0;
        virtual void deserialize(const std::string &key, std::string &value) = 0;

        // Deserialization methods for vectors of primitive types and strings.
        virtual void deserialize(const std::string &key, std::vector<bool> &value) = 0;
        virtual void deserialize(const std::string &key, std::vector<size_t> &value) = 0;
        virtual void deserialize(const std::string &key, std::vector<std::int32_t> &value) = 0;
        virtual void deserialize(const std::string &key, std::vector<float> &value) = 0;
        virtual void deserialize(const std::string &key, std::vector<std::string> &value) = 0;
    };
}

#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.tpp"