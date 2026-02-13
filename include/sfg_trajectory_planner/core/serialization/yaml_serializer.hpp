#pragma once

#include <yaml-cpp/yaml.h>

#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::core::serialization
{
    class YamlSerializer : public AbstractSerializer
    {
    public:
        YamlSerializer();

        void save_to_file(const std::filesystem::path &path) override;
        void load_from_file(const std::filesystem::path &path) override;

        void serialize(const std::string &key, bool value) override;
        void serialize(const std::string &key, size_t value) override;
        void serialize(const std::string &key, std::int32_t value) override;
        void serialize(const std::string &key, float value) override;
        void serialize(const std::string &key, const std::string &value) override;

        void serialize(const std::string &key, const std::vector<bool> &value) override;
        void serialize(const std::string &key, const std::vector<size_t> &value) override;
        void serialize(const std::string &key, const std::vector<std::int32_t> &value) override;
        void serialize(const std::string &key, const std::vector<float> &value) override;
        void serialize(const std::string &key, const std::vector<std::string> &value) override;

        void begin_group(const std::string &name) override;
        void end_group() override;

        size_t begin_sequence(const std::string &name, Mode mode) override;
        void next_item() override;
        void end_sequence() override;

    protected:
        void deserialize(const std::string &key, bool &value) override;
        void deserialize(const std::string &key, size_t &value) override;
        void deserialize(const std::string &key, std::int32_t &value) override;
        void deserialize(const std::string &key, float &value) override;
        void deserialize(const std::string &key, std::string &value) override;

        void deserialize(const std::string &key, std::vector<bool> &value) override;
        void deserialize(const std::string &key, std::vector<size_t> &value) override;
        void deserialize(const std::string &key, std::vector<std::int32_t> &value) override;
        void deserialize(const std::string &key, std::vector<float> &value) override;
        void deserialize(const std::string &key, std::vector<std::string> &value) override;

    private:
        struct SequenceContext
        {
            const Mode m_mode;
            size_t m_index = 0;
            bool m_pop_node_stack = false;
        };

        YAML::Node current_node();
        SequenceContext &current_sequence_context();

        YAML::Node m_root_node;
        std::vector<YAML::Node> m_node_stack;
        std::vector<SequenceContext> m_sequence_context;
    };
}