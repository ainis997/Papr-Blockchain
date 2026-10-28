#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include "klases.h"

std::vector<User> generate_users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator);

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator);