#include <stdexcept>

#include "blokai.h"
#include "pagalb_fjos.h"

// BlockHeader

BlockHeader::BlockHeader(
    Hash prev_hash,
    std::uint64_t timestamp,
    std::uint32_t version,
    std::uint64_t nonce,
    std::uint32_t difficulty,     // kiek nulių pradžioj
    std::vector<Transaction> &txs // NEKEISIMAS VEKTORIUS NIEKAIP
    ) : prev_hash_(prev_hash), timestamp_(timestamp), version_(version), nonce_(nonce), difficulty_(difficulty)
{
    // tx_root_ apskaičiavimas
    tx_root_ = calc_tx_root(txs);
}

//

Hash Block::block_hash()
{
    std::string prev_hash_str(header_.prev_hash_.begin(), header_.prev_hash_.end());
    std::string tx_root_str(header_.tx_root_.begin(), header_.tx_root_.end());
    std::string str_hashint = // block headerio stringas hashinimui
        prev_hash_str + "|" +
        std::to_string(header_.timestamp_) + "|" +
        std::to_string(header_.version_) + "|" +
        tx_root_str + "|" +
        std::to_string(header_.nonce_) + "|" +
        std::to_string(header_.difficulty_);
    return str_i_hasha(str_hashint);
}

// blockchain

Block Blockchain::create_genesis_block()
{
    std::vector<Transaction> txs_empty;          // sukuriam atskirą kintamąjį, nes rvalue nepriims
    Block genesis_block(Hash(32, 0), txs_empty); // paduodam tuščią transakcijų vektorių

    return genesis_block;
}

Hash Blockchain::get_last_block_hash()
{
    if (chain_.size() == 0)
        return Hash(32, 0);                           // neaktualu
    return chain_.at(chain_.size() - 1).block_hash(); // iškviečiam paskutinio blockchaino blokui .block_hash() metodą, kad gaut to bloko hashą
}