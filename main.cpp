#include <random>
#include <iostream>
#include <chrono>

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

    while (mempool.valid_txs_remaining() > 0)
    {
        // atsirenkam iki 100 valid transakcijų
        std::vector<Transaction> candidate_txs = mempool.collect_txs(100, generator);

        // kandidatinio bloko sukūrimas
        Hash prev_hash = blockchain.get_last_block_hash();
        Block candidate_block(prev_hash, candidate_txs);

        // kasimas
        Hash valid_hash;
        std::size_t attempts = 1; // pirmas bandymas — įeinant while ciklan
        auto mining_start = std::chrono::high_resolution_clock::now();
        while (!is_valid_hash(valid_hash = candidate_block.block_hash()))
        {
            candidate_block.incr_nonce();
            ++attempts;
        }
        auto mining_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dur = mining_end - mining_start;
        double duration = dur.count();

        bool ar_blokas_pridetas = blockchain.append(candidate_block);
        if (!ar_blokas_pridetas)
            continue; // cancel, per naują kandidatuot

        // SUMAŽINT MEMPOOL
        //

        // ATNAUJINT BALANSUS
        //

        blockchain.present_block_data(blockchain.height() - 1); // pridėtasis blokas — paskutinis
        // papildomai pridedam nes pats blockchain nemato
        std::cout << "Attempts: " << attempts << "\nMining duration: " << duration << " s\n";
    }

    return 0;
}