#ifndef SWIFTCODE_VERBOSE_MID_BLOCK_H
#define SWIFTCODE_VERBOSE_MID_BLOCK_H

#include "raw_block.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace verbose {

struct MidVerboseValue {
    std::string name;
    std::string expression;
};

struct MidVerboseInstruction {
    std::string enum_name;
    std::string array_name;
    int version{0};
    std::vector<std::string> namespaces;
    std::vector<MidVerboseValue> values;
};

class MidBlock {
public:
    explicit MidBlock(RawBlock const& raw)
        : infile{raw.infile}, outfile{raw.outfile} {
        for (auto const& instruction : raw.instructions) {
            if (!instruction.need_update) {
                continue;
            }

            MidVerboseInstruction mid;
            mid.enum_name = instruction.enum_name;
            mid.array_name = instruction.array_name;
            mid.version = instruction.version;
            mid.namespaces = instruction.namespaces;

            for (auto const& value : instruction.values) {
                MidVerboseValue mid_value;
                mid_value.name = value.name;
                mid_value.expression = "static_cast<int>(" + mid.enum_name + "::" + value.name + ")";
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
    std::filesystem::path outfile;
    std::vector<MidVerboseInstruction> instructions;
};

} // namespace verbose

#endif // SWIFTCODE_VERBOSE_MID_BLOCK_H
