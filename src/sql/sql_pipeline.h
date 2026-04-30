//
// Created by zver on 30.04.2026.
//

#ifndef CODEGEN_SQL_PIPELINE_H
#define CODEGEN_SQL_PIPELINE_H

#include "raw_block.h"
#include "mid_block.h"
#include "output_block.h"

#include <filesystem>
#include <vector>
#include <pair>

namespace sql {

    void process_sql(std::filesystem::path const& input,
                    std::filesystem::path const& output) {

        // First processing dirs, creating them, creating if not exists
        // And create map for following processing (first input, second output)
        std::vector<std::pair<std::filesystem::path, std::filesystem::path>> iofiles;

        // Create raw blocks from input files
        std::vector<RawBlock> raw_blocks;

        // Create mid blocks from raw blocks
        std::vector<MidBlock> mid_blocks;

        // Create out blocks from mid blocks
        std::vector<OutBlock> out_blocks;

        //implements out blocks to streams
        for (auto& block : raw_blocks) {
            block.update();
        }

    };

} // namespace sql

#endif //CODEGEN_SQL_PIPELINE_H