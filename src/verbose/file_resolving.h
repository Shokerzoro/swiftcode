#ifndef SWIFTCODE_VERBOSE_FILE_RESOLVING_H
#define SWIFTCODE_VERBOSE_FILE_RESOLVING_H

#include <filesystem>
#include <utility>
#include <vector>

namespace verbose {

using FilePair = std::pair<std::filesystem::path, std::filesystem::path>;

std::vector<FilePair> resolve_verbose_files(std::filesystem::path const& input,
                                            std::filesystem::path const& output);

} // namespace verbose

#endif // SWIFTCODE_VERBOSE_FILE_RESOLVING_H
