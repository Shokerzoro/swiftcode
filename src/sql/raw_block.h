//
// Created by zver on 30.04.2026.
//

#ifndef CODEGEN_RawBlock_H
#define CODEGEN_RawBlock_H

#include <string>
#include <fstream>
#include <filesystem>

namespace sql {

    // Forward declaration
    class MidBlock;

    // Nullable class for sql blocks detacting
    class RawBlock {
    public:
        // Gets filestram data and processes
        RawBlock(std::pair<std::filesystem::path, std::filesystem::path> pathpairs)
            : outfile{pathpairs.second} {

            std::ifstream& input{pathpairs.first};

            std::string tmp;
            std::getline(in, part, "-- CodeGen");

            // Found block entry
            if (!tmp.empty()) {
                // Add tmp to raw_strings

                // Parse until sybol ';'
                // in parallel emplaceing strings into raw_strings
            }

        };

        // Check result
        explicit operator bool() const noexcept {
            return result;
        }
    private:
        // For private data access
        friend class MidBlock;

        // Data
        std::list<std::string> raw_strings;
        std::filesystem::path outfile;

        // Результат
        bool result{false};
    };

} // namespace sql

#endif //CODEGEN_RawBlock_H