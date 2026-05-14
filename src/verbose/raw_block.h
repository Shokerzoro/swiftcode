#ifndef SWIFTCODE_VERBOSE_RAW_BLOCK_H
#define SWIFTCODE_VERBOSE_RAW_BLOCK_H

#include "file_resolving.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace verbose {

struct RawVerboseValue {
    std::string name;
};

struct RawVerboseInstruction {
    std::string enum_name;
    std::string array_name;
    int version{0};
    bool need_update{false};
    std::size_t enum_line{0};
    std::size_t enum_end_line{0};
    std::vector<std::string> namespaces;
    std::vector<RawVerboseValue> values;
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
            throw std::runtime_error("Cannot update verbose source file: " + infile.string());
        }

        for (auto const& line : source_lines) {
            output << line << '\n';
        }
    }

private:
    friend class MidBlock;

    struct NamespaceScope {
        std::string name;
        int depth{0};
    };

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

    static bool is_verbose_marker(std::string const& line) {
        return line.find("// Verbose needed") != std::string::npos ||
               line.find("// Verbosed version ") != std::string::npos;
    }

    static bool is_verbose_enum_line(std::string const& line) {
        return line.find("enum class ") != std::string::npos && is_verbose_marker(line);
    }

    static std::string parse_enum_name(std::string const& line) {
        auto enum_pos = line.find("enum class ");
        if (enum_pos == std::string::npos) {
            return {};
        }

        auto name_begin = enum_pos + std::string{"enum class "}.size();
        while (name_begin < line.size() &&
               std::isspace(static_cast<unsigned char>(line[name_begin])) != 0) {
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

    static int parse_version(std::string const& line) {
        auto marker_pos = line.find("// Verbosed version ");
        if (marker_pos == std::string::npos) {
            return 0;
        }

        auto version_begin = marker_pos + std::string{"// Verbosed version "}.size();
        auto version_end = version_begin;
        while (version_end < line.size() &&
               std::isdigit(static_cast<unsigned char>(line[version_end])) != 0) {
            ++version_end;
        }
        if (version_end == version_begin) {
            throw std::runtime_error("Invalid Verbosed version marker: " + line);
        }

        return std::stoi(line.substr(version_begin, version_end - version_begin));
    }

    static bool parse_need_update(std::string const& line) {
        if (line.find("// Verbose needed") != std::string::npos) {
            return true;
        }
        auto marker_pos = line.find("// Verbosed version ");
        if (marker_pos == std::string::npos) {
            return false;
        }

        auto update_part = to_upper_copy(line.substr(marker_pos));
        return update_part.find("NEED UPDATE(YES/NO): YES") != std::string::npos;
    }

    static std::string make_version_marker(int version) {
        return "// Verbosed version " + std::to_string(version) +
               ". Need update(yes/no): no";
    }

    static std::string replace_marker(std::string const& line, int version) {
        auto marker_pos = line.find("// Verbose needed");
        if (marker_pos == std::string::npos) {
            marker_pos = line.find("// Verbosed version ");
        }
        if (marker_pos == std::string::npos) {
            return line;
        }
        return line.substr(0, marker_pos) + make_version_marker(version);
    }

    static RawVerboseValue parse_enum_value(std::string line) {
        line = trim_copy(strip_block_comment(strip_line_comment(std::move(line))));
        if (!line.empty() && line.back() == ',') {
            line.pop_back();
        }
        line = trim_copy(line);

        RawVerboseValue value;
        if (line.empty() || line == "{" || line == "};" || line == "}") {
            return value;
        }

        auto assign_pos = line.find('=');
        if (assign_pos != std::string::npos) {
            line = line.substr(0, assign_pos);
        }

        auto name = trim_copy(line);
        if (name.empty()) {
            return value;
        }

        auto ch = static_cast<unsigned char>(name.front());
        if (std::isalpha(ch) == 0 && name.front() != '_') {
            return value;
        }

        value.name = std::move(name);
        return value;
    }

    static std::vector<std::string> split_namespace_name(std::string name) {
        std::vector<std::string> result;
        for (;;) {
            auto pos = name.find("::");
            auto part = trim_copy(name.substr(0, pos));
            if (!part.empty()) {
                result.push_back(part);
            }
            if (pos == std::string::npos) {
                break;
            }
            name = name.substr(pos + 2);
        }
        return result;
    }

    static std::vector<std::string> parse_namespace_names(std::string const& line) {
        auto code = trim_copy(strip_line_comment(line));
        std::string const prefix = "namespace ";
        if (!starts_with(code, prefix)) {
            return {};
        }

        auto name_begin = prefix.size();
        while (name_begin < code.size() &&
               std::isspace(static_cast<unsigned char>(code[name_begin])) != 0) {
            ++name_begin;
        }

        auto name_end = name_begin;
        while (name_end < code.size()) {
            auto ch = static_cast<unsigned char>(code[name_end]);
            if (std::isalnum(ch) == 0 && code[name_end] != '_' && code[name_end] != ':') {
                break;
            }
            ++name_end;
        }

        if (name_end == name_begin) {
            return {};
        }

        return split_namespace_name(code.substr(name_begin, name_end - name_begin));
    }

    static int brace_delta(std::string const& line) {
        auto code = strip_line_comment(line);
        int delta = 0;
        for (auto ch : code) {
            if (ch == '{') {
                ++delta;
            } else if (ch == '}') {
                --delta;
            }
        }
        return delta;
    }

    void read_source() {
        std::ifstream input{infile};
        if (!input) {
            throw std::runtime_error("Cannot open verbose source file: " + infile.string());
        }

        std::string line;
        while (std::getline(input, line)) {
            source_lines.push_back(line);
        }
    }

    void parse_source() {
        int brace_depth = 0;
        std::vector<NamespaceScope> namespace_stack;

        for (std::size_t index = 0; index < source_lines.size(); ++index) {
            while (!namespace_stack.empty() && namespace_stack.back().depth > brace_depth) {
                namespace_stack.pop_back();
            }

            if (is_verbose_enum_line(source_lines[index])) {
                RawVerboseInstruction instruction;
                instruction.enum_line = index;
                instruction.enum_name = parse_enum_name(source_lines[index]);
                if (instruction.enum_name.empty()) {
                    throw std::runtime_error("Cannot parse verbose enum class name in: " + infile.string());
                }
                instruction.array_name = instruction.enum_name + "Pairs";
                instruction.version = parse_version(source_lines[index]);
                instruction.need_update = parse_need_update(source_lines[index]);
                for (auto const& scope : namespace_stack) {
                    instruction.namespaces.push_back(scope.name);
                }

                parse_enum_body(index, instruction);

                if (instruction.need_update) {
                    has_updates = true;
                    instruction.version += 1;
                    source_lines[index] = replace_marker(source_lines[index], instruction.version);
                }

                instructions.push_back(std::move(instruction));
            }

            auto namespace_names = parse_namespace_names(source_lines[index]);
            for (auto const& name : namespace_names) {
                namespace_stack.push_back(NamespaceScope{name, brace_depth + 1});
            }

            brace_depth += brace_delta(source_lines[index]);
            if (brace_depth < 0) {
                brace_depth = 0;
            }
        }
    }

    void parse_enum_body(std::size_t enum_line, RawVerboseInstruction& instruction) const {
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

        throw std::runtime_error("Unterminated verbose enum class in: " + infile.string());
    }

    std::filesystem::path infile;
    std::filesystem::path outfile;
    std::vector<std::string> source_lines;
    std::vector<RawVerboseInstruction> instructions;
    bool has_updates{false};
};

} // namespace verbose

#endif // SWIFTCODE_VERBOSE_RAW_BLOCK_H
