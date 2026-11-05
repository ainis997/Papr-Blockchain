#include "testavimas.h"
#include "pagalb_fjos.h"

#include <iostream>

// Merkle Tree testai

void check_test(const std::string &name, bool passed)
{
    std::cout << (passed ? "[PASS] " : "[FAIL] ")
              << name << '\n';
}

void test_merkle_tree_sizes()
{
    std::vector<Transaction> txs = generate_test_transactions(5);
    std::vector<Hash> ids = extract_tx_ids(txs);

    // 1 transakcija
    MerkleTree tree1({ids[0]});
    check_test("Merkle tree: 1 transaction",
               tree1.root() == ids[0]);

    // 2 transakcijos
    MerkleTree tree2({ids[0], ids[1]});
    Hash expected2 = hash_pair(ids[0], ids[1]);

    check_test("Merkle tree: 2 transactions",
               tree2.root() == expected2);

    // 3 transakcijos
    MerkleTree tree3({ids[0], ids[1], ids[2]});

    Hash left = hash_pair(ids[0], ids[1]);
    Hash right = hash_pair(ids[2], ids[2]);
    Hash expected3 = hash_pair(left, right);

    check_test("Merkle tree: 3 transactions",
               tree3.root() == expected3);

    // 5 transakcijos
    MerkleTree tree5({ids[0], ids[1], ids[2], ids[3], ids[4]});

    // Pirmasis lygis
    Hash node1 = hash_pair(ids[0], ids[1]);
    Hash node2 = hash_pair(ids[2], ids[3]);
    Hash node3 = hash_pair(ids[4], ids[4]);

    // Antrasis lygis
    Hash node4 = hash_pair(node1, node2);
    Hash node5 = hash_pair(node3, node3);

    // Šaknis
    Hash expected5 = hash_pair(node4, node5);

    check_test("Merkle tree: 5 transactions",
               tree5.root() == expected5);
}

void test_transaction_content_change()
{
    std::vector<Transaction> original = generate_test_transactions(3);

    Hash original_id = original[0].transaction_id();
    Hash original_root = calc_tx_root(original);

    std::vector<Transaction> modified = original;

    // pakeičiam pirmosios transakcijos turinį ir perskaičiuojam jos ID
    modified[0] = create_modified_transaction(modified[0]);

    Hash modified_id = modified[0].transaction_id();
    Hash modified_root = calc_tx_root(modified);

    check_test("Transaction ID changes after content modification",
               original_id != modified_id);

    check_test("Merkle root changes after content modification",
               original_root != modified_root);
}

void test_transaction_order_change()
{
    std::vector<Transaction> original = generate_test_transactions(3);

    Hash original_root = calc_tx_root(original);

    std::vector<Transaction> reordered = original;
    std::swap(reordered[0], reordered[1]);

    Hash reordered_root = calc_tx_root(reordered);

    check_test("Merkle root changes after transaction reordering",
               original_root != reordered_root);
}

void run_merkle_tests()
{
    std::cout << "\n=== MERKLE TREE TESTS ===\n\n";

    test_merkle_tree_sizes();
    test_transaction_content_change();
    test_transaction_order_change();
}

// pagalbinės funkcijos

std::vector<Transaction> generate_test_transactions(std::size_t n)
{
    std::vector<Transaction> txs;
    txs.reserve(n);

    std::mt19937 generator(123456);
    std::uniform_int_distribution<Money> amount_dist(10, 100);

    for (std::size_t i = 0; i < n; ++i)
    {
        Transaction tx("User1", "User2", amount_dist(generator), i);
        txs.push_back(tx);
    }
    return txs;
}

Transaction create_modified_transaction(const Transaction &tx)
{
    return Transaction(tx.sender(), tx.receiver(), tx.amount() + 1, tx.sequence());
}