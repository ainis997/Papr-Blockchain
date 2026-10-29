#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime> // paprasčiau
#include <stdexcept>
#include <algorithm>
#include "maisa.h"

using Hash = std::vector<std::uint8_t>; // dīgi muzīlai boūt std::array<std::uint8_t, 32>
using Money = std::uint32_t;

// KONSTANTOS

constexpr std::uint32_t Version = 1;
constexpr std::uint32_t Difficulty = 3; // pirmi 3 — nuliai

// ============ USER(S)

class User
{
    std::string name_;
    std::string public_key_;
    Money balance_;

public:
    User() = default;

    User(std::string &name, std::string &public_key) : name_(name), public_key_(public_key) {}

    User(std::string &name, std::string &public_key, Money balance) : name_(name), public_key_(public_key), balance_(balance) {}

    std::string name()
    {
        return name_;
    }

    std::string public_key()
    {
        return public_key_;
    }

    Money balance()
    {
        return balance_;
    }
};

class Users
{
    std::vector<User> users_;

public:
    Users() = default;

    Users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator) // sugeneruot num userių
    {
        std::vector<User> users;
        users.reserve(num);

        // balansui
        std::uniform_int_distribution<Money> dist(min_balance, max_balance);

        for (int i = 0; i < num; ++i)
        {
            std::string name = "User" + (i + 1);
            std::string public_key = generate_random_string(64, "0123456789abcdef", generator);
            Money balance = dist(generator);
            users.push_back(User(name, public_key, balance));
        }
    }

    std::vector<User> users()
    {
        return users_;
    }

    std::size_t size()
    {
        return users_.size();
    }

    User at(std::size_t i)
    {
        return users_.at(i);
    }

    // jeigu būtų besikartojančių vardų (nors neturėtų būt), tai rastų tik pirmąjį tokiu vardu
    std::size_t get_user_idx_by_name(std::string &name)
    {
        for (auto i = 0; i < users_.size(); ++i)
        {
            if (users_.at(i).name() == name)
            {
                return i;
            }
        }
        throw std::runtime_error("No user with name " + name);
    }
};

// =========== TRANSAKCIJOS

class Transaction
{
    std::string sender_;
    std::string receiver_;
    Money amount_;         // unsigned, bo neneigiamas
    std::size_t sequence_; // transakcijos eilės numeris (naudinga pvz. jei yra dvi+ tokios pačios transakcijos — kad skirtųsi id ir pan.)
    Hash transaction_id_;

public:
    Transaction(
        std::string sender,
        std::string receiver,
        Money amount,
        std::size_t sequence) : sender_(sender), receiver_(receiver), amount_(amount), sequence_(sequence)
    {
        transaction_id_ = str_i_hasha(sender + "|" + receiver + "|" + std::to_string(amount) + "|" + std::to_string(sequence));
    }

    std::string sender()
    {
        return sender_;
    }

    std::string receiver()
    {
        return receiver_;
    }

    Money amount()
    {
        return amount_;
    }

    std::size_t sequence()
    {
        return sequence_;
    }

    Hash transaction_id()
    {
        return transaction_id_;
    }
};

class Mempool
{
    std::vector<Transaction> mempool_;
    Users &users_;
    // std::vector<std::vector<std::size_t>> candidate_txs_idxs_; // sąrašai transakcijų, kurios pasirinktos kandidatam blokam, indeksų (vienas vektorius — vienas kandidat. blokų transakcijų rinkinys)

public:
    Mempool(Users &users) : users_(users) {};

    Mempool(Users &users, std::size_t num, Money min_amount, Money max_amount, std::mt19937 &generator) : users_(users)
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

    std::vector<Transaction> collect_txs(std::size_t max_n, std::mt19937 &generator)
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
};

// BLOKAI

// gal suliet Block klasėn
class BlockHeader
{
public:
    Hash prev_hash_;
    std::uint64_t timestamp_;
    std::uint32_t version_;
    Hash tx_root_;
    std::uint64_t nonce_;
    std::uint32_t difficulty_; // kiek nulių pradžioj

public:
    BlockHeader(
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
};

class Block
{
    BlockHeader header_;
    std::vector<Transaction> transactions_; // šjp bus Merkle Tree struktūra

public:
    Block(Hash prev_hash, std::vector<Transaction> &transactions)
        : transactions_(transactions),
          header_(
              prev_hash,
              std::time(0),
              Version,
              0,
              Difficulty,
              transactions) {}
    // transakcijos bus pertikrinamos pas blockchain prieš pridedant transakciją jan

    Hash block_hash()
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
};

class Blockchain
{
    std::vector<Block> chain_;
    Users &users_; // balansam, siuntėjam/gavėjam tikrint; Nuoroda users vektoriun main() funkcijoj

public:
    Blockchain(Users &users) : users_(users) // konstruktorius, sukuria users nuorodą ir sukuria genesis block
    {
        chain_.push_back(create_genesis_block());
    }

    Block create_genesis_block()
    {
        std::vector<Transaction> txs_empty;          // sukuriam atskirą kintamąjį, nes rvalue nepriims
        Block genesis_block(Hash(32, 0), txs_empty); // paduodam tuščią transakcijų vektorių

        return genesis_block;
    }

    Hash get_last_block_hash()
    {
        if (chain_.size() == 0)
            return Hash(32, 0);                           // neaktualu
        return chain_.at(chain_.size() - 1).block_hash(); // iškviečiam paskutinio blockchaino blokui .block_hash() metodą, kad gaut to bloko hashą
    }

    // bool validateBlock(Block block) // nors tikrinimą darys ir mineris (prieš sukurdamas kandidatinį bloką), bet reik daryt ir galutinį pertikrinimą prieš patį pridėjimą
    // {
    //     // visa logika...
    // }

    // void append(Block block)
    // {
    //     // pirma validate tada pridėt
    // }
};

// ===============================================
// USER
