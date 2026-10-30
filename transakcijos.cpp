#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "transakcijos.h"
#include "pagalb_fjos.h"

Hash get_tx_id(std::string &sender, std::string &receiver, Money amount, std::size_t sequence)
{
    return str_i_hasha(sender + "|" + receiver + "|" + std::to_string(amount) + "|" + std::to_string(sequence));
}
// rvalue perdengimas
Hash get_tx_id(std::string &&sender, std::string &&receiver, Money amount, std::size_t sequence)
{
    return str_i_hasha(sender + "|" + receiver + "|" + std::to_string(amount) + "|" + std::to_string(sequence));
}

void Transaction::print_data(Users &users) const // reikalinga nuoroda prie users kad matyt vartotojų vardus ir balansus
{
    std::string sender_name = users.get_user_by_pk(sender_).name();
    std::string receiver_name = users.get_user_by_pk(receiver_).name();
    Money sender_balance = users.get_user_by_pk(sender_).balance();
    Money receiver_balance = users.get_user_by_pk(receiver_).balance();
    std::cout << std::left << "\n"
              << std::setw(25) << "sender: " << sender_name << '\n'
              << std::setw(25) << "sender balance: " << sender_balance << '\n'
              << std::setw(25) << "receiver: " << receiver_name << '\n'
              << std::setw(25) << "receiver balance: " << receiver_balance << '\n'
              << std::setw(25) << "amount: " << amount_ << '\n'
              << std::setw(25) << "sequence: " << sequence_ << '\n'
              << std::setw(25) << "id: " << to_hex(transaction_id_) << '\n';
}

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

        std::string sender = users.at(sender_idx).public_key();
        std::string receiver = users.at(receiver_idx).public_key();
        Money amount = amount_dist(generator);

        add_tx(sender, receiver, amount, i + 1);
    }
}

bool Mempool::add_tx(std::string sender, std::string receiver, Money amount)
{
    std::size_t sequence = mempool_.size() + 1; // +1 nes indeksuojam sequence NUO 1 (ne nuo 0)
    Hash tx_id = get_tx_id(sender, receiver, amount, sequence);
    if (!id_exists(tx_id)) // praktiškai neturėtų nutikt id sutapimas — sequence skirsis
    {
        mempool_.push_back(Transaction(sender, receiver, amount, sequence));
        return true;
    }
    return false;
}
// perdengimas žinant sequence
bool Mempool::add_tx(std::string sender, std::string receiver, Money amount, std::size_t sequence)
{
    Hash tx_id = get_tx_id(sender, receiver, amount, sequence);
    if (!id_exists(tx_id)) // praktiškai neturėtų nutikt id sutapimas — sequence skirsis
    {
        mempool_.push_back(Transaction(sender, receiver, amount, sequence));
        return true;
    }
    return false;
}

bool Mempool::id_exists(Hash tx_id) // naudot PRIEŠ įterpimą mempoolan, PO
{
    auto it = std::find_if(mempool_.begin(), mempool_.end(), [tx_id](Transaction &tx)
                           { return tx.transaction_id() == tx_id; });
    if (it != mempool_.end())
        return true;
    return false;
}

Transaction Mempool::get_tx_by_id(Hash tx_id) // grąžina transakcijos KOPIJĄ
{
    auto it = std::find_if(mempool_.begin(), mempool_.end(), [tx_id](Transaction &tx)
                           { return tx.transaction_id() == tx_id; });
    if (it != mempool_.end())
        return *it;
    throw std::runtime_error("Transaction with id " + to_hex(tx_id) + " not found");
}

std::vector<Transaction> Mempool::valid_txs_remaining()
{
    std::vector<Transaction> valid_txs;
    for (Transaction &tx : mempool_)
    {
        // transakcijos id tikrinimas
        if (tx.transaction_id() != get_tx_id(tx.sender(), tx.receiver(), tx.amount(), tx.sequence()))
            continue;

        // siuntėjo ir gavėjo (ne)sutapimas
        if (tx.sender() == tx.receiver())
        {
            continue;
        }
        // siuntėjo gavėjo egzistavimas
        User sender_user;
        User receiver_user;
        try
        {
            // jeigu bus neegzistuojantis vartotojas, mes errorą
            sender_user = users_.get_user_by_pk(tx.sender());
            receiver_user = users_.get_user_by_pk(tx.receiver());
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
        // jeigu validus
        valid_txs.push_back(tx);
    }
    return valid_txs;
}

std::size_t Mempool::num_valid_txs_remaining()
{
    return valid_txs_remaining().size();
}

std::vector<Transaction> Mempool::invalid_txs_remaining()
{
    std::vector<Transaction> invalid_txs;
    for (Transaction &tx : mempool_)
    {
        // transakcijos id tikrinimas
        if (tx.transaction_id() != get_tx_id(tx.sender(), tx.receiver(), tx.amount(), tx.sequence()))
            invalid_txs.push_back(tx);
        // siuntėjo ir gavėjo (ne)sutapimas
        if (tx.sender() == tx.receiver())
        {
            invalid_txs.push_back(tx);
        }
        // siuntėjo gavėjo egzistavimas
        User sender_user;
        User receiver_user;
        try
        {
            // jeigu bus neegzistuojantis vartotojas, mes errorą
            sender_user = users_.get_user_by_pk(tx.sender());
            receiver_user = users_.get_user_by_pk(tx.receiver());
        }
        catch (std::runtime_error &e)
        {
            invalid_txs.push_back(tx);
        }
        // siuntėjo būsimo balanso po transakcijos tikrinimas
        int new_balance = static_cast<int>(sender_user.balance()) - static_cast<int>(tx.amount());
        if (new_balance < 0)
        {
            invalid_txs.push_back(tx);
        }
        // jeigu validus — neinskaitom
    }
    return invalid_txs;
}

std::size_t Mempool::num_invalid_txs_remaining()
{
    return invalid_txs_remaining().size();
}

void Mempool::remove_tx(Hash tx_id)
{
    std::erase_if(mempool_, [tx_id](Transaction &tx)
                  { return tx.transaction_id() == tx_id; }); // transakciją identifikuoja transaction_id
}

void Mempool::remove_invalid_tx(Hash tx_id)
{
    rejected_txs_.push_back(get_tx_by_id(tx_id));

    std::erase_if(mempool_, [tx_id](Transaction &tx)
                  { return tx.transaction_id() == tx_id; }); // transakciją identifikuoja transaction_id
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

        checked_idxs.push_back(idx);
        Transaction tx = mempool_.at(idx);

        // transakcijos id tikrinimas
        if (tx.transaction_id() != get_tx_id(tx.sender(), tx.receiver(), tx.amount(), tx.sequence()))
        {
            continue;
        }
        // siuntėjo ir gavėjo (ne)sutapimas
        if (tx.sender() == tx.receiver())
        {
            continue;
        }
        // siuntėjo, gavėjo egzistavimo tikrinimas
        User sender_user;
        User receiver_user;
        try
        {
            // jeigu bus neegzistuojantis vartotojas, mes errorą
            sender_user = users_.get_user_by_pk(tx.sender());
            receiver_user = users_.get_user_by_pk(tx.receiver());
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