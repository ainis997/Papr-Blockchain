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