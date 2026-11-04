#pragma once

#include "transakcijos.h"

// Merkle Tree testai

void check_test(const std::string &name, bool passed);
void test_merkle_tree_sizes();
void test_transaction_content_change();
void test_transaction_order_change();
void run_merkle_tests();

// pagalb fjos
std::vector<Transaction> generate_test_transactions(std::size_t n);
Transaction create_modified_transaction(const Transaction &tx);