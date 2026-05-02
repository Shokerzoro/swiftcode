#include "enum/enum_pipline.h"
#include "sql/sql_pipeline.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_usage(char const* executable) {
    std::cerr << "Usage:\n"
              << "  " << executable << " -sql <raw_sql_dir> <gen_sql_dir>\n"
              << "  " << executable << " -enum <source_dir>\n"
              << "  " << executable << " -enum-domain <contract_dir> <database_dir>\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    try {
        std::string const mode{argv[1]};

        if (mode == "-sql") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }

            sql::process_sql(std::filesystem::path{argv[2]},
                             std::filesystem::path{argv[3]});
            return 0;
        }

        if (mode == "-enum") {
            if (argc != 3) {
                print_usage(argv[0]);
                return 1;
            }

            enum_pipeline::process_enums(std::filesystem::path{argv[2]});
            return 0;
        }

        if (mode == "-enum-domain") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }

            enum_pipeline::process_enum_domains(std::filesystem::path{argv[2]},
                                                std::filesystem::path{argv[3]});
            return 0;
        }

        print_usage(argv[0]);
        return 1;
    } catch (std::exception const& e) {
        std::cerr << "SwiftCode error: " << e.what() << '\n';
        return 1;
    }
}
