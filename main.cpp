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
    // TIKRINIMUI: vartotojų balansų būsenos gavimas (manipuliuosim tikrinimui, ATNAUJINSIM PO KIEKVIENO BLOKO PRIDĖTO)
    BalanceState balance_state(users);

    Mempool mempool(users, 10000, 10, 10000, generator); // tyčia netikrinama, susigeneruos ir klaidingų transakcijų (pvz. turi 100 bet moka 1000)

    auto total_start = std::chrono::steady_clock::now(); // PRADŽIA ČIA (matuojam blockchaino laiką, ne visko visko)

    // blockchaino pradžia (konstruktorius iškart sukuria ir genesis bloką)
    Blockchain blockchain(users, mempool);

    while (true) // PRIEŠ KIEKVIENĄ CIKLĄ (kiekvienam blokui) ATNAUJINAM BALANSUS PAGAL DAB. BLOCKCHAINO DUOMENIS (kad galėtumėm tiksliai tikrint)
    {
        balance_state = BalanceState(users);
        if (mempool.num_valid_txs_remaining(balance_state) == 0)
            break;

        // KANDIDATŲ PARUOŠIMAS

        constexpr std::size_t NumCandidates = 5;

        Hash prev_hash = blockchain.get_last_block_hash();

        std::vector<Block> candidates;
        candidates.reserve(NumCandidates);

        for (std::size_t i = 0; i < NumCandidates; ++i)
        {
            BalanceState candidate_state = balance_state;
            // atsirenkam iki 100 valid transakcijų
            std::vector<Transaction> candidate_txs = mempool.collect_txs(MaxTxsInBlock, candidate_state, generator);

            // kandidatinio bloko sukūrimas (iškart vektoriaus viduj — emplace_back() tą daro)
            candidates.emplace_back(prev_hash, std::move(candidate_txs));
        }

        // KASIMAS

        const auto InitialMiningBudget = std::chrono::seconds(1);
        const auto MaxMiningBudget = std::chrono::seconds(5);

        std::vector<bool> mined(NumCandidates, false);
        std::vector<std::size_t> attempts(NumCandidates, 0);
        std::vector<double> mining_durations(NumCandidates, 0.0);

        auto mining_budget = InitialMiningBudget;

        while (true)
        {
            for (std::size_t i = 0; i < NumCandidates; ++i)
            {
                // jei kandidatas jau iškastas arba jei jis tuščias — nekasam
                if (mined.at(i) || candidates.at(i).transactions().empty())
                    continue;

                auto mining_start = std::chrono::steady_clock::now();

                while (std::chrono::steady_clock::now() - mining_start < mining_budget)
                {
                    Hash hash = candidates.at(i).block_hash();
                    ++attempts.at(i);

                    if (is_valid_hash(hash))
                    {
                        mined.at(i) = true;
                        break;
                    }

                    candidates.at(i).incr_nonce();
                }

                auto mining_end = std::chrono::steady_clock::now();

                std::chrono::duration<double> dur = mining_end - mining_start;
                mining_durations.at(i) += dur.count();
            }

            // su any_of() tikrinam ar bent vienas kandidatas buvo iškastas
            bool are_any_mined = std::any_of(mined.begin(), mined.end(), [](bool is_mined)
                                             { return is_mined; });

            if (are_any_mined)
                break;

            if (mining_budget >= MaxMiningBudget)
                break;

            // padidinam biudžetą (jeigu dar nieko nepavyko iškast)
            mining_budget = std::min(mining_budget * 2, MaxMiningBudget);
        }

        // NUGALĖTOJO ATRANKA

        std::size_t winner_idx = NumCandidates; // toks idx negali būt naudojamas — toks idx liks tik jeigu nugalėtojo neradom
        for (std::size_t i = 0; i < NumCandidates; ++i)
        {
            // winner_idx = i, jei i-tasis blokas iškastas ir winner_idx dar nepriskirtas (== NumCandidates) arba jei i-tojo bloko kasimas truko trumpiau nei kol kas laimėtojo
            if (mined.at(i) && (winner_idx == NumCandidates || mining_durations.at(i) < mining_durations.at(winner_idx)))
            {
                winner_idx = i;
            }
        }
        // jei kažkodėl nepavyko nieko iškast, sekančiam kartojime bandysim vėl
        if (winner_idx == NumCandidates)
            continue;

        Block &winning_block = candidates.at(winner_idx);

        // NUGALĖTOJO PATIKRINIMAS

        if (!blockchain.is_block_valid(winning_block, balance_state))
        {
            std::cout << "\nBLOCK REJECTED\n";
            continue; // cancel, per naują kandidatuo
        }

        blockchain.append(winning_block);

        // SUMAŽINT MEMPOOL IR PAKEIST BALANSUS

        for (auto &tx : winning_block.transactions())
        {
            // atnaujint balansai
            try
            {
                User &sender = users.get_user_by_pk(tx.sender());
                User &receiver = users.get_user_by_pk(tx.receiver());
                sender.change_balance(-1 * static_cast<int>(tx.amount())); // transakcijos amount visada teigiamas — atimam amountą iš siuntėjo
                receiver.change_balance(static_cast<int>(tx.amount()));
            }
            catch (const std::runtime_error &e)
            {
                // niturīlai audātun sin
                std::cerr << e.what() << '\n';
            }

            mempool.remove_tx(tx.transaction_id());
        }

        std::cout << "\nAccepted block:";
        blockchain.present_block_data(blockchain.height() - 1); // pridėtasis blokas — paskutinis
        // papildomai pridedam nes pats blockchain nemato
        present_mining_data(attempts.at(winner_idx), mining_durations.at(winner_idx));
    }

    auto total_end = std::chrono::steady_clock::now();

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

    // // debugsnai
    // // blockchain.get_block(27).print_txs(users);
    // for (auto &tx : blockchain.get_txs_by_user(users.get_user_by_name("User8").public_key()))
    // {
    //     tx.print_data(users);
    // }
    // blockchain.present_block_data(27);

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

    return 0;
}