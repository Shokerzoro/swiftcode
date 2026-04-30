//
// Created by zver on 30.04.2026.
//

#ifndef CODEGEN_OutBlock_H
#define CODEGEN_OutBlock_H

#include <string>
#include <fstream>
#include "mid_view.h"

namespace sql {

    // Class for output
    class OutBlock {
    public:
        OutBlock(MidBlock& mid) : outfile(mid.outfile), version{mid.version++} {
            // Turnes midblock into strings
        }

        // And knows how to be emplaced in filestram
        void update() {
            // Look for named block with same name and replaces it

            // If no such block found adds it
        }
    private:
        // Main data
        std::filesystem::path outfile;
        std::string name;
        int version;

        // Raw data
        std::list<std::string> outputstrings; // Ready to input in file static constexpr const char* {NAME} = R" ... " /n R" ... " /n ... /n R" ...";
    };

} // namespace sql

#endif //CODEGEN_OutBlock_H