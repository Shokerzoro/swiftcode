#ifndef SWIFTCODE_VERBOSE_PIPELINE_H
#define SWIFTCODE_VERBOSE_PIPELINE_H

#include "../flags.h"
#include "file_resolving.h"
#include "mid_block.h"
#include "output_block.h"
#include "raw_block.h"

#include <utility>
#include <vector>

namespace verbose {

inline void process_verbose(VerboseFlags const& flags) {
    std::vector<FilePair> iofiles = resolve_verbose_files(flags.input_dir, flags.output_dir);

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
    for (auto const& mid : mid_blocks) {
        OutBlock out{mid, flags.generate_qdebug};
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

} // namespace verbose

#endif // SWIFTCODE_VERBOSE_PIPELINE_H
