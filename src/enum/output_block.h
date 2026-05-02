#ifndef SWIFTCODE_ENUM_OUTPUT_BLOCK_H
#define SWIFTCODE_ENUM_OUTPUT_BLOCK_H

#include "mid_block.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace enum_pipeline {

class OutBlock {
public:
    explicit OutBlock(MidBlock const& mid)
        : infile{mid.infile}, source_lines{mid.source_lines}, instructions{mid.instructions} {}

    std::vector<std::string> updated_source() const {
        auto content = source_lines;
        remove_replaced_blocks(content);

        auto ordered = instructions;
        std::sort(ordered.begin(), ordered.end(),
                  [](MidEnumInstruction const& left, MidEnumInstruction const& right) {
                      return left.enum_end_line > right.enum_end_line;
                  });

        for (auto const& instruction : ordered) {
            auto insert_pos = find_enum_end(content, instruction.enum_name) + 1U;
            auto generated = make_generated_block(instruction);
            content.insert(content.begin() + static_cast<std::ptrdiff_t>(insert_pos),
                           generated.begin(), generated.end());
        }

        return content;
    }

    static std::size_t find_enum_end(std::vector<std::string> const& content,
                                     std::string const& enum_name) {
        std::string const enum_prefix = "enum class " + enum_name;
        for (std::size_t index = 0; index < content.size(); ++index) {
            if (content[index].find(enum_prefix) == std::string::npos) {
                continue;
            }

            for (std::size_t body_index = index; body_index < content.size(); ++body_index) {
                if (content[body_index].find('}') != std::string::npos) {
                    return body_index;
                }
            }
        }

        throw std::runtime_error("Cannot find enum class for generated array: " + enum_name);
    }

private:
    static std::vector<std::string> make_generated_block(MidEnumInstruction const& instruction) {
        std::vector<std::string> block;
        block.push_back("");
        block.push_back("// CodeGen from " + instruction.enum_name + " version " +
                        std::to_string(instruction.version) + ".");
        block.push_back("inline std::array<std::pair<int, std::string>, " +
                        std::to_string(instruction.values.size()) + "> " +
                        instruction.array_name + " = {{");

        for (auto const& value : instruction.values) {
            block.push_back("    {" + value.expression + ", \"" + value.text + "\"},");
        }

        block.push_back("}};");
        return block;
    }

    void remove_replaced_blocks(std::vector<std::string>& content) const {
        std::unordered_set<std::string> names;
        for (auto const& instruction : instructions) {
            names.insert(instruction.array_name);
            names.insert(instruction.enum_name);
        }

        std::vector<std::string> filtered;
        for (std::size_t index = 0; index < content.size();) {
            auto line = trim_copy(content[index]);
            std::string const prefix = "// CodeGen from ";
            if (!starts_with(line, prefix)) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            auto rest = line.substr(prefix.size());
            auto version_pos = rest.find(" version ");
            auto block_name = version_pos == std::string::npos ? rest : rest.substr(0, version_pos);
            auto domain_pos = block_name.find(" domain ");
            if (domain_pos != std::string::npos) {
                block_name = block_name.substr(0, domain_pos);
            }
            if (names.find(block_name) == names.end()) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            if (!filtered.empty() && filtered.back().empty()) {
                filtered.pop_back();
            }

            ++index;
            while (index < content.size() && trim_copy(content[index]) != "}};") {
                ++index;
            }
            if (index < content.size()) {
                ++index;
            }
        }

        content = std::move(filtered);
    }

    std::filesystem::path infile;
    std::vector<std::string> source_lines;
    std::vector<MidEnumInstruction> instructions;
};

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_OUTPUT_BLOCK_H
