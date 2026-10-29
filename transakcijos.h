#pragma once

#include "maisa.h"
#include "vartotojai.h"
#include "konstantos.h"

Hash get_tx_id(std::string &sender, std::string &receiver, Money amount, std::size_t sequence);

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
        transaction_id_ = get_tx_id(sender, receiver, amount, sequence);
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

    Mempool(Users &users, std::size_t num, Money min_amount, Money max_amount, std::mt19937 &generator);

    std::size_t valid_txs_remaining();

    std::vector<Transaction> collect_txs(std::size_t max_n, std::mt19937 &generator);
};