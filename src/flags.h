//
// Created by zver on 14.05.2026.
//

#ifndef SWIFTCODE_FLAGS_H
#define SWIFTCODE_FLAGS_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

enum class CallType : std::uint8_t {
    UNKNOWN = 0,
    DOMAIN = 1,
    ENUM = 2,
    SQL = 3,
    VERBOSE = 4
};

struct Flags {
    explicit Flags(CallType call) : type(call) {}
    virtual ~Flags() = default;

    CallType type;
};

struct VerboseFlags final : Flags {
    VerboseFlags(std::filesystem::path input,
                 std::filesystem::path output,
                 bool qdebug_enabled)
        : Flags(CallType::VERBOSE),
          input_dir(std::move(input)),
          output_dir(std::move(output)),
          generate_qdebug(qdebug_enabled) {}

    std::filesystem::path input_dir;
    std::filesystem::path output_dir;
    bool generate_qdebug{false};
};

#endif // SWIFTCODE_FLAGS_H
