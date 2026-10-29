#include <iostream>
#include <iomanip>
#include <sstream>

#include "pagalb_fjos.h"
#include "blokai.h"

// std::vector<User> generate_users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator)
// {
//     std::vector<User> users;
//     users.reserve(num);

//     // balansui
//     std::uniform_int_distribution<Money> dist(min_balance, max_balance);

//     for (int i = 0; i < num; ++i)
//     {
//         std::string name = "User" + (i + 1);
//         std::string public_key = generate_random_string(64, "0123456789abcdef", generator);
//         Money balance = dist(generator);
//         users.push_back(User(name, public_key, balance));
//     }

//     return users;
// }

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

// std::vector<Transaction> generate_txs(std::size_t num, Money min_amount, Money max_amount, std::vector<User> &users, std::mt19937 &generator)
// {
//     std::vector<Transaction> txs;
//     txs.reserve(num);

//     std::uniform_int_distribution<Money> amount_dist(min_amount, max_amount);
//     std::uniform_int_distribution<std::size_t> user_idx_dist(0, users.size() - 1);

//     for (std::size_t i = 0; i < num; ++i)
//     {
//         std::size_t sender_idx = user_idx_dist(generator); // pasirenkam random-tąjį vartotoją-siuntėją
//         std::size_t receiver_idx;
//         do
//         {
//             receiver_idx = user_idx_dist(generator);
//         } while (receiver_idx == sender_idx); // pasirenkam random-tąjį NE TA PATĮ vartotoją-gavėją

//         std::string sender = users.at(sender_idx).name();
//         std::string receiver = users.at(receiver_idx).name();
//         Money amount = amount_dist(generator);
//         // sequence = i + 1 (kad pradėt nuo vieno)

//         txs.push_back(Transaction(sender, receiver, amount, i + 1));
//     }

//     return txs;
// }

Hash calc_tx_root(std::vector<Transaction> &txs)
{
    Hash tx_root;
    std::vector<std::uint8_t> bytes; // surinksim visų transaction_id baitus (transaction_id — hashas)
    for (Transaction &tx : txs)
    {
        Hash id = tx.transaction_id();
        bytes.insert(bytes.end(), id.begin(), id.end());
    }
    if (bytes.empty()) // genesis blokui
    {
        tx_root = str_i_hasha(" ");
    }
    else
    {
        tx_root = gaut_256bit_hasha(baitus_sujungt_po_4(std::move(bytes)));
    }
    return tx_root;
}

// koleikas hardcoded trijų tikrinimas
bool is_valid_hash(Hash hash) // dėl spartos netikrina ar hashas tinkamas — ar yra 32 baitų
{
    return hash.size() >= 2 && hash[0] == 0x00 && (hash[1] & 0xF0) == 0x00; // hash[1] & 0xF0 tikrina ar antrojo baito (hash[1] svarbesnioji pusė lygi 0 === ar trečias hasho skaitmuo == 0)
}

std::string to_hex(std::vector<std::uint8_t> &baitai)
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