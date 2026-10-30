#include <random>
#include <iostream>
#include <chrono>

#include "blokai.h"
#include "pagalb_fjos.h"

const int Seed = 123456789;

int main()
{
    std::random_device rd;
    std::mt19937 generator(Seed); // rd() — sėkla, rd() grąžina pseudo-random unsigned int tiesiai iš kompo

    Users users(1000, 100, 1000000, generator);
    const std::size_t initial_money_supply = users.calc_money_supply(); // pradinė visų lėšų suma

    Mempool mempool(users, 10000, 10, 10000, generator); // tyčia netikrinama, susigeneruos ir klaidingų transakcijų (pvz. turi 100 bet moka 1000)

    auto total_start = std::chrono::high_resolution_clock::now(); // PRADŽIA ČIA (matuojam blockchaino laiką, ne visko visko)

    // blockchaino pradžia (konstruktorius iškart sukuria ir genesis bloką)
    Blockchain blockchain(users, mempool);

    while (mempool.num_valid_txs_remaining() > 0)
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
        {
            std::cout << "\nBLOCK REJECTED\n";
            continue; // cancel, per naują kandidatuo
        }

        // SUMAŽINT MEMPOOL IR PAKEIST BALANSUS
        for (auto &tx : candidate_block.transactions())
        {
            mempool.remove_tx(tx.transaction_id());
            // atnaujint balansai
            // DADARYTINA TEORINĖS KLAIDOS LOGIKA (kaip ir viskas patikrinta bet)
            try
            {
                User &sender = users.get_user_by_pk(tx.sender());
                User &receiver = users.get_user_by_pk(tx.receiver());
                sender.change_balance(-1 * static_cast<int>(tx.amount())); // transakcijos amount visada teigiamas — atimam amountą iš siuntėjo
                receiver.change_balance(static_cast<int>(tx.amount()));
            }
            catch (const std::runtime_error &e)
            {
                // PERMISLYTINA
                std::cerr << e.what() << '\n';
            }
        }

        std::cout << "\nAccepted block:";
        blockchain.present_block_data(blockchain.height() - 1); // pridėtasis blokas — paskutinis
        // papildomai pridedam nes pats blockchain nemato
        present_mining_data(attempts, duration);
    }

    auto total_end = std::chrono::high_resolution_clock::now();

    // galutinė visų lėšų suma
    const std::size_t final_money_supply = users.calc_money_supply();

    // BENDRI DUOMENYS

    // atmestos
    std::cout << "\nREJECTED TRANSACTIONS:\n";
    for (const auto &tx : mempool.rejected_txs())
    {
        tx.print_data(users);
    }
    // likusios
    std::cout << "\nREMAINING TRANSACTIONS:\n";
    for (const auto &tx : mempool.txs())
    {
        tx.print_data(users);
    }

    // BENDRA STATISTIKA
    std::size_t num_accepted_blocks = blockchain.height();
    std::size_t num_users = users.size();
    std::size_t num_accepted_txs = blockchain.get_num_txs();
    std::size_t num_removed_invalid_txs = mempool.num_rejected_txs(); // atmestų (visos blogos transakcijos išskyrus per didelio amount transakcijas)
    std::size_t num_remaining_txs = mempool.size();                   // likusių (pagal viską — tik per didelio amount transakcijos; nebent kitokiom sąlygom gali likt ir validžių transakcijų)
    std::chrono::duration<double> total_duration_temp = total_end - total_start;
    double total_duration = total_duration_temp.count();
    std::cout << "\nGENERAL STATS:";
    present_overall_data(
        num_users,
        num_accepted_blocks,
        num_accepted_txs,
        num_removed_invalid_txs,
        num_remaining_txs,
        initial_money_supply,
        final_money_supply,
        total_duration);

    // pristatyt vartotojų, transakcijų statistiką ir konkrečias likusias transakcijas, pradinę/galutinę lėšų sumas, bendrą laiką

    return 0;
}