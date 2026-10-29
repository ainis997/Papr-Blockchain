#include <random>
#include <iostream>

#include "blokai.h"
#include "pagalb_fjos.h"

int main()
{
    std::random_device rd;
    std::mt19937 generator(rd()); // rd() — sėkla, rd() grąžina pseudo-random unsigned int tiesiai iš kompo

    Users users(1000, 100, 1000000, generator);

    Mempool mempool(users, 10000, 10, 10000, generator); // tyčia netikrinama, susigeneruos ir klaidingų transakcijų (pvz. turi 100 bet moka 1000)

    // blockchaino pradžia (konstruktorius iškart sukuria ir genesis bloką)
    Blockchain blockchain(users);

    // atsirenkam iki 100 valid transakcijų
    std::vector<Transaction> candidate_txs = mempool.collect_txs(100, generator);

    // kandidatinio bloko sukūrimas
    Hash prev_hash = blockchain.get_last_block_hash();
    Block candidate_block(prev_hash, candidate_txs);

    std::cout << "kasimo pradzia\n";

    // kasimas
    while (!is_valid_hash(candidate_block.block_hash()))
    {
        candidate_block.incr_nonce();
    }

    Hash valid_hash = candidate_block.block_hash();

    std::cout << "maisa: " << to_hex(valid_hash) << '\n';

    return 0;
}