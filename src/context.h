//
// Created by zver on 14.05.2026.
//

#ifndef SWIFTCODE_CONTEXT_H
#define SWIFTCODE_CONTEXT_H

#include "flags.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

class Context {
public:
    void init(int argc, char* argv[]) {
        wrong_args = true;
        call_type = CallType::UNKNOWN;
        flags.reset();
        executable = "swiftcode";

        if (argc > 0 && argv[0] != nullptr) {
            std::filesystem::path executable_path{argv[0]};
            auto filename = executable_path.filename().string();
            if (!filename.empty()) {
                executable = std::move(filename);
            }
        }

        if (argc < 2 || argv[1] == nullptr) {
            return;
        }

        std::string const mode{argv[1]};
        if (mode == "--verbose") {
            init_verbose(argc, argv);
            return;
        }

        if (mode == "-sql") {
            call_type = CallType::SQL;
            wrong_args = argc != 4;
            return;
        }

        if (mode == "-enum") {
            call_type = CallType::ENUM;
            wrong_args = argc != 3;
            return;
        }

        if (mode == "-enum-domain") {
            call_type = CallType::DOMAIN;
            wrong_args = argc != 4;
            return;
        }
    }

    CallType get_call_type() const { return call_type; }
    std::shared_ptr<Flags> const& get_flags() const { return flags; }
    explicit operator bool() const { return wrong_args; }

    void print_usage() const {
        std::cerr << "Usage:\n"
                  << "  " << executable << " -sql <raw_sql_dir> <gen_sql_dir> [deprecated]\n"
                  << "  " << executable << " -enum <source_dir> [deprecated]\n"
                  << "  " << executable << " -enum-domain <contract_dir> <database_dir> [deprecated]\n"
                  << "  " << executable << " --verbose <inputdir> <outputdir> [-qdebug]\n";
    }

private:
    void init_verbose(int argc, char* argv[]) {
        call_type = CallType::VERBOSE;

        if (argc != 4 && argc != 5) {
            return;
        }

        bool generate_qdebug = false;
        if (argc == 5) {
            if (argv[4] == nullptr || std::string{argv[4]} != "-qdebug") {
                return;
            }
            generate_qdebug = true;
        }

        flags = std::make_shared<VerboseFlags>(
            std::filesystem::path{argv[2]},
            std::filesystem::path{argv[3]},
            generate_qdebug);
        wrong_args = false;
    }

    CallType call_type{CallType::UNKNOWN};
    std::string executable{"swiftcode"};
    bool wrong_args{true};
    std::shared_ptr<Flags> flags;
};

#endif // SWIFTCODE_CONTEXT_H
