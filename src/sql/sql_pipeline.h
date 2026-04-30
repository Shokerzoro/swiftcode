//
// Created by zver on 30.04.2026.
//

#ifndef SWIFTCODE_SQL_PIPELINE_H
#define SWIFTCODE_SQL_PIPELINE_H

#include "file_resolving.h"
#include "raw_block.h"
#include "mid_block.h"
#include "output_block.h"

#include <filesystem>

namespace sql {

inline void process_sql(std::filesystem::path const& input,
                        std::filesystem::path const& output) {
    auto files = resolve_sql_files(input, output);

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
        out.update();
        raw.update();
    }
}

} // namespace sql

#endif // SWIFTCODE_SQL_PIPELINE_H
