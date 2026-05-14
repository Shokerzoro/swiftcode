#ifndef SWIFTCODE_DOMAIN_RAW_BLOCK_H
#define SWIFTCODE_DOMAIN_RAW_BLOCK_H

#include "files_resolving.h"

#include "../enum/raw_block.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace domain {

struct RawDomainValue {
    std::string name;
    std::string explicit_value;
};

struct RawDomainInstruction {
    std::filesystem::path output_file;
    std::string table_name;
    std::vector<RawDomainValue> values;
};

class RawBlock {
public:
    explicit RawBlock(FilePair const& pathpair)
        : infile{pathpair.first}, outfile{pathpair.second} {
        enum_pipeline::RawBlock raw{infile};
        for (auto const& instruction : raw.instructions) {
            if (!instruction.has_domain()) {
                continue;
            }

            RawDomainInstruction domain_instruction;
            domain_instruction.output_file = outfile;
            domain_instruction.table_name = instruction.domain_table;
            for (auto const& value : instruction.values) {
                domain_instruction.values.push_back(RawDomainValue{value.name, value.explicit_value});
            }
            instructions.push_back(std::move(domain_instruction));
        }
    }

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

    void update() const {}

private:
    friend class MidBlock;

    std::filesystem::path infile;
    std::filesystem::path outfile;
    std::vector<RawDomainInstruction> instructions;
};

} // namespace domain

#endif // SWIFTCODE_DOMAIN_RAW_BLOCK_H
