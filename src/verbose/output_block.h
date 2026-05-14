#ifndef SWIFTCODE_VERBOSE_OUTPUT_BLOCK_H
#define SWIFTCODE_VERBOSE_OUTPUT_BLOCK_H

#include "mid_block.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace verbose {

class OutBlock {
public:
    explicit OutBlock(MidBlock const& mid, bool qdebug_enabled = false)
        : infile{mid.infile},
          outfile{mid.outfile},
          instructions{mid.instructions},
          generate_qdebug{qdebug_enabled} {}

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

    void update() const {
        auto content = read_existing_header();
        remove_replaced_blocks(content);

        if (content.empty()) {
            content = make_header_prefix();
        } else if (generate_qdebug) {
            ensure_include(content, "#include <QDebug>");
        }

        while (!content.empty() && content.back().empty()) {
            content.pop_back();
        }

        for (auto const& instruction : instructions) {
            content.push_back("");
            auto block = make_generated_block(instruction, generate_qdebug);
            content.insert(content.end(), block.begin(), block.end());
        }

        std::filesystem::create_directories(outfile.parent_path());
        std::ofstream output{outfile, std::ios::trunc};
        if (!output) {
            throw std::runtime_error("Cannot update verbose generated header: " + outfile.string());
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

    std::vector<std::string> make_header_prefix() const {
        std::vector<std::string> content;
        content.push_back("#pragma once");
        content.push_back("");
        content.push_back("#include <array>");
        if (generate_qdebug) {
            content.push_back("#include <QDebug>");
        }
        content.push_back("#include <ostream>");
        content.push_back("#include <string>");
        content.push_back("#include <utility>");
        content.push_back("");
        content.push_back("#include \"" + source_include_path() + "\"");
        return content;
    }

    std::string source_include_path() const {
        std::filesystem::path include_path;
        try {
            include_path = std::filesystem::relative(infile, outfile.parent_path());
        } catch (std::exception const&) {
            include_path = infile;
        }

        if (include_path.empty()) {
            include_path = infile.filename();
        }

        return include_path.generic_string();
    }

    static std::vector<std::string> make_generated_block(MidVerboseInstruction const& instruction,
                                                         bool generate_qdebug) {
        std::vector<std::string> block;
        block.push_back("// Verbose from " + instruction.enum_name + " version " +
                        std::to_string(instruction.version) + ".");

        for (auto const& name : instruction.namespaces) {
            block.push_back("namespace " + name + " {");
        }

        block.push_back("inline std::array<std::pair<std::string, int>, " +
                        std::to_string(instruction.values.size()) + "> " +
                        instruction.array_name + " = {{");
        for (auto const& value : instruction.values) {
            block.push_back("    {\"" + value.name + "\", " + value.expression + "},");
        }
        block.push_back("}};");
        block.push_back("");
        block.push_back("inline std::ostream& operator<<(std::ostream& os, " +
                        instruction.enum_name + " value) {");
        block.push_back("    for (auto const& item : " + instruction.array_name + ") {");
        block.push_back("        if (item.second == static_cast<int>(value)) {");
        block.push_back("            return os << item.first;");
        block.push_back("        }");
        block.push_back("    }");
        block.push_back("    return os << static_cast<int>(value);");
        block.push_back("}");
        if (generate_qdebug) {
            block.push_back("");
            block.push_back("inline QDebug operator<<(QDebug dbg, " +
                            instruction.enum_name + " value) {");
            block.push_back("    for (auto const& item : " + instruction.array_name + ") {");
            block.push_back("        if (item.second == static_cast<int>(value)) {");
            block.push_back("            dbg << item.first.c_str();");
            block.push_back("            return dbg;");
            block.push_back("        }");
            block.push_back("    }");
            block.push_back("    dbg << static_cast<int>(value);");
            block.push_back("    return dbg;");
            block.push_back("}");
        }

        for (auto iter = instruction.namespaces.rbegin();
             iter != instruction.namespaces.rend(); ++iter) {
            block.push_back("} // namespace " + *iter);
        }

        block.push_back("// Verbose end " + instruction.enum_name + ".");
        return block;
    }

    void remove_replaced_blocks(std::vector<std::string>& content) const {
        std::unordered_set<std::string> names;
        for (auto const& instruction : instructions) {
            names.insert(instruction.enum_name);
        }

        std::vector<std::string> filtered;
        for (std::size_t index = 0; index < content.size();) {
            auto line = trim_copy(content[index]);
            std::string const prefix = "// Verbose from ";
            if (!starts_with(line, prefix)) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            auto rest = line.substr(prefix.size());
            auto version_pos = rest.find(" version ");
            auto enum_name = version_pos == std::string::npos ? rest : rest.substr(0, version_pos);
            if (names.find(enum_name) == names.end()) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            ++index;
            auto end_marker = "// Verbose end " + enum_name + ".";
            while (index < content.size() && trim_copy(content[index]) != end_marker) {
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

    static void ensure_include(std::vector<std::string>& content, std::string const& include_line) {
        for (auto const& line : content) {
            if (trim_copy(line) == include_line) {
                return;
            }
        }

        std::size_t insert_index = 0;
        for (std::size_t index = 0; index < content.size(); ++index) {
            auto const line = trim_copy(content[index]);
            if (starts_with(line, "#include \"")) {
                insert_index = index;
                break;
            }
            if (starts_with(line, "#include ")) {
                insert_index = index + 1;
            }
        }

        content.insert(content.begin() + static_cast<std::ptrdiff_t>(insert_index), include_line);
    }

    std::filesystem::path infile;
    std::filesystem::path outfile;
    std::vector<MidVerboseInstruction> instructions;
    bool generate_qdebug{false};
};

} // namespace verbose

#endif // SWIFTCODE_VERBOSE_OUTPUT_BLOCK_H
