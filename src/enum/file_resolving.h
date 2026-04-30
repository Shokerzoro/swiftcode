#ifndef SWIFTCODE_ENUM_FILE_RESOLVING_H
#define SWIFTCODE_ENUM_FILE_RESOLVING_H

#include <filesystem>
#include <vector>

namespace enum_pipeline {

std::vector<std::filesystem::path> resolve_enum_files(std::filesystem::path const& input);

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_FILE_RESOLVING_H
