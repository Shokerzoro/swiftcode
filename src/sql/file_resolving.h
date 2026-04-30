//
// Created by zver on 30.04.2026.
//

#ifndef SWIFTCODE_SQL_FILE_RESOLVING_H
#define SWIFTCODE_SQL_FILE_RESOLVING_H

#include <filesystem>
#include <utility>
#include <vector>

namespace sql {

using FilePair = std::pair<std::filesystem::path, std::filesystem::path>;

std::vector<FilePair> resolve_sql_files(std::filesystem::path const& input,
                                        std::filesystem::path const& output);

} // namespace sql

#endif // SWIFTCODE_SQL_FILE_RESOLVING_H
