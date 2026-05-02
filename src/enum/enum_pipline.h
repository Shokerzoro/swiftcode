#ifndef SWIFTCODE_ENUM_PIPELINE_H
#define SWIFTCODE_ENUM_PIPELINE_H

#include "file_resolving.h"
#include "raw_block.h"
#include "mid_block.h"
#include "output_block.h"
#include "domain_sql_block.h"

#include <filesystem>
#include <utility>
#include <vector>

namespace enum_pipeline {

inline void process_enums(std::filesystem::path const& input) {
    auto files = resolve_enum_files(input);

    for (auto const& file : files) {
        RawBlock raw{file};
        if (!raw) {
            continue;
        }

        MidBlock mid{raw};
        if (!mid) {
            continue;
        }

        OutBlock out{mid};
        raw.update_source(out.updated_source());
    }
}

inline void process_enum_domains(std::filesystem::path const& contract,
                                 std::filesystem::path const& database) {
    auto files = resolve_enum_files(contract);
    std::vector<MidBlock> processed_mids;

    for (auto const& file : files) {
        RawBlock raw{file};
        MidBlock mid{raw, true};
        if (mid) {
            processed_mids.push_back(std::move(mid));
        }
    }

    DomainSqlBlock domains{contract, database, processed_mids};
    if (domains) {
        domains.update();
    }
}

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_PIPELINE_H
