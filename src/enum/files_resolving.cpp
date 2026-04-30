#include "file_resolving.h"

#include <stdexcept>

namespace enum_pipeline {

namespace {

bool is_source_extension(std::filesystem::path const& path) {
    auto extension = path.extension().string();
    return extension == ".h" || extension == ".hpp" ||
           extension == ".hh" || extension == ".cpp" ||
           extension == ".cc" || extension == ".cxx";
}

} // namespace

std::vector<std::filesystem::path> resolve_enum_files(std::filesystem::path const& input) {
    if (!std::filesystem::exists(input)) {
        throw std::runtime_error("Enum source path does not exist: " + input.string());
    }

    std::vector<std::filesystem::path> files;
    if (std::filesystem::is_regular_file(input)) {
        if (is_source_extension(input)) {
            files.push_back(input);
        }
        return files;
    }

    if (!std::filesystem::is_directory(input)) {
        throw std::runtime_error("Enum source path is not a file or directory: " + input.string());
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && is_source_extension(entry.path())) {
            files.push_back(entry.path());
        }
    }

    return files;
}

} // namespace enum_pipeline
