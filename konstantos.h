#pragma once

#include <vector>
#include <cstdint>

using Hash = std::vector<std::uint8_t>; // dīgi muzīlai boūt std::array<std::uint8_t, 32>
using Money = std::uint32_t;

// KONSTANTOS

constexpr std::uint32_t Version = 1;
constexpr std::uint32_t Difficulty = 3; // pirmi 3 - nuliai