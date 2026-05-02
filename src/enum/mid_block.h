#ifndef SWIFTCODE_ENUM_MID_BLOCK_H
#define SWIFTCODE_ENUM_MID_BLOCK_H

#include "raw_block.h"

#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace enum_pipeline {

struct MidEnumValue {
    std::string expression;
    std::string text;
    std::string sql_id;
};

struct MidEnumInstruction {
    std::string array_name;
    std::string enum_name;
    std::string domain_table;
    int version{0};
    std::size_t enum_end_line{0};
    std::vector<MidEnumValue> values;

    bool has_domain() const noexcept {
        return !domain_table.empty();
    }
};

class MidBlock {
public:
    explicit MidBlock(RawBlock const& raw, bool include_unchanged = false)
        : infile{raw.infile}, source_lines{raw.source_lines} {
        for (auto const& instruction : raw.instructions) {
            if (!include_unchanged && !instruction.need_update) {
                continue;
            }

            MidEnumInstruction mid;
            mid.array_name = instruction.array_name;
            mid.enum_name = instruction.enum_name;
            mid.domain_table = instruction.domain_table;
            mid.version = instruction.version;
            mid.enum_end_line = instruction.enum_end_line;

            int next_implicit_value = 0;
            for (auto const& value : instruction.values) {
                MidEnumValue mid_value;
                mid_value.expression = "static_cast<int>(" + mid.enum_name + "::" + value.name + ")";
                mid_value.text = value.name;
                if (!value.explicit_value.empty()) {
                    mid_value.sql_id = value.explicit_value;
                    next_implicit_value = parse_integral_literal(value.explicit_value) + 1;
                } else {
                    mid_value.sql_id = std::to_string(next_implicit_value);
                    ++next_implicit_value;
                }
                mid.values.push_back(std::move(mid_value));
            }

            instructions.push_back(std::move(mid));
        }
    }

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

private:
    friend class OutBlock;
    friend class DomainSqlBlock;

    static int parse_integral_literal(std::string value) {
        value = trim_copy(std::move(value));
        while (!value.empty()) {
            auto ch = static_cast<unsigned char>(value.back());
            if (std::isdigit(ch) != 0 || value.back() == 'x' || value.back() == 'X') {
                break;
            }
            value.pop_back();
        }

        std::size_t parsed = 0;
        int result = std::stoi(value, &parsed, 0);
        if (parsed != value.size()) {
            throw std::runtime_error("Unsupported enum value for SQL domain: " + value);
        }
        return result;
    }

    std::filesystem::path infile;
    std::vector<std::string> source_lines;
    std::vector<MidEnumInstruction> instructions;
};

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_MID_BLOCK_H
