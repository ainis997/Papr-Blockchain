#include <array>
#include <string>
#include <vector>

using Hash = std::array<std::uint8_t, 32>;

class Transaction
{
public:
    std::string sender;
    std::string receiver;
    std::uint32_t amount;   // unsigned, bo neneigiamas
    std::uint64_t sequence; // transakcijos eilės numeris (naudinga pvz. jei yra dvi+ tokios pačios transakcijos — kad skirtųsi id ir pan.)
    Hash transaction_id;
};

class BlockHeader
{
public:
    Hash prev_hash;
    std::uint64_t timestamp;
    std::uint32_t version;
    Hash tx_root;
    std::uint64_t nonce;
    std::uint32_t difficulty; // kiek nulių pradžioj
};

class Block
{
    BlockHeader header;
    std::vector<Transaction> transactions; // šjp bus Merkle Tree struktūra
};
