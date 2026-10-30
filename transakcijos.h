#pragma once

#include "maisa.h"
#include "vartotojai.h"
#include "konstantos.h"

Hash get_tx_id(std::string &sender, std::string &receiver, Money amount, std::size_t sequence);
Hash get_tx_id(std::string &&sender, std::string &&receiver, Money amount, std::size_t sequence);

class Transaction
{
    std::string sender_;   // siuntėjo public_key
    std::string receiver_; // gavėjo public_key
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

    void print_data() const;
};

class Mempool
{
    std::vector<Transaction> mempool_;
    Users &users_;
    std::vector<Transaction> rejected_txs_; // rejected txs = removed invalid txs

public:
    Mempool(Users &users) : users_(users) {};

    Mempool(Users &users, std::size_t num, Money min_amount, Money max_amount, std::mt19937 &generator);

    const std::vector<Transaction> &txs() const // galinis const — garantuoja kad niekaip viduj nekeičiam mempoolo; pradinis const — neleidzia keist mempoolo isorej
    {
        return mempool_;
    }

    std::size_t size()
    {
        return mempool_.size();
    }

    const std::vector<Transaction> &rejected_txs()
    {
        return rejected_txs_;
    }
    std::size_t num_rejected_txs()
    {
        return rejected_txs_.size();
    }

    bool add_tx(std::string sender, std::string receiver, Money amount);
    bool add_tx(std::string sender, std::string receiver, Money amount, std::size_t sequence);

    bool id_exists(Hash tx_id);

    Transaction get_tx_by_id(Hash tx_id); // grąžina transakcijos KOPIJĄ

    std::vector<Transaction> valid_txs_remaining();

    std::size_t num_valid_txs_remaining();

    std::vector<Transaction> invalid_txs_remaining();

    std::size_t num_invalid_txs_remaining();

    void remove_tx(Hash tx_id);
    void remove_invalid_tx(Hash tx_id);

    std::vector<Transaction> collect_txs(std::size_t max_n, std::mt19937 &generator);
};