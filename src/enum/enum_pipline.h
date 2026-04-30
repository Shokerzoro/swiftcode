#ifndef SWIFTCODE_ENUM_PIPELINE_H
#define SWIFTCODE_ENUM_PIPELINE_H

#include "file_resolving.h"
#include "raw_block.h"
#include "mid_block.h"
#include "output_block.h"

#include <filesystem>

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

} // namespace enum_pipeline

#endif // SWIFTCODE_ENUM_PIPELINE_H
