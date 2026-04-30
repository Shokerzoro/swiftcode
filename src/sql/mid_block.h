#ifndef SWIFTCODE_SQL_MID_BLOCK_H
#define SWIFTCODE_SQL_MID_BLOCK_H

#include "raw_block.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace sql {

struct MidSqlInstruction {
    std::string name;
    int version{0};
    std::vector<std::string> generated_lines;
};

class MidBlock {
public:
    explicit MidBlock(RawBlock const& raw)
        : infile{raw.infile}, outfile{raw.outfile} {
        for (auto const& instruction : raw.instructions) {
            if (!instruction.need_update) {
                continue;
            }

            MidSqlInstruction mid;
            mid.name = instruction.name;
            mid.version = instruction.version;
            mid.generated_lines = normalize_sql(instruction.sql_lines);
            instructions.push_back(std::move(mid));
        }
    }

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

private:
    friend class OutBlock;

    static std::vector<std::string> split_replaceables(std::string const& list) {
        std::vector<std::string> replaceables;
        std::size_t begin = 0;

        while (begin < list.size()) {
            auto comma = list.find(',', begin);
            auto value = trim_copy(list.substr(begin, comma - begin));
            if (!value.empty()) {
                replaceables.push_back(value);
            }

            if (comma == std::string::npos) {
                break;
            }
            begin = comma + 1;
        }

        return replaceables;
    }

    static void replace_all(std::string& line, std::string const& from, std::string const& to) {
        if (from.empty()) {
            return;
        }

        std::size_t pos = 0;
        while ((pos = line.find(from, pos)) != std::string::npos) {
            line.replace(pos, from.size(), to);
            pos += to.size();
        }
    }

    static std::vector<std::string> normalize_sql(std::vector<std::string> const& sql_lines) {
        std::vector<std::string> normalized;
        normalized.reserve(sql_lines.size());

        for (auto line : sql_lines) {
            auto replaceable_pos = line.find("-- replaceable:");
            if (replaceable_pos != std::string::npos) {
                auto replaceables = split_replaceables(line.substr(replaceable_pos + std::string{"-- replaceable:"}.size()));
                line = trim_copy(line.substr(0, replaceable_pos));

                for (auto const& replaceable : replaceables) {
                    replace_all(line, replaceable, "%" + to_upper_copy(replaceable) + "%");
                }
            }

            normalized.push_back(std::move(line));
        }

        return normalized;
    }

    std::filesystem::path infile;
    std::filesystem::path outfile;
    std::vector<MidSqlInstruction> instructions;
};

} // namespace sql

#endif // SWIFTCODE_SQL_MID_BLOCK_H
