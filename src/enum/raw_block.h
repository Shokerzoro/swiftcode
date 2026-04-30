#ifndef SWIFTCODE_ENUM_RAW_BLOCK_H
#define SWIFTCODE_ENUM_RAW_BLOCK_H

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace enum_pipeline {

struct RawEnumValue {
    std::string name;
    std::string explicit_value;
};

struct RawEnumInstruction {
    std::string array_name;
    std::string enum_name;
    int version{0};
    bool need_update{false};
    std::size_t marker_line{0};
    std::size_t enum_end_line{0};
    std::vector<RawEnumValue> values;
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
    explicit RawBlock(std::filesystem::path path)
        : infile{std::move(path)} {
        read_source();
        parse_source();
    }

    explicit operator bool() const noexcept {
        return has_updates;
    }

    void update_source(std::vector<std::string> updated_lines) const {
        std::ofstream output{infile, std::ios::trunc};
        if (!output) {
            throw std::runtime_error("Cannot update enum source file: " + infile.string());
        }

        for (auto const& line : updated_lines) {
            output << line << '\n';
        }
    }

private:
    friend class MidBlock;
    friend class OutBlock;

    static bool is_codegen_marker(std::string const& line) {
        auto trimmed = trim_copy(line);
        return starts_with(trimmed, "// CodeGen need") ||
               starts_with(trimmed, "// CodeGen: ");
    }

    static std::string strip_line_comment(std::string line) {
        auto comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        return line;
    }

    static std::string strip_block_comment(std::string line) {
        auto begin = line.find("/*");
        auto end = line.find("*/");
        if (begin != std::string::npos && end != std::string::npos && begin < end) {
            line.erase(begin, end - begin + 2);
        }
        return line;
    }

    static std::string parse_marker_name(std::string const& line) {
        auto trimmed = trim_copy(line);
        std::string const prefix = "// CodeGen need for ";
        if (!starts_with(trimmed, prefix)) {
            return {};
        }

        auto name = trim_copy(trimmed.substr(prefix.size()));
        if (!name.empty() && name.back() == '.') {
            name.pop_back();
        }
        return name;
    }

    static RawEnumInstruction parse_marker(std::string const& marker, std::size_t line_index) {
        auto trimmed = trim_copy(marker);
        RawEnumInstruction instruction;
        instruction.marker_line = line_index;

        std::string const need_prefix = "// CodeGen need";
        std::string const version_prefix = "// CodeGen: ";

        if (starts_with(trimmed, need_prefix)) {
            instruction.array_name = parse_marker_name(trimmed);
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

        instruction.array_name = trim_copy(rest.substr(0, version_pos));
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

    static std::string parse_enum_name(std::string const& line) {
        auto enum_pos = line.find("enum class ");
        if (enum_pos == std::string::npos) {
            return {};
        }

        auto name_begin = enum_pos + std::string{"enum class "}.size();
        while (name_begin < line.size() && std::isspace(static_cast<unsigned char>(line[name_begin])) != 0) {
            ++name_begin;
        }

        auto name_end = name_begin;
        while (name_end < line.size()) {
            auto ch = static_cast<unsigned char>(line[name_end]);
            if (std::isalnum(ch) == 0 && line[name_end] != '_') {
                break;
            }
            ++name_end;
        }

        return line.substr(name_begin, name_end - name_begin);
    }

    static RawEnumValue parse_enum_value(std::string line) {
        line = trim_copy(strip_block_comment(strip_line_comment(std::move(line))));
        if (!line.empty() && line.back() == ',') {
            line.pop_back();
        }
        line = trim_copy(line);

        RawEnumValue value;
        if (line.empty() || line == "{" || line == "};" || line == "}") {
            return value;
        }

        auto assign_pos = line.find('=');
        if (assign_pos == std::string::npos) {
            value.name = trim_copy(line);
            return value;
        }

        value.name = trim_copy(line.substr(0, assign_pos));
        value.explicit_value = trim_copy(line.substr(assign_pos + 1));
        return value;
    }

    void read_source() {
        std::ifstream input{infile};
        if (!input) {
            throw std::runtime_error("Cannot open enum source file: " + infile.string());
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
            auto enum_line = find_enum_line(index + 1);
            if (enum_line >= source_lines.size()) {
                throw std::runtime_error("CodeGen marker without following enum class in: " + infile.string());
            }

            instruction.enum_name = parse_enum_name(source_lines[enum_line]);
            if (instruction.enum_name.empty()) {
                throw std::runtime_error("Cannot parse enum class name in: " + infile.string());
            }
            if (instruction.array_name.empty()) {
                instruction.array_name = instruction.enum_name + "Pairs";
            }

            parse_enum_body(enum_line, instruction);

            if (instruction.need_update) {
                has_updates = true;
                instruction.version += 1;
                source_lines[index] = make_version_marker(instruction);
            }

            instructions.push_back(instruction);
            index = instruction.enum_end_line;
        }
    }

    std::size_t find_enum_line(std::size_t begin) const {
        for (std::size_t index = begin; index < source_lines.size(); ++index) {
            if (is_codegen_marker(source_lines[index])) {
                return source_lines.size();
            }
            if (source_lines[index].find("enum class ") != std::string::npos) {
                return index;
            }
        }
        return source_lines.size();
    }

    void parse_enum_body(std::size_t enum_line, RawEnumInstruction& instruction) const {
        bool in_body = source_lines[enum_line].find('{') != std::string::npos;

        for (std::size_t index = enum_line + 1U; index < source_lines.size(); ++index) {
            auto line = source_lines[index];
            if (!in_body) {
                in_body = line.find('{') != std::string::npos;
                continue;
            }

            if (line.find('}') != std::string::npos) {
                instruction.enum_end_line = index;
                return;
            }

            auto value = parse_enum_value(line);
            if (!value.name.empty()) {
                instruction.values.push_back(std::move(value));
            }
        }

        throw std::runtime_error("Unterminated enum class in: " + infile.string());
    }

    static std::string make_version_marker(RawEnumInstruction const& instruction) {
        return "// CodeGen: " + instruction.array_name + " version " +
               std::to_string(instruction.version) +
               ". Need update (yes/no): no";
    }

    std::filesystem::path infile;
    std::vector<std::string> source_lines;
    std::vector<RawEnumInstruction> instructions;
    bool has_updates{false};
};

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_RAW_BLOCK_H
