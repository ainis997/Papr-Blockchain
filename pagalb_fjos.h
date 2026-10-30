#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include "blokai.h"
#include "konstantos.h"

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator);

Hash calc_tx_root(std::vector<Transaction> &txs);

bool is_valid_hash(Hash hash);

std::string to_hex(std::vector<std::uint8_t> &baitai);
std::string to_hex(std::vector<std::uint8_t> &&baitai);

void present_mining_data(std::size_t attempts, double duration);