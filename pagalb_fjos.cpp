#include <iostream>
#include <iomanip>
#include <sstream>

#include "pagalb_fjos.h"
#include "blokai.h"

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator)
{
    std::uniform_int_distribution<std::size_t> dist(0, chars.size() - 1);
    std::string random_str;
    random_str.reserve(length);

    for (int i = 0; i < length; ++i)
    {
        random_str += chars[dist(generator)];
    }

    return random_str;
}

std::vector<Hash> extract_tx_ids(const std::vector<Transaction> &txs)
{
    std::vector<Hash> ids;
    ids.reserve(txs.size());
    for (const auto &tx : txs)
    {
        ids.push_back(tx.transaction_id());
    }
    return ids;
}

Hash calc_tx_root(const std::vector<Transaction> &txs)
{
    auto tx_ids = extract_tx_ids(txs);
    MerkleTree tx_mt(tx_ids);
    return tx_mt.root();
    // Hash tx_root;
    // std::vector<std::uint8_t> bytes; // surinksim visų transaction_id baitus (transaction_id — hashas)
    // for (Transaction &tx : txs)
    // {
    //     Hash id = tx.transaction_id();
    //     bytes.insert(bytes.end(), id.begin(), id.end());
    // }
    // if (bytes.empty()) // genesis blokui
    // {
    //     tx_root = str_i_hasha(" ");
    // }
    // else
    // {
    //     tx_root = gaut_256bit_hasha(baitus_sujungt_po_4(std::move(bytes)));
    // }
    // return tx_root;
}
// RVALUE PARAMETRO PERDENGIMAS
Hash calc_tx_root(std::vector<Transaction> &&txs)
{
    auto tx_ids = extract_tx_ids(txs);
    MerkleTree tx_mt(tx_ids);
    return tx_mt.root();
    // Hash tx_root;
    // std::vector<std::uint8_t> bytes; // surinksim visų transaction_id baitus (transaction_id — hashas)
    // for (Transaction &tx : txs)
    // {
    //     Hash id = tx.transaction_id();
    //     bytes.insert(bytes.end(), id.begin(), id.end());
    // }
    // if (bytes.empty()) // genesis blokui
    // {
    //     tx_root = str_i_hasha(" ");
    // }
    // else
    // {
    //     tx_root = gaut_256bit_hasha(baitus_sujungt_po_4(std::move(bytes)));
    // }
    // return tx_root;
}

Hash hash_pair(const Hash &l, const Hash &r)
{
    Hash l_copy = l; // kad neparedaguot ir nepamove'int l nurodomojo objekto
    l_copy.insert(l_copy.end(), r.begin(), r.end());
    return gaut_256bit_hasha(baitus_sujungt_po_4(std::move(l_copy)));
}

// koleikas hardcoded trijų tikrinimas
bool is_valid_hash(const Hash &hash) // dėl spartos netikrina ar hashas tinkamas — ar yra 32 baitų
{
    return hash.size() >= 2 && hash[0] == 0x00 && (hash[1] & 0xF0) == 0x00; // hash[1] & 0xF0 tikrina ar antrojo baito (hash[1] svarbesnioji pusė lygi 0 === ar trečias hasho skaitmuo == 0)
}

std::string to_hex(const std::vector<std::uint8_t> &baitai)
{
    std::ostringstream oss;
    oss << std::hex; // tai lieka kaip nustatymas
    for (std::uint8_t baitas : baitai)
    {
        oss << std::setfill('0') << std::setw(2) << static_cast<int>(baitas);
    }
    oss << std::dec << '\n';

    return oss.str();
}
// rvalue parametro perdengimas
std::string to_hex(std::vector<std::uint8_t> &&baitai)
{
    std::ostringstream oss;
    oss << std::hex; // tai lieka kaip nustatymas
    for (std::uint8_t baitas : baitai)
    {
        oss << std::setfill('0') << std::setw(2) << static_cast<int>(baitas);
    }

    return oss.str();
}

void present_mining_data(std::size_t attempts, double duration)
{
    std::cout << std::setw(15) << "attempts: " << attempts << "\n"
              << std::setw(15) << "duration: " << duration << " s\n";
}

void present_overall_data(
    std::size_t num_users,
    std::size_t num_accepted_blocks,
    std::size_t num_accepted_txs,
    std::size_t num_removed_invalid_txs,
    std::size_t num_remaining_txs,
    Money initial_money_supply,
    Money final_money_supply,
    double total_duration)
{
    std::cout << std::left << "\n"
              << std::setw(30) << "Users: " << num_users << '\n'
              << std::setw(30) << "Blocks: " << num_accepted_blocks << '\n'
              << std::setw(30) << "Accepted transactions: " << num_accepted_txs << '\n'
              << std::setw(30) << "Rejected transactions: " << num_removed_invalid_txs << '\n'
              << std::setw(30) << "Remaining transactions: " << num_remaining_txs << '\n'
              << std::setw(30) << "Initial money supply: " << initial_money_supply << '\n'
              << std::setw(30) << "Final money supply: " << final_money_supply << '\n'
              << std::setw(30) << "Total duration: " << total_duration << '\n';
}

void present_candidate_info(std::vector<std::size_t> &attempts, std::vector<double> &mining_durations, std::vector<bool> &mined)
{
    std::cout << std::left << "\n\n"
              << "Candidate blocks:\n"
              << std::setw(15) << "Candidate" << std::setw(20) << "Mining attempts" << std::setw(20) << "Mining duration" << "Result\n";
    // atspara — attempts vektorius, bet galėtų ir bilkuris būt, svarbu indeksai (visi vienodo ilgio turi būt)
    for (std::size_t i = 0; i < attempts.size(); ++i)
    {
        std::cout << std::setw(15) << i + 1 << std::setw(20) << attempts.at(i) << std::setw(20) << mining_durations.at(i) << ((mined.at(i)) ? "mined" : "not mined") << "\n";
    }
}