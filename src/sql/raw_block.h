#ifndef SWIFTCODE_SQL_RAW_BLOCK_H
#define SWIFTCODE_SQL_RAW_BLOCK_H

#include "file_resolving.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

namespace sql {

struct RawSqlInstruction {
    std::string name;
    int version{0};
    bool need_update{false};
    std::size_t marker_line{0};
    std::vector<std::string> sql_lines;
};

inline std::string trim_copy(std::string value) {
    auto is_space = [](unsigned char ch) { return std::isspace(ch) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(),
                                            [&](unsigned char ch) { return !is_space(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(),
                             [&](unsigned char ch) { return !is_space(ch); }).base(),
                value.end());
    return value;
}

inline std::string to_upper_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return value;
}

inline bool starts_with(std::string const& value, std::string const& prefix) {
    return value.compare(0, prefix.size(), prefix) == 0;
}

class RawBlock {
public:
    explicit RawBlock(FilePair const& pathpair)
        : infile{pathpair.first}, outfile{pathpair.second} {
        read_source();
        parse_source();
    }

    explicit operator bool() const noexcept {
        return has_updates;
    }

    void update() const {
        std::ofstream output{infile, std::ios::trunc};
        if (!output) {
            throw std::runtime_error("Cannot update raw SQL file: " + infile.string());
        }

        for (auto const& line : source_lines) {
            output << line << '\n';
        }
    }

private:
    friend class MidBlock;

    static bool is_codegen_marker(std::string const& line) {
        auto trimmed = trim_copy(line);
        return starts_with(trimmed, "-- CodeGen need for ") ||
               starts_with(trimmed, "-- CodeGen: ");
    }

    static bool ends_sql_instruction(std::string const& line) {
        auto sql_part = line;
        auto comment_pos = sql_part.find("-- replaceable:");
        if (comment_pos != std::string::npos) {
            sql_part = sql_part.substr(0, comment_pos);
        }

        auto trimmed = trim_copy(sql_part);
        return !trimmed.empty() && trimmed.back() == ';';
    }

    static RawSqlInstruction parse_marker(std::string const& marker, std::size_t line_index) {
        auto trimmed = trim_copy(marker);
        RawSqlInstruction instruction;
        instruction.marker_line = line_index;

        std::string const need_prefix = "-- CodeGen need for ";
        std::string const version_prefix = "-- CodeGen: ";

        if (starts_with(trimmed, need_prefix)) {
            instruction.name = trim_copy(trimmed.substr(need_prefix.size()));
            if (!instruction.name.empty() && instruction.name.back() == '.') {
                instruction.name.pop_back();
            }
            instruction.need_update = true;
            return instruction;
        }

        if (!starts_with(trimmed, version_prefix)) {
            return instruction;
        }

        auto rest = trimmed.substr(version_prefix.size());
        auto version_pos = rest.find(" version ");
        if (version_pos == std::string::npos) {
            throw std::runtime_error("Invalid CodeGen marker: " + marker);
        }

        instruction.name = trim_copy(rest.substr(0, version_pos));
        auto after_version = rest.substr(version_pos + std::string{" version "}.size());
        auto dot_pos = after_version.find('.');
        if (dot_pos == std::string::npos) {
            throw std::runtime_error("Invalid CodeGen version marker: " + marker);
        }

        instruction.version = std::stoi(after_version.substr(0, dot_pos));
        auto update_part = to_upper_copy(after_version.substr(dot_pos + 1));
        instruction.need_update = update_part.find("NEED UPDATE (YES/NO): YES") != std::string::npos;
        return instruction;
    }

    void read_source() {
        std::ifstream input{infile};
        if (!input) {
            throw std::runtime_error("Cannot open raw SQL file: " + infile.string());
        }

        std::string line;
        while (std::getline(input, line)) {
            source_lines.push_back(line);
        }
    }

    void parse_source() {
        for (std::size_t index = 0; index < source_lines.size(); ++index) {
            if (!is_codegen_marker(source_lines[index])) {
                continue;
            }

            auto instruction = parse_marker(source_lines[index], index);
            for (std::size_t line_index = index + 1; line_index < source_lines.size(); ++line_index) {
                if (is_codegen_marker(source_lines[line_index])) {
                    break;
                }

                instruction.sql_lines.push_back(source_lines[line_index]);
                if (ends_sql_instruction(source_lines[line_index])) {
                    break;
                }
            }

            if (instruction.need_update) {
                has_updates = true;
                instruction.version += 1;
                source_lines[index] = "-- CodeGen: " + instruction.name + " version " +
                                      std::to_string(instruction.version) +
                                      ". Need update (yes/no): no";
            }

            instructions.push_back(instruction);
        }
    }

    std::filesystem::path infile;
    std::filesystem::path outfile;
    std::vector<std::string> source_lines;
    std::vector<RawSqlInstruction> instructions;
    bool has_updates{false};
};

} // namespace sql

#endif // SWIFTCODE_SQL_RAW_BLOCK_H
