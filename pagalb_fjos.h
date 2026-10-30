#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include "blokai.h"
#include "konstantos.h"

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator);

Hash calc_tx_root(std::vector<Transaction> &txs);

bool is_valid_hash(Hash hash);

std::string to_hex(const std::vector<std::uint8_t> &baitai);
std::string to_hex(std::vector<std::uint8_t> &&baitai);

void present_mining_data(std::size_t attempts, double duration);
void present_overall_data(
    std::size_t num_users,
    std::size_t num_accepted_blocks,
    std::size_t num_accepted_txs,
    std::size_t num_removed_invalid_txs,
    std::size_t num_remaining_txs,
    Money initial_money_supply,
    Money final_money_supply,
    double total_duration);