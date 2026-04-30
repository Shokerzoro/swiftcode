//
// Created by zver on 30.04.2026.
//

#ifndef CODEGEN_MID_VIEW_H
#define CODEGEN_MID_VIEW_H

#include "raw_block.h"

namespace sql {

    // Forward declaration
    class OutBlock;

    class MidBlock {
    public:
        // Procceses to normalized view
        MidBlock(RawBlock& raw) : outfile(raw.outfile) {

            // Procceces string by string
            // First string main data extaction

            // Other string extraction & tokenize if need update

        };

        //Can be nullable if raw_block based on needs no update
        explicit operator bool() {
            return need_update;
        }

    private:
        // For main data access
        friend class OutBlock;

        // Main data
        bool need_update{true};
        std::string name;
        int version{0}; // defauld if first generated and no other mentioned (OutBlock will increment)
        std::filesystem::path outfile;

        // Vaulted ordered strings with tokens to be replaced
        std::list<std::pair<std::string, std::vector<std::string>>> strings_tokenized;
    };

} // namespace sql

#endif //CODEGEN_MID_VIEW_H