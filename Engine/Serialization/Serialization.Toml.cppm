module;

#include <toml++/toml.hpp>

export module Serialization.Toml;

import Core;

export namespace Toml
{
    using Table = toml::table;
    using Node = toml::node;

    Table fromFile(const std::filesystem::path& path)
    {
        return toml::parse_file(path.c_str());
    }

    void toFile(const Table& table, const std::filesystem::path& path)
    {
        std::ofstream file{path};
        file << table;
    }
}
