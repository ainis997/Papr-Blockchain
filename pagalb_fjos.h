#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include "blokai.h"
#include "konstantos.h"

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator);

std::vector<Hash> extract_tx_ids(const std::vector<Transaction> &txs);

Hash calc_tx_root(const std::vector<Transaction> &txs);
// RVALUE PARAMETRO PERDENGIMAS
Hash calc_tx_root(std::vector<Transaction> &&txs);

Hash hash_pair(const Hash &l, const Hash &r);

bool is_valid_hash(const Hash &hash);

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

void present_candidate_info(std::vector<std::size_t> &attempts, std::vector<double> &mining_durations, std::vector<bool> &mined);