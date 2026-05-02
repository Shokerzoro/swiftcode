#include "file_resolving.h"

#include "../enum/file_resolving.h"

#include <stdexcept>
#include <string>

namespace domain {

std::vector<FilePair> resolve_domain_files(std::filesystem::path const& contract,
                                           std::filesystem::path const& database) {
    if (!std::filesystem::exists(contract)) {
        throw std::runtime_error("Contract source path does not exist: " + contract.string());
    }

    std::vector<FilePair> files;
    for (auto const& source : enum_pipeline::resolve_enum_files(contract)) {
        auto relative = std::filesystem::relative(source, contract);
        std::string subsystem = "common";
        if (relative.has_parent_path()) {
            auto first = *relative.begin();
            if (first != relative.filename()) {
                subsystem = first.string();
            }
        }

        auto output = database / subsystem / ("raw_sql_" + subsystem) / "domains.sql";
        files.emplace_back(source, output);
    }

    return files;
}

} // namespace domain
