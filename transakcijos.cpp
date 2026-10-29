#include <algorithm>
#include <stdexcept>

#include "transakcijos.h"

Mempool::Mempool(Users &users, std::size_t num, Money min_amount, Money max_amount, std::mt19937 &generator) : users_(users)
{
    mempool_.reserve(num);

    std::uniform_int_distribution<Money> amount_dist(min_amount, max_amount);
    std::uniform_int_distribution<std::size_t> user_idx_dist(0, users.size() - 1);

    for (std::size_t i = 0; i < num; ++i)
    {
        std::size_t sender_idx = user_idx_dist(generator); // pasirenkam random-tąjį vartotoją-siuntėją
        std::size_t receiver_idx;
        do
        {
            receiver_idx = user_idx_dist(generator);
        } while (receiver_idx == sender_idx); // pasirenkam random-tąjį NE TA PATĮ vartotoją-gavėją

        std::string sender = users.at(sender_idx).name();
        std::string receiver = users.at(receiver_idx).name();
        Money amount = amount_dist(generator);
        // sequence = i + 1 (kad pradėt nuo vieno)

        mempool_.push_back(Transaction(sender, receiver, amount, i + 1));
    }
}

std::vector<Transaction> Mempool::collect_txs(std::size_t max_n, std::mt19937 &generator)
{
    std::size_t min_n = static_cast<std::size_t>(0.9 * max_n); // įvairovei, kad ne visad pvz. 100
    std::uniform_int_distribution<std::size_t> n_dist(min_n, max_n);
    std::size_t n = n_dist(generator); // kiek realiai bandysim įterpt transakcijų blokan

    std::vector<Transaction> txs;
    txs.reserve(n); // jeigu bus mažiau — po to reiks atpūst

    std::uniform_int_distribution<std::size_t> txs_idx_dist(0, mempool_.size() - 1);
    std::vector<std::size_t> chosen_idxs;
    std::vector<std::size_t> checked_idxs;
    while (txs.size() < n && checked_idxs.size() != mempool_.size())
    {
        std::size_t idx;
        do
        {
            idx = txs_idx_dist(generator);
        } while (std::find(checked_idxs.begin(), checked_idxs.end(), idx) != checked_idxs.end());

        chosen_idxs.push_back(idx);
        Transaction tx = mempool_.at(idx);

        // siuntėjo, gavėjo egzistavimo tikrinimas
        std::string sender = tx.sender();     // sender vardas
        std::string receiver = tx.receiver(); // receiver vardas
        User sender_user;
        User receiver_user;
        try
        {
            // jeigu bus neegzistuojantis vartotojas, mes errorą
            sender_user = users_.at(users_.get_user_idx_by_name(sender));
            receiver_user = users_.at(users_.get_user_idx_by_name(receiver));
        }
        catch (std::runtime_error &e)
        {
            continue;
        }

        // siuntėjo būsimo balanso po transakcijos tikrinimas
        int new_balance = static_cast<int>(sender_user.balance()) - static_cast<int>(tx.amount());
        if (new_balance < 0)
        {
            continue;
        }

        // jeigu abu tikrinimus praėjo, tai transakcija turi būt valid — pridėsma kandidatinian blokan
        txs.push_back(tx);
        chosen_idxs.push_back(idx);
    }
    // kandidat. blokui parinktų transakcijų iš mempoolo DAR NEŠALINAM; pašalinsim tik kai bus patvirtintas blokas (kad žinot ką šalint, surenkam pasirinktų transakcijų indeksų sąrašą)
    // candidate_txs_idxs_.push_back(chosen_idxs);

    return txs;
}