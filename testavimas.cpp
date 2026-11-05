// #include "testavimas.h"
// #include "pagalb_fjos.h"

// #include <iostream>

// // Merkle Tree testai

// void check_test(const std::string &name, bool passed)
// {
//     std::cout << (passed ? "[PASS] " : "[FAIL] ")
//               << name << '\n';
// }

// void test_merkle_tree_sizes()
// {
//     std::vector<Transaction> txs = generate_test_transactions(5);
//     std::vector<Hash> ids = extract_tx_ids(txs);

//     // 1 transakcija
//     MerkleTree tree1({ids[0]});
//     check_test("Merkle tree: 1 transaction",
//                tree1.root() == ids[0]);

//     // 2 transakcijos
//     MerkleTree tree2({ids[0], ids[1]});
//     Hash expected2 = hash_pair(ids[0], ids[1]);

//     check_test("Merkle tree: 2 transactions",
//                tree2.root() == expected2);

//     // 3 transakcijos
//     MerkleTree tree3({ids[0], ids[1], ids[2]});

//     Hash left = hash_pair(ids[0], ids[1]);
//     Hash right = hash_pair(ids[2], ids[2]);
//     Hash expected3 = hash_pair(left, right);

//     check_test("Merkle tree: 3 transactions",
//                tree3.root() == expected3);

//     // 5 transakcijos
//     MerkleTree tree5({ids[0], ids[1], ids[2], ids[3], ids[4]});

//     // Pirmasis lygis
//     Hash node1 = hash_pair(ids[0], ids[1]);
//     Hash node2 = hash_pair(ids[2], ids[3]);
//     Hash node3 = hash_pair(ids[4], ids[4]);

//     // Antrasis lygis
//     Hash node4 = hash_pair(node1, node2);
//     Hash node5 = hash_pair(node3, node3);

//     // Šaknis
//     Hash expected5 = hash_pair(node4, node5);

//     check_test("Merkle tree: 5 transactions",
//                tree5.root() == expected5);
// }

// void test_transaction_content_change()
// {
//     std::vector<Transaction> original = generate_test_transactions(3);

//     Hash original_id = original[0].transaction_id();
//     Hash original_root = calc_tx_root(original);

//     std::vector<Transaction> modified = original;

//     // pakeičiam pirmosios transakcijos turinį ir perskaičiuojam jos ID
//     modified[0] = create_modified_transaction(modified[0]);

//     Hash modified_id = modified[0].transaction_id();
//     Hash modified_root = calc_tx_root(modified);

//     check_test("Transaction ID changes after content modification",
//                original_id != modified_id);

//     check_test("Merkle root changes after content modification",
//                original_root != modified_root);
// }

// void test_transaction_order_change()
// {
//     std::vector<Transaction> original = generate_test_transactions(3);

//     Hash original_root = calc_tx_root(original);

//     std::vector<Transaction> reordered = original;
//     std::swap(reordered[0], reordered[1]);

//     Hash reordered_root = calc_tx_root(reordered);

//     check_test("Merkle root changes after transaction reordering",
//                original_root != reordered_root);
// }

// void run_merkle_tests()
// {
//     std::cout << "\n=== MERKLE TREE TESTS ===\n\n";

//     test_merkle_tree_sizes();
//     test_transaction_content_change();
//     test_transaction_order_change();
// }

// // pagalbinės funkcijos

// std::vector<Transaction> generate_test_transactions(std::size_t n)
// {
//     std::vector<Transaction> txs;
//     txs.reserve(n);

//     std::mt19937 generator(123456);
//     std::uniform_int_distribution<Money> amount_dist(10, 100);

//     for (std::size_t i = 0; i < n; ++i)
//     {
//         Transaction tx("User1", "User2", amount_dist(generator), i);
//         txs.push_back(tx);
//     }
//     return txs;
// }

// Transaction create_modified_transaction(const Transaction &tx)
// {
//     return Transaction(tx.sender(), tx.receiver(), tx.amount() + 1, tx.sequence());
// }

#include "testavimas.h"
#include "pagalb_fjos.h"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstdint>
#include <stdexcept>

// ============================================================
// TESTAVIMO PRIEIGA PRIE PRIVACIŲ KLASIŲ LAUKŲ
// ============================================================

// Leidžia testavimo metu sugadinti transakcijos ID.
struct TransactionTestAccess
{
    static void corrupt_id(Transaction &tx)
    {
        Hash corrupted_id = tx.transaction_id();

        if (corrupted_id.empty())
            throw std::runtime_error("Transaction ID is empty.");

        corrupted_id[0] ^= 0x01;
        tx.transaction_id_ = std::move(corrupted_id);
    }
};

// Leidžia įterpti specialiai sugadintą transakciją į Mempool.
// Įprastai transakcijos turėtų patekti į Mempool per add_tx().
struct MempoolTestAccess
{
    static void insert(Mempool &mempool, const Transaction &tx)
    {
        mempool.mempool_.push_back(tx);
    }
};

// Leidžia pakeisti bloko turinį arba antraštės laukus
// jau sukūrus bloką.
struct BlockTestAccess
{
    static void replace_transaction(
        Block &block,
        std::size_t index,
        Transaction tx)
    {
        block.transactions_.at(index) = std::move(tx);
    }

    static void set_version(Block &block, std::uint32_t version)
    {
        block.header_.version_ = version;
    }

    static void set_difficulty(Block &block, std::uint32_t difficulty)
    {
        block.header_.difficulty_ = difficulty;
    }
};

// ============================================================
// BENDRI TESTAVIMO PAGALBINIAI METODAI
// ============================================================

namespace
{
    std::size_t passed_tests = 0;
    std::size_t failed_tests = 0;

    // Sukuriame tris deterministinius vartotojus.
    // Visi pradeda turėdami po 100 vienetų.
    Users make_test_users()
    {
        std::vector<User> users;
        users.reserve(3);

        std::string name_a = "Alice";
        std::string name_b = "Bob";
        std::string name_c = "Charlie";

        std::string pk_a(64, 'a');
        std::string pk_b(64, 'b');
        std::string pk_c(64, 'c');

        users.emplace_back(name_a, pk_a, static_cast<Money>(100));
        users.emplace_back(name_b, pk_b, static_cast<Money>(100));
        users.emplace_back(name_c, pk_c, static_cast<Money>(100));

        return Users(std::move(users));
    }

    // Kiekvienas testas turi atskirą vartotojų, Mempool
    // ir Blockchain egzempliorių.
    struct TestContext
    {
        Users users;
        Mempool mempool;
        Blockchain blockchain;

        TestContext()
            : users(make_test_users()),
              mempool(users),
              blockchain(users, mempool)
        {
        }

        std::string pk(std::size_t index)
        {
            return users.at(index).public_key();
        }
    };

    // Iškasame bloką. Grąžiname false, jeigu per nurodytą
    // bandymų skaičių nepavyko rasti tinkamo nonce.
    bool mine_for_test(Block &block, std::size_t max_attempts = 1'000'000)
    {
        for (std::size_t i = 0; i < max_attempts; ++i)
        {
            if (is_valid_hash(block.block_hash()))
                return true;

            block.incr_nonce();
        }

        return false;
    }

    // Po sėkmingo validavimo ir append() pritaikome transakcijas
    // tikriesiems vartotojų balansams ir pašaliname jas iš Mempool.
    //
    // Ši funkcija atitinka dabartinę tavo main() logiką:
    // Blockchain::append() pats balansų neatnaujina.
    void apply_accepted_block(
        Block &block,
        Users &users,
        Mempool &mempool)
    {
        const std::vector<Transaction> txs = block.transactions();

        for (const Transaction &tx : txs)
        {
            users.get_user_by_pk(tx.sender()).change_balance(-static_cast<int>(tx.amount()));

            users.get_user_by_pk(tx.receiver()).change_balance(static_cast<int>(tx.amount()));

            mempool.remove_tx(tx.transaction_id());
        }
    }

    Money balance_of(BalanceState &state, const std::string &public_key)
    {
        // BalanceState::get_user_balance() šiuo metu priima
        // std::string&, todėl perduodame lokalią kopiją.
        std::string key = public_key;
        return state.get_user_balance(key);
    }

    bool all_balances_equal(
        Users &users,
        const std::string &a,
        const std::string &b,
        const std::string &c,
        Money expected)
    {
        return users.get_user_by_pk(a).balance() == expected &&
               users.get_user_by_pk(b).balance() == expected &&
               users.get_user_by_pk(c).balance() == expected;
    }

    // ========================================================
    // MERKLE MEDŽIO TESTAI
    // ========================================================

    void test_merkle_tree_sizes()
    {
        const std::vector<Transaction> txs = generate_test_transactions(5);
        const std::vector<Hash> ids = extract_tx_ids(txs);

        // Vienas lapas: jis pats yra šaknis.
        MerkleTree tree1({ids[0]});

        check_test(
            "Merkle tree: 1 transaction",
            tree1.root() == ids[0]);

        // Du lapai.
        MerkleTree tree2({ids[0], ids[1]});

        const Hash expected2 = hash_pair(ids[0], ids[1]);

        check_test(
            "Merkle tree: 2 transactions",
            tree2.root() == expected2);

        // Trys lapai: paskutinis kopijuojamas.
        MerkleTree tree3({ids[0], ids[1], ids[2]});

        const Hash left3 = hash_pair(ids[0], ids[1]);
        const Hash right3 = hash_pair(ids[2], ids[2]);
        const Hash expected3 = hash_pair(left3, right3);

        check_test(
            "Merkle tree: 3 transactions",
            tree3.root() == expected3);

        // Penki lapai: tikrinama visa medžio struktūra.
        MerkleTree tree5({ids[0], ids[1], ids[2], ids[3], ids[4]});

        const Hash node1 = hash_pair(ids[0], ids[1]);
        const Hash node2 = hash_pair(ids[2], ids[3]);
        const Hash node3 = hash_pair(ids[4], ids[4]);

        const Hash node4 = hash_pair(node1, node2);
        const Hash node5 = hash_pair(node3, node3);

        const Hash expected5 = hash_pair(node4, node5);

        check_test(
            "Merkle tree: 5 transactions",
            tree5.root() == expected5);
    }

    void test_transaction_content_change()
    {
        const std::vector<Transaction> original =
            generate_test_transactions(3);

        const Hash original_id = original[0].transaction_id();
        const Hash original_root = calc_tx_root(original);

        std::vector<Transaction> modified = original;

        modified[0] = create_modified_transaction(modified[0]);

        const Hash modified_id = modified[0].transaction_id();
        const Hash modified_root = calc_tx_root(modified);

        check_test(
            "Transaction ID changes after content modification",
            original_id != modified_id);

        check_test(
            "Merkle root changes after content modification",
            original_root != modified_root);
    }

    void test_transaction_order_change()
    {
        const std::vector<Transaction> original =
            generate_test_transactions(3);

        const Hash original_root = calc_tx_root(original);

        std::vector<Transaction> reordered = original;

        std::swap(reordered[0], reordered[1]);

        const Hash reordered_root = calc_tx_root(reordered);

        check_test(
            "Merkle root changes after transaction reordering",
            original_root != reordered_root);
    }

    // ========================================================
    // TRANSAKCIJŲ IR BLOKŲ VALIDAVIMO TESTAI
    // ========================================================

    void test_valid_payment()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);
        const std::string other = ctx.pk(2);

        const Money supply_before = ctx.users.calc_money_supply();
        const std::size_t height_before = ctx.blockchain.height();

        const Money amount = 30;
        const std::size_t sequence = 1;

        const Hash id = get_tx_id(sender, receiver, amount, sequence);

        const bool added =
            ctx.mempool.add_tx(sender, receiver, amount, sequence);

        Transaction tx = ctx.mempool.get_tx_by_id(id);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool mined = mine_for_test(block);

        const BalanceState initial_state(ctx.users);

        const bool valid =
            mined && ctx.blockchain.is_block_valid(block, initial_state);

        check_test("Valid payment: added to Mempool", added);
        check_test("Valid payment: block mined", mined);
        check_test("Valid payment: block passes validation", valid);

        if (valid)
        {
            ctx.blockchain.append(block);
            apply_accepted_block(block, ctx.users, ctx.mempool);
        }

        check_test(
            "Valid payment: sender balance is 70",
            ctx.users.get_user_by_pk(sender).balance() == 70);

        check_test(
            "Valid payment: receiver balance is 130",
            ctx.users.get_user_by_pk(receiver).balance() == 130);

        check_test(
            "Valid payment: unrelated balance unchanged",
            ctx.users.get_user_by_pk(other).balance() == 100);

        check_test(
            "Valid payment: money supply unchanged",
            ctx.users.calc_money_supply() == supply_before);

        check_test(
            "Valid payment: transaction included exactly once",
            ctx.blockchain.get_num_txs() == 1);

        check_test(
            "Valid payment: transaction removed from Mempool",
            ctx.mempool.size() == 0);

        check_test(
            "Valid payment: chain height increased",
            ctx.blockchain.height() == height_before + 1);
    }

    void test_unknown_receiver()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = "unknown-public-key";

        const std::string b = ctx.pk(1);
        const std::string c = ctx.pk(2);

        const Money supply_before = ctx.users.calc_money_supply();
        const std::size_t height_before = ctx.blockchain.height();

        const Money amount = 10;
        const std::size_t sequence = 1;

        const Hash id = get_tx_id(sender, receiver, amount, sequence);

        const bool added =
            ctx.mempool.add_tx(sender, receiver, amount, sequence);

        Transaction tx = ctx.mempool.get_tx_by_id(id);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Unknown receiver: added to Mempool", added);
        check_test("Unknown receiver: block mined", mined);
        check_test("Unknown receiver: block rejected", !accepted);

        check_test(
            "Unknown receiver: transaction recorded as rejected",
            ctx.mempool.num_rejected_txs() == 1);

        check_test(
            "Unknown receiver: chain unchanged",
            ctx.blockchain.height() == height_before &&
                ctx.blockchain.get_num_txs() == 0);

        check_test(
            "Unknown receiver: balances and money supply unchanged",
            all_balances_equal(ctx.users, sender, b, c, 100) &&
                ctx.users.calc_money_supply() == supply_before);
    }

    void test_zero_amount()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);
        const std::string other = ctx.pk(2);

        const Money supply_before = ctx.users.calc_money_supply();
        const std::size_t height_before = ctx.blockchain.height();

        const Money amount = 0;
        const std::size_t sequence = 1;

        const Hash id = get_tx_id(sender, receiver, amount, sequence);

        // Dabartinis add_tx() leidžia įterpti 0 sumą.
        // Ši transakcija turi būti atmesta bloko validavimo metu.
        const bool added =
            ctx.mempool.add_tx(sender, receiver, amount, sequence);

        Transaction tx = ctx.mempool.get_tx_by_id(id);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Zero amount: added to Mempool", added);
        check_test("Zero amount: block mined", mined);
        check_test("Zero amount: block rejected", !accepted);

        check_test(
            "Zero amount: transaction recorded as rejected",
            ctx.mempool.num_rejected_txs() == 1);

        check_test(
            "Zero amount: chain unchanged",
            ctx.blockchain.height() == height_before &&
                ctx.blockchain.get_num_txs() == 0);

        check_test(
            "Zero amount: balances and supply unchanged",
            all_balances_equal(ctx.users, sender, receiver, other, 100) &&
                ctx.users.calc_money_supply() == supply_before);
    }

    void test_self_payment()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);

        const Money amount = 10;
        const std::size_t sequence = 1;

        const Hash id = get_tx_id(sender, sender, amount, sequence);

        const bool added =
            ctx.mempool.add_tx(sender, sender, amount, sequence);

        Transaction tx = ctx.mempool.get_tx_by_id(id);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Self-payment: added to Mempool", added);
        check_test("Self-payment: block mined", mined);
        check_test("Self-payment: block rejected", !accepted);

        check_test(
            "Self-payment: transaction recorded as rejected",
            ctx.mempool.num_rejected_txs() == 1);

        check_test(
            "Self-payment: balances unchanged",
            ctx.users.get_user_by_pk(sender).balance() == 100);
    }

    void test_invalid_transaction_id()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);

        Transaction tx(sender, receiver, 10, 1);

        // Sąmoningai sugadiname ID, nekeičiant kitų laukų.
        TransactionTestAccess::corrupt_id(tx);

        // Įterpiame į Mempool tiesiogiai, nes įprastas add_tx()
        // visada sukuria ID iš transakcijos laukų.
        MempoolTestAccess::insert(ctx.mempool, tx);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Invalid transaction ID: block mined", mined);
        check_test("Invalid transaction ID: block rejected", !accepted);

        check_test(
            "Invalid transaction ID: transaction recorded as rejected",
            ctx.mempool.num_rejected_txs() == 1);

        check_test(
            "Invalid transaction ID: removed from Mempool",
            ctx.mempool.size() == 0);

        check_test(
            "Invalid transaction ID: chain unchanged",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0);

        check_test(
            "Invalid transaction ID: balances unchanged",
            ctx.users.get_user_by_pk(sender).balance() == 100 &&
                ctx.users.get_user_by_pk(receiver).balance() == 100);
    }

    void test_two_payments_exceed_balance()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver1 = ctx.pk(1);
        const std::string receiver2 = ctx.pk(2);

        // Siuntėjas turi 100.
        // Pirmoji transakcija: 100 - 60 = 40.
        // Antroji transakcija: 40 - 50 < 0.
        std::vector<Transaction> txs;

        txs.emplace_back(sender, receiver1, 60, 1);
        txs.emplace_back(sender, receiver2, 50, 2);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::move(txs));

        const bool mined = mine_for_test(block);

        BalanceState original_state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, original_state);

        check_test("Overspending: block mined", mined);
        check_test("Overspending: whole block rejected", !accepted);

        // is_block_valid() gauna BalanceState kopiją.
        // Todėl ir pati perduota būsena neturi pasikeisti.
        check_test(
            "Overspending: original BalanceState unchanged",
            balance_of(original_state, sender) == 100 &&
                balance_of(original_state, receiver1) == 100 &&
                balance_of(original_state, receiver2) == 100);

        check_test(
            "Overspending: real balances and chain unchanged",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0 &&
                ctx.users.get_user_by_pk(sender).balance() == 100 &&
                ctx.users.get_user_by_pk(receiver1).balance() == 100 &&
                ctx.users.get_user_by_pk(receiver2).balance() == 100);
    }

    void test_duplicate_id_inside_block()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);

        Transaction tx(sender, receiver, 1, 1);

        // Ta pati transakcija bloke pateikiama du kartus.
        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx, tx});

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Duplicate ID in block: block mined", mined);
        check_test("Duplicate ID in block: block rejected", !accepted);

        check_test(
            "Duplicate ID in block: no transaction committed",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0 &&
                ctx.users.get_user_by_pk(sender).balance() == 100 &&
                ctx.users.get_user_by_pk(receiver).balance() == 100);
    }

    void test_confirmed_transaction_replay()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);

        const Money amount = 10;
        const std::size_t sequence = 1;

        const Hash id = get_tx_id(sender, receiver, amount, sequence);

        const bool added =
            ctx.mempool.add_tx(sender, receiver, amount, sequence);

        Transaction tx = ctx.mempool.get_tx_by_id(id);

        // Pirmas blokas su šia transakcija.
        Block first(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool first_mined = mine_for_test(first);

        BalanceState first_state(ctx.users);

        const bool first_valid =
            first_mined &&
            ctx.blockchain.is_block_valid(first, first_state);

        if (first_valid)
        {
            ctx.blockchain.append(first);
            apply_accepted_block(first, ctx.users, ctx.mempool);
        }

        const Money sender_after_first =
            ctx.users.get_user_by_pk(sender).balance();

        const Money receiver_after_first =
            ctx.users.get_user_by_pk(receiver).balance();

        const std::size_t height_after_first =
            ctx.blockchain.height();

        // Antras blokas bando įtraukti tą pačią transakciją.
        // prev_hash jau yra dabartinio paskutinio bloko hash.
        Block replay(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{tx});

        const bool replay_mined = mine_for_test(replay);

        BalanceState replay_state(ctx.users);

        const bool replay_accepted =
            replay_mined &&
            ctx.blockchain.is_block_valid(replay, replay_state);

        check_test("Replay: transaction added to Mempool", added);
        check_test(
            "Replay: original transaction accepted",
            first_mined && first_valid);

        check_test("Replay: second block mined", replay_mined);
        check_test(
            "Replay: already confirmed transaction rejected",
            !replay_accepted);

        check_test(
            "Replay: no second block or transaction added",
            ctx.blockchain.height() == height_after_first &&
                ctx.blockchain.get_num_txs() == 1);

        check_test(
            "Replay: balances not changed for a second time",
            ctx.users.get_user_by_pk(sender).balance() == sender_after_first &&
                ctx.users.get_user_by_pk(receiver).balance() == receiver_after_first);
    }

    void test_wrong_prev_hash()
    {
        TestContext ctx;

        Hash wrong_prev(32, static_cast<std::uint8_t>(0xFF));

        const Hash correct_prev =
            ctx.blockchain.get_last_block_hash();

        if (wrong_prev == correct_prev)
            wrong_prev = Hash(32, static_cast<std::uint8_t>(0xFE));

        Block block(wrong_prev, std::vector<Transaction>{});

        // Bloką iškasame, kad atmetimo priežastis būtų prev_hash,
        // o ne netinkamas PoW.
        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Wrong prev_hash: block has valid PoW", mined);
        check_test("Wrong prev_hash: block rejected", !accepted);

        check_test(
            "Wrong prev_hash: chain unchanged",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0);
    }

    void test_invalid_pow()
    {
        TestContext ctx;

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{});

        // Sąmoningai randame nonce, kurio hash neatitinka PoW.
        // Negalime tiesiog manyti, kad nonce = 0 bus netinkamas.
        bool found_invalid_pow = false;

        for (std::size_t i = 0; i < 10'000; ++i)
        {
            if (!is_valid_hash(block.block_hash()))
            {
                found_invalid_pow = true;
                break;
            }

            block.incr_nonce();
        }

        const BalanceState state(ctx.users);

        const bool accepted =
            found_invalid_pow &&
            ctx.blockchain.is_block_valid(block, state);

        check_test(
            "Invalid PoW: found nonce that fails difficulty",
            found_invalid_pow);

        check_test(
            "Invalid PoW: block rejected",
            found_invalid_pow && !accepted);

        check_test(
            "Invalid PoW: chain unchanged",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0);
    }

    void test_modified_block_content()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver = ctx.pk(1);

        Transaction original_tx(sender, receiver, 10, 1);

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{original_tx});

        const bool mined = mine_for_test(block);

        // Pakeičiame transakciją jau sukurtame bloke,
        // tačiau Merkle šaknis antraštėje lieka nepakeista.
        // Taip imituojame turinio pakeitimą po bloko sudarymo.
        Transaction modified_tx(sender, receiver, 20, 1);

        BlockTestAccess::replace_transaction(block, 0, modified_tx);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Modified block content: original block mined", mined);
        check_test(
            "Modified block content: block rejected",
            !accepted);

        check_test(
            "Modified block content: chain unchanged",
            ctx.blockchain.height() == 1 &&
                ctx.blockchain.get_num_txs() == 0);

        check_test(
            "Modified block content: balances unchanged",
            ctx.users.get_user_by_pk(sender).balance() == 100 &&
                ctx.users.get_user_by_pk(receiver).balance() == 100);
    }

    void test_wrong_block_version()
    {
        TestContext ctx;

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{});

        // Nustatome nepalaikomą versiją.
        BlockTestAccess::set_version(block, Version + 1);

        // Vėl iškasame, kad blokas turėtų tinkamą PoW
        // pagal jau pakeistą antraštę.
        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Wrong version: block mined", mined);
        check_test(
            "Wrong version: block rejected",
            !accepted);
    }

    void test_wrong_block_difficulty()
    {
        TestContext ctx;

        if (Difficulty == 0)
        {
            check_test(
                "Wrong difficulty: skipped because Difficulty is zero",
                true);
            return;
        }

        Block block(
            ctx.blockchain.get_last_block_hash(),
            std::vector<Transaction>{});

        // Bandoma sumažinti antraštėje nurodytą sudėtingumą.
        BlockTestAccess::set_difficulty(block, Difficulty - 1);

        const bool mined = mine_for_test(block);

        const BalanceState state(ctx.users);

        const bool accepted =
            mined && ctx.blockchain.is_block_valid(block, state);

        check_test("Wrong difficulty: block mined", mined);
        check_test(
            "Wrong difficulty: block rejected",
            !accepted);
    }

    void test_losing_candidate_state_isolation()
    {
        TestContext ctx;

        const std::string sender = ctx.pk(0);
        const std::string receiver_b = ctx.pk(1);
        const std::string receiver_c = ctx.pk(2);

        const Money amount1 = 70;
        const Money amount2 = 80;

        const std::size_t seq1 = 1;
        const std::size_t seq2 = 2;

        const Hash id1 = get_tx_id(
            sender, receiver_b, amount1, seq1);

        const Hash id2 = get_tx_id(
            sender, receiver_c, amount2, seq2);

        const bool added1 =
            ctx.mempool.add_tx(sender, receiver_b, amount1, seq1);

        const bool added2 =
            ctx.mempool.add_tx(sender, receiver_c, amount2, seq2);

        Transaction tx1 = ctx.mempool.get_tx_by_id(id1);
        Transaction tx2 = ctx.mempool.get_tx_by_id(id2);

        const Hash previous_hash =
            ctx.blockchain.get_last_block_hash();

        // Abu kandidatai remiasi tuo pačiu prev_hash.
        Block candidate1(
            previous_hash,
            std::vector<Transaction>{tx1});

        Block candidate2(
            previous_hash,
            std::vector<Transaction>{tx2});

        const bool mined1 = mine_for_test(candidate1);
        const bool mined2 = mine_for_test(candidate2);

        // Abu kandidatai tikrinami nuo tos pačios pradinės būsenos.
        BalanceState initial_state(ctx.users);

        const bool valid1 =
            mined1 &&
            ctx.blockchain.is_block_valid(candidate1, initial_state);

        const bool valid2 =
            mined2 &&
            ctx.blockchain.is_block_valid(candidate2, initial_state);

        // Pasirenkame pirmą kandidatą nugalėtoju.
        if (valid1)
        {
            ctx.blockchain.append(candidate1);
            apply_accepted_block(candidate1, ctx.users, ctx.mempool);
        }

        check_test(
            "Candidate isolation: both transactions added",
            added1 && added2);

        check_test(
            "Candidate isolation: both candidates mined",
            mined1 && mined2);

        check_test(
            "Candidate isolation: both valid from same initial state",
            valid1 && valid2);

        check_test(
            "Candidate isolation: original BalanceState unchanged",
            balance_of(initial_state, sender) == 100 &&
                balance_of(initial_state, receiver_b) == 100 &&
                balance_of(initial_state, receiver_c) == 100);

        check_test(
            "Candidate isolation: only winner included",
            valid1 && ctx.blockchain.get_num_txs() == 1);

        check_test(
            "Candidate isolation: losing transaction remains in Mempool",
            ctx.mempool.size() == 1 &&
                ctx.mempool.txs().front().transaction_id() == id2);

        check_test(
            "Candidate isolation: losing candidate did not change balances",
            ctx.users.get_user_by_pk(sender).balance() == 30 &&
                ctx.users.get_user_by_pk(receiver_b).balance() == 170 &&
                ctx.users.get_user_by_pk(receiver_c).balance() == 100);
    }
}

// ============================================================
// VIEŠAI PRIEINAMI TESTAVIMO METODAI
// ============================================================

void check_test(const std::string &name, bool passed)
{
    std::cout << (passed ? "[PASS] " : "[FAIL] ")
              << name << '\n';

    if (passed)
        ++passed_tests;
    else
        ++failed_tests;
}

std::vector<Transaction> generate_test_transactions(std::size_t n)
{
    std::vector<Transaction> txs;
    txs.reserve(n);

    // Deterministiniai duomenys: kiekviena transakcija turi
    // kitą sequence ir sumą.
    for (std::size_t i = 0; i < n; ++i)
    {
        txs.emplace_back(
            "User1",
            "User2",
            static_cast<Money>(10 + i),
            i + 1);
    }

    return txs;
}

Transaction create_modified_transaction(const Transaction &tx)
{
    return Transaction(
        tx.sender(),
        tx.receiver(),
        tx.amount() + 1,
        tx.sequence());
}

void run_merkle_tests()
{
    std::cout << "\n=== MERKLE TREE TESTS ===\n";

    test_merkle_tree_sizes();
    test_transaction_content_change();
    test_transaction_order_change();
}

void run_blockchain_tests()
{
    std::cout << "\n=== BLOCKCHAIN VALIDATION TESTS ===\n";

    test_valid_payment();
    test_unknown_receiver();
    test_zero_amount();
    test_self_payment();
    test_invalid_transaction_id();
    test_two_payments_exceed_balance();
    test_duplicate_id_inside_block();
    test_confirmed_transaction_replay();
    test_wrong_prev_hash();
    test_invalid_pow();
    test_modified_block_content();

    test_wrong_block_version();
    test_wrong_block_difficulty();

    test_losing_candidate_state_isolation();
}

int run_all_tests()
{
    passed_tests = 0;
    failed_tests = 0;

    run_merkle_tests();
    run_blockchain_tests();

    std::cout << "\n=== TEST SUMMARY ===\n"
              << "Passed: " << passed_tests << '\n'
              << "Failed: " << failed_tests << '\n';

    return failed_tests == 0 ? 0 : 1;
}