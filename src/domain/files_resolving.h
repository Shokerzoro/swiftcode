#ifndef SWIFTCODE_DOMAIN_FILE_RESOLVING_H
#define SWIFTCODE_DOMAIN_FILE_RESOLVING_H

#include <filesystem>
#include <utility>
#include <vector>

namespace domain {

using FilePair = std::pair<std::filesystem::path, std::filesystem::path>;

std::vector<FilePair> resolve_domain_files(std::filesystem::path const& contract,
                                           std::filesystem::path const& database);

} // namespace domain

#endif // SWIFTCODE_DOMAIN_FILE_RESOLVING_H
