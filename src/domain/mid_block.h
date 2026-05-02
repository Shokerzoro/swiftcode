#ifndef SWIFTCODE_DOMAIN_MID_BLOCK_H
#define SWIFTCODE_DOMAIN_MID_BLOCK_H

#include "raw_block.h"

#include "../enum/raw_block.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace domain {

struct MidDomainValue {
    std::string id;
    std::string text;
};

struct MidDomainInstruction {
    std::filesystem::path output_file;
    std::string table_name;
    std::vector<MidDomainValue> values;
};

class MidBlock {
public:
    explicit MidBlock(RawBlock const& raw) {
        for (auto const& instruction : raw.instructions) {
            MidDomainInstruction mid;
            mid.output_file = instruction.output_file;
            mid.table_name = instruction.table_name;

            int next_implicit_value = 0;
            for (auto const& value : instruction.values) {
                MidDomainValue mid_value;
                mid_value.text = value.name;
                if (!value.explicit_value.empty()) {
                    mid_value.id = value.explicit_value;
                    next_implicit_value = parse_integral_literal(value.explicit_value) + 1;
                } else {
                    mid_value.id = std::to_string(next_implicit_value);
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

    static int parse_integral_literal(std::string value) {
        value = enum_pipeline::trim_copy(std::move(value));
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

    std::vector<MidDomainInstruction> instructions;
};

} // namespace domain

#endif // SWIFTCODE_DOMAIN_MID_BLOCK_H
