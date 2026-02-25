#include "sfg_trajectory_planner/engine/core/serialization/yaml_serializer.hpp"

#include <cstring>
#include <fstream>

namespace sfg_trajectory_planner::engine::core::serialization
{
    YamlSerializer::YamlSerializer() : AbstractSerializer(), m_root_node(YAML::Node(YAML::NodeType::Map))
    {
        m_node_stack.push_back(m_root_node);
    }

    void YamlSerializer::save_to_file(const std::filesystem::path &path)
    {
        std::ofstream stream(path);
        stream << m_root_node;
    }

    void YamlSerializer::load_from_file(const std::filesystem::path &path)
    {
        m_root_node = YAML::LoadFile(path);
        m_node_stack.clear();
        m_node_stack.push_back(m_root_node);
        m_sequence_context.clear();
    }

    std::vector<std::uint8_t> YamlSerializer::to_bytes() const
    {
        YAML::Emitter emitter;
        emitter << m_root_node;
        auto size = emitter.size();
        const auto yaml = reinterpret_cast<const std::uint8_t *>(emitter.c_str());
        return {yaml, yaml + size};
    }

    void YamlSerializer::from_bytes(const std::vector<std::uint8_t> &data)
    {
        if (data.empty())
        {
            m_root_node = YAML::Node(YAML::NodeType::Map);
            return;
        }

        m_root_node = YAML::Load(std::string(reinterpret_cast<const char *>(data.data()), data.size()));
        m_node_stack.clear();
        m_node_stack.push_back(m_root_node);
        m_sequence_context.clear();
    }

    void YamlSerializer::serialize(const std::string &key, bool value)
    {
        current_node()[key] = value;
    }

    void YamlSerializer::serialize(const std::string &key, size_t value)
    {
        current_node()[key] = value;
    }

    void YamlSerializer::serialize(const std::string &key, std::int32_t value)
    {
        current_node()[key] = value;
    }

    void YamlSerializer::serialize(const std::string &key, float value)
    {
        current_node()[key] = value;
    }

    void YamlSerializer::serialize(const std::string &key, const std::string &value)
    {
        current_node()[key] = value;
    }

    void YamlSerializer::serialize(const std::string &key, const std::vector<bool> &value)
    {
        serialize_vector(key, value);
    }

    void YamlSerializer::serialize(const std::string &key, const std::vector<size_t> &value)
    {
        serialize_vector(key, value);
    }

    void YamlSerializer::serialize(const std::string &key, const std::vector<std::int32_t> &value)
    {
        serialize_vector(key, value);
    }

    void YamlSerializer::serialize(const std::string &key, const std::vector<float> &value)
    {
        serialize_vector(key, value);
    }

    void YamlSerializer::serialize(const std::string &key, const std::vector<std::string> &value)
    {
        serialize_vector(key, value);
    }

    void YamlSerializer::deserialize(const std::string &key, bool &value)
    {
        value = current_node()[key].as<bool>();
    }

    void YamlSerializer::deserialize(const std::string &key, size_t &value)
    {
        value = current_node()[key].as<size_t>();
    }

    void YamlSerializer::deserialize(const std::string &key, std::int32_t &value)
    {
        value = current_node()[key].as<std::int32_t>();
    }

    void YamlSerializer::deserialize(const std::string &key, float &value)
    {
        value = current_node()[key].as<float>();
    }

    void YamlSerializer::deserialize(const std::string &key, std::string &value)
    {
        value = current_node()[key].as<std::string>();
    }

    void YamlSerializer::deserialize(const std::string &key, std::vector<bool> &value)
    {
        value.clear();

        for (const auto &item : current_node()[key])
        {
            value.push_back(item.as<bool>());
        }
    }

    void YamlSerializer::deserialize(const std::string &key, std::vector<size_t> &value)
    {
        value.clear();

        for (const auto &item : current_node()[key])
        {
            value.push_back(item.as<size_t>());
        }
    }

    void YamlSerializer::deserialize(const std::string &key, std::vector<std::int32_t> &value)
    {
        value.clear();

        for (const auto &item : current_node()[key])
        {
            value.push_back(item.as<std::int32_t>());
        }
    }

    void YamlSerializer::deserialize(const std::string &key, std::vector<float> &value)
    {
        value.clear();

        for (const auto &item : current_node()[key])
        {
            value.push_back(item.as<float>());
        }
    }

    void YamlSerializer::deserialize(const std::string &key, std::vector<std::string> &value)
    {
        value.clear();

        for (const auto &item : current_node()[key])
        {
            value.push_back(item.as<std::string>());
        }
    }

    void YamlSerializer::begin_group(const std::string &name)
    {
        m_node_stack.push_back({current_node()[name]});
    }

    void YamlSerializer::end_group()
    {
        m_node_stack.pop_back();
    }

    size_t YamlSerializer::begin_sequence(const std::string &name, Mode mode)
    {
        m_sequence_context.push_back({mode});

        switch (mode)
        {
            case Mode::Read:
            {
                m_node_stack.push_back({current_node()[name]});
                return current_node().size();
            }
            case Mode::Write:
            {
                m_node_stack.push_back({current_node()[name]});
                break;
            }
        }
        return 0;
    }

    void YamlSerializer::next_item()
    {
        auto &sequence_context = current_sequence_context();

        if (sequence_context.m_pop_node_stack)
        {
            m_node_stack.pop_back();
        }
        sequence_context.m_pop_node_stack = true;

        switch (sequence_context.m_mode)
        {
            case Mode::Read:
            {
                m_node_stack.push_back(current_node()[sequence_context.m_index++]);
                break;
            }
            case Mode::Write:
            {
                current_node().push_back({});
                m_node_stack.push_back(current_node()[current_node().size() - 1]);
                break;
            }
        }
    }

    void YamlSerializer::end_sequence()
    {
        if (current_sequence_context().m_pop_node_stack)
        {
            m_node_stack.pop_back();
        }
        m_node_stack.pop_back();
        m_sequence_context.pop_back();
    }

    YAML::Node YamlSerializer::current_node()
    {
        return m_node_stack.back();
    }

    YamlSerializer::SequenceContext &YamlSerializer::current_sequence_context()
    {
        return m_sequence_context.back();
    }
}