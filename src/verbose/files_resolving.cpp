#include "file_resolving.h"

#include <stdexcept>

namespace verbose {

namespace {

bool is_source_extension(std::filesystem::path const& path) {
    auto extension = path.extension().string();
    return extension == ".h" || extension == ".hpp" ||
           extension == ".hh" || extension == ".cpp" ||
           extension == ".cc" || extension == ".cxx";
}

} // namespace

std::vector<FilePair> resolve_verbose_files(std::filesystem::path const& input,
                                            std::filesystem::path const& output) {
    if (!std::filesystem::exists(input)) {
        throw std::runtime_error("Verbose input directory does not exist: " + input.string());
    }
    if (!std::filesystem::is_directory(input)) {
        throw std::runtime_error("Verbose input path is not a directory: " + input.string());
    }

    std::filesystem::create_directories(output);

    std::vector<FilePair> files;
    for (auto const& entry : std::filesystem::recursive_directory_iterator(input)) {
        if (!entry.is_regular_file() || !is_source_extension(entry.path())) {
            continue;
        }

        auto relative = std::filesystem::relative(entry.path(), input);
        auto generated = output / relative;
        if (std::filesystem::absolute(generated) == std::filesystem::absolute(entry.path())) {
            throw std::runtime_error("Verbose output file would overwrite input source: " +
                                     entry.path().string());
        }
        std::filesystem::create_directories(generated.parent_path());
        files.emplace_back(entry.path(), generated);
    }

    return files;
}

} // namespace verbose
