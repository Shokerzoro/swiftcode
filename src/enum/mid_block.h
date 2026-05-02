#ifndef SWIFTCODE_ENUM_MID_BLOCK_H
#define SWIFTCODE_ENUM_MID_BLOCK_H

#include "raw_block.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace enum_pipeline {

struct MidEnumValue {
    std::string expression;
    std::string text;
};

struct MidEnumInstruction {
    std::string array_name;
    std::string enum_name;
    int version{0};
    std::size_t enum_end_line{0};
    std::vector<MidEnumValue> values;
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
            mid.version = instruction.version;
            mid.enum_end_line = instruction.enum_end_line;

            for (auto const& value : instruction.values) {
                MidEnumValue mid_value;
                mid_value.expression = "static_cast<int>(" + mid.enum_name + "::" + value.name + ")";
                mid_value.text = value.name;
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

    std::filesystem::path infile;
    std::vector<std::string> source_lines;
    std::vector<MidEnumInstruction> instructions;
};

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_MID_BLOCK_H
