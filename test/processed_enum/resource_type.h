#pragma once

#include <array>
#include <string>
#include <utility>

// CodeGen: ResourceTypePairs version 1. Need update (yes/no): no
enum class ResourceType {
    Employees = 10,
    Production = 11,
    Integration = 12,
};

// CodeGen from ResourceType version 1.
inline std::array<std::pair<int, std::string>, 3> ResourceTypePairs = {{
    {static_cast<int>(ResourceType::Employees), "Employees"},
    {static_cast<int>(ResourceType::Production), "Production"},
    {static_cast<int>(ResourceType::Integration), "Integration"},
}};
