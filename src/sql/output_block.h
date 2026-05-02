#ifndef SWIFTCODE_SQL_OUTPUT_BLOCK_H
#define SWIFTCODE_SQL_OUTPUT_BLOCK_H

#include "mid_block.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace sql {

class OutBlock {
public:
    explicit OutBlock(MidBlock const& mid)
        : outfile{mid.outfile}, instructions{mid.instructions} {}

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

    void update() const {
        auto content = read_existing_header();
        remove_replaced_blocks(content);

        if (content.empty()) {
            content.push_back("#pragma once");
            content.push_back("");
            content.push_back("namespace sql {");
        } else if (!content.empty() && trim_copy(content.back()) == "} // namespace sql") {
            content.pop_back();
        }

        for (auto const& instruction : instructions) {
            if (!content.empty() && !content.back().empty()) {
                content.push_back("");
            }

            content.push_back("// CodeGen from " + instruction.name + " version " +
                              std::to_string(instruction.version) + ".");
            content.push_back("static constexpr const char* " + instruction.name + " = R\"SQL(");
            for (auto const& line : instruction.generated_lines) {
                content.push_back(line);
            }
            content.push_back(")SQL\";");
        }

        content.push_back("");
        content.push_back("} // namespace sql");

        std::ofstream output{outfile, std::ios::trunc};
        if (!output) {
            throw std::runtime_error("Cannot update generated SQL header: " + outfile.string());
        }

        for (auto const& line : content) {
            output << line << '\n';
        }
    }

private:
    std::vector<std::string> read_existing_header() const {
        std::vector<std::string> content;
        std::ifstream input{outfile};
        if (!input) {
            return content;
        }

        std::string line;
        while (std::getline(input, line)) {
            content.push_back(line);
        }

        return content;
    }

    void remove_replaced_blocks(std::vector<std::string>& content) const {
        std::unordered_set<std::string> names;
        for (auto const& instruction : instructions) {
            names.insert(instruction.name);
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
            if (names.find(block_name) == names.end()) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            ++index;
            while (index < content.size() && trim_copy(content[index]) != ")SQL\";") {
                ++index;
            }
            if (index < content.size()) {
                ++index;
            }
        }

        while (!filtered.empty() && filtered.back().empty()) {
            filtered.pop_back();
        }

        content = std::move(filtered);
    }

    std::filesystem::path outfile;
    std::vector<MidSqlInstruction> instructions;
};

} // namespace sql

#endif // SWIFTCODE_SQL_OUTPUT_BLOCK_H
