#ifndef SWIFTCODE_ENUM_DOMAIN_SQL_BLOCK_H
#define SWIFTCODE_ENUM_DOMAIN_SQL_BLOCK_H

#include "mid_block.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace enum_pipeline {

struct DomainSqlValue {
    std::string id;
    std::string text;
};

struct DomainSqlInstruction {
    std::filesystem::path output_file;
    std::string table_name;
    std::vector<DomainSqlValue> values;
};

class DomainSqlBlock {
public:
    DomainSqlBlock(std::filesystem::path contract_root,
                   std::filesystem::path database_root,
                   std::vector<MidBlock> const& mids)
        : contract_root{std::move(contract_root)}, database_root{std::move(database_root)} {
        for (auto const& mid : mids) {
            collect(mid);
        }
        for (auto& [key, instruction] : grouped) {
            (void)key;
            instructions.push_back(std::move(instruction));
        }
    }

    explicit operator bool() const noexcept {
        return !instructions.empty();
    }

    void update() const {
        std::map<std::filesystem::path, std::vector<DomainSqlInstruction>> by_file;
        for (auto const& instruction : instructions) {
            by_file[instruction.output_file].push_back(instruction);
        }

        for (auto& [file, file_instructions] : by_file) {
            std::sort(file_instructions.begin(), file_instructions.end(),
                      [](DomainSqlInstruction const& left, DomainSqlInstruction const& right) {
                          return left.table_name < right.table_name;
                      });
            update_file(file, file_instructions);
        }
    }

private:
    struct ExistingBlock {
        std::string marker;
        int version{0};
        std::vector<std::string> sql_lines;
    };

    static std::string to_upper_identifier(std::string value) {
        for (auto& ch : value) {
            auto uch = static_cast<unsigned char>(ch);
            if (std::isalnum(uch) != 0) {
                ch = static_cast<char>(std::toupper(uch));
            } else {
                ch = '_';
            }
        }
        return value;
    }

    static std::string sql_string(std::string value) {
        std::string escaped;
        escaped.reserve(value.size());
        for (auto ch : value) {
            escaped.push_back(ch);
            if (ch == '\'') {
                escaped.push_back('\'');
            }
        }
        return "'" + escaped + "'";
    }

    static bool is_sql_codegen_marker(std::string const& line) {
        auto trimmed = trim_copy(line);
        return starts_with(trimmed, "-- CodeGen need for ") ||
               starts_with(trimmed, "-- CodeGen: ");
    }

    static bool ends_sql_instruction(std::string const& line) {
        auto trimmed = trim_copy(line);
        return !trimmed.empty() && trimmed.back() == ';';
    }

    static std::vector<std::string> read_lines(std::filesystem::path const& file) {
        std::vector<std::string> lines;
        std::ifstream input{file};
        if (!input) {
            return lines;
        }

        std::string line;
        while (std::getline(input, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    static std::string marker_name(std::string const& marker) {
        auto trimmed = trim_copy(marker);
        std::string const need_prefix = "-- CodeGen need for ";
        if (starts_with(trimmed, need_prefix)) {
            auto name = trim_copy(trimmed.substr(need_prefix.size()));
            if (!name.empty() && name.back() == '.') {
                name.pop_back();
            }
            return name;
        }

        std::string const version_prefix = "-- CodeGen: ";
        if (!starts_with(trimmed, version_prefix)) {
            return {};
        }

        auto rest = trimmed.substr(version_prefix.size());
        auto version_pos = rest.find(" version ");
        return version_pos == std::string::npos ? trim_copy(rest) : trim_copy(rest.substr(0, version_pos));
    }

    static int marker_version(std::string const& marker) {
        auto trimmed = trim_copy(marker);
        std::string const version_prefix = "-- CodeGen: ";
        if (!starts_with(trimmed, version_prefix)) {
            return 0;
        }

        auto rest = trimmed.substr(version_prefix.size());
        auto version_pos = rest.find(" version ");
        if (version_pos == std::string::npos) {
            return 0;
        }
        auto after_version = rest.substr(version_pos + std::string{" version "}.size());
        auto dot_pos = after_version.find('.');
        if (dot_pos == std::string::npos) {
            return 0;
        }
        return std::stoi(after_version.substr(0, dot_pos));
    }

    static std::vector<std::string> make_create_sql(DomainSqlInstruction const& instruction) {
        return {
            "CREATE TABLE IF NOT EXISTS " + instruction.table_name + " (",
            "    id   INTEGER PRIMARY KEY,",
            "    enum TEXT NOT NULL UNIQUE",
            ");"
        };
    }

    static std::vector<std::string> make_fill_sql(DomainSqlInstruction const& instruction) {
        std::vector<std::string> lines;
        lines.push_back("INSERT OR IGNORE INTO " + instruction.table_name + " (id, enum) VALUES");
        for (std::size_t index = 0; index < instruction.values.size(); ++index) {
            auto const& value = instruction.values[index];
            auto suffix = index + 1U == instruction.values.size() ? ";" : ",";
            lines.push_back("    (" + value.id + ", " + sql_string(value.text) + ")" + suffix);
        }
        return lines;
    }

    static std::string make_need_marker(std::string const& name) {
        return "-- CodeGen need for " + name + ".";
    }

    static std::string make_update_marker(std::string const& name, int version) {
        return "-- CodeGen: " + name + " version " + std::to_string(version) +
               ". Need update (yes/no): yes";
    }

    static bool lines_equal(std::vector<std::string> const& left,
                            std::vector<std::string> const& right) {
        if (left.size() != right.size()) {
            return false;
        }
        for (std::size_t index = 0; index < left.size(); ++index) {
            if (trim_copy(left[index]) != trim_copy(right[index])) {
                return false;
            }
        }
        return true;
    }

    static std::vector<std::string> block_lines(std::string const& name,
                                                std::vector<std::string> const& sql_lines,
                                                std::unordered_map<std::string, ExistingBlock> const& existing) {
        std::vector<std::string> lines;
        auto found = existing.find(name);
        if (found != existing.end() && lines_equal(found->second.sql_lines, sql_lines)) {
            lines.push_back(found->second.marker);
        } else if (found != existing.end() && starts_with(trim_copy(found->second.marker), "-- CodeGen: ")) {
            lines.push_back(make_update_marker(name, found->second.version));
        } else {
            lines.push_back(make_need_marker(name));
        }

        lines.insert(lines.end(), sql_lines.begin(), sql_lines.end());
        return lines;
    }

    static void collect_existing(std::vector<std::string> const& content,
                                 std::unordered_map<std::string, ExistingBlock>& existing) {
        for (std::size_t index = 0; index < content.size(); ++index) {
            if (!is_sql_codegen_marker(content[index])) {
                continue;
            }

            ExistingBlock block;
            block.marker = content[index];
            block.version = marker_version(block.marker);
            auto name = marker_name(block.marker);
            for (std::size_t line_index = index + 1; line_index < content.size(); ++line_index) {
                if (is_sql_codegen_marker(content[line_index])) {
                    break;
                }
                block.sql_lines.push_back(content[line_index]);
                if (ends_sql_instruction(content[line_index])) {
                    break;
                }
            }
            existing.emplace(std::move(name), std::move(block));
        }
    }

    static void remove_generated_blocks(std::vector<std::string>& content,
                                        std::unordered_set<std::string> const& names) {
        std::vector<std::string> filtered;
        for (std::size_t index = 0; index < content.size();) {
            if (!is_sql_codegen_marker(content[index]) || names.find(marker_name(content[index])) == names.end()) {
                filtered.push_back(content[index]);
                ++index;
                continue;
            }

            ++index;
            while (index < content.size() && !is_sql_codegen_marker(content[index])) {
                auto done = ends_sql_instruction(content[index]);
                ++index;
                if (done) {
                    break;
                }
            }
        }

        while (!filtered.empty() && filtered.back().empty()) {
            filtered.pop_back();
        }
        content = std::move(filtered);
    }

    void collect(MidBlock const& mid) {
        for (auto const& enum_instruction : mid.instructions) {
            if (!enum_instruction.has_domain()) {
                continue;
            }

            auto output_file = domain_file_for(mid.infile);
            auto key = output_file.string() + "|" + enum_instruction.domain_table;
            auto& domain = grouped[key];
            if (domain.table_name.empty()) {
                domain.output_file = std::move(output_file);
                domain.table_name = enum_instruction.domain_table;
            }
            for (auto const& value : enum_instruction.values) {
                domain.values.push_back(DomainSqlValue{value.sql_id, value.text});
            }
        }
    }

    std::filesystem::path domain_file_for(std::filesystem::path const& source_file) const {
        auto relative = std::filesystem::relative(source_file, contract_root);
        std::string subsystem = "common";
        if (relative.has_parent_path()) {
            auto first = *relative.begin();
            if (first != relative.filename()) {
                subsystem = first.string();
            }
        }

        return database_root / subsystem / ("raw_sql_" + subsystem) / "domains.sql";
    }

    void update_file(std::filesystem::path const& file,
                     std::vector<DomainSqlInstruction> const& file_instructions) const {
        auto content = read_lines(file);

        std::unordered_map<std::string, ExistingBlock> existing;
        collect_existing(content, existing);

        std::unordered_set<std::string> names;
        for (auto const& instruction : file_instructions) {
            auto upper_table = to_upper_identifier(instruction.table_name);
            names.insert("CREATE_" + upper_table + "_SQL");
            names.insert("FILL_" + upper_table + "_SQL");
        }
        remove_generated_blocks(content, names);

        if (!content.empty()) {
            content.push_back("");
        }

        for (auto const& instruction : file_instructions) {
            auto upper_table = to_upper_identifier(instruction.table_name);
            auto create_name = "CREATE_" + upper_table + "_SQL";
            auto fill_name = "FILL_" + upper_table + "_SQL";

            auto create_block = block_lines(create_name, make_create_sql(instruction), existing);
            content.insert(content.end(), create_block.begin(), create_block.end());
            content.push_back("");

            auto fill_block = block_lines(fill_name, make_fill_sql(instruction), existing);
            content.insert(content.end(), fill_block.begin(), fill_block.end());
            content.push_back("");
        }

        while (!content.empty() && content.back().empty()) {
            content.pop_back();
        }

        std::filesystem::create_directories(file.parent_path());
        std::ofstream output{file, std::ios::trunc};
        if (!output) {
            throw std::runtime_error("Cannot update enum domain SQL file: " + file.string());
        }

        for (auto const& line : content) {
            output << line << '\n';
        }
    }

    std::filesystem::path contract_root;
    std::filesystem::path database_root;
    std::map<std::string, DomainSqlInstruction> grouped;
    std::vector<DomainSqlInstruction> instructions;
};

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_DOMAIN_SQL_BLOCK_H
