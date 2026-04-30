#include "file_resolving.h"

#include <stdexcept>

namespace sql {

std::vector<FilePair> resolve_sql_files(std::filesystem::path const& input,
                                        std::filesystem::path const& output) {
    if (!std::filesystem::exists(input)) {
        throw std::runtime_error("Input SQL directory does not exist: " + input.string());
    }
    if (!std::filesystem::is_directory(input)) {
        throw std::runtime_error("Input SQL path is not a directory: " + input.string());
    }

    std::filesystem::create_directories(output);

    std::vector<FilePair> files;
    for (auto const& entry : std::filesystem::recursive_directory_iterator(input)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".sql") {
            continue;
        }

        auto relative = std::filesystem::relative(entry.path(), input);
        auto generated = output / relative;
        generated.replace_extension(".h");
        std::filesystem::create_directories(generated.parent_path());

        files.emplace_back(entry.path(), generated);
    }

    return files;
}

} // namespace sql
