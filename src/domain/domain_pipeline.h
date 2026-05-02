#ifndef SWIFTCODE_DOMAIN_PIPELINE_H
#define SWIFTCODE_DOMAIN_PIPELINE_H

#include "file_resolving.h"
#include "raw_block.h"
#include "mid_block.h"
#include "output_block.h"

#include <filesystem>
#include <utility>
#include <vector>

namespace domain {

inline void process_domains(std::filesystem::path const& input,
                            std::filesystem::path const& output) {
    std::vector<FilePair> iofiles = resolve_domain_files(input, output);

    std::vector<RawBlock> raw_blocks;
    for (auto const& file : iofiles) {
        RawBlock raw{file};
        if (raw) {
            raw_blocks.push_back(std::move(raw));
        }
    }

    std::vector<MidBlock> mid_blocks;
    for (auto const& raw : raw_blocks) {
        MidBlock mid{raw};
        if (mid) {
            mid_blocks.push_back(std::move(mid));
        }
    }

    std::vector<OutBlock> out_blocks;
    if (!mid_blocks.empty()) {
        OutBlock out{mid_blocks};
        if (out) {
            out_blocks.push_back(std::move(out));
        }
    }

    for (auto const& block : out_blocks) {
        block.update();
    }

    for (auto const& block : raw_blocks) {
        block.update();
    }
}

} // namespace domain

#endif // SWIFTCODE_DOMAIN_PIPELINE_H
