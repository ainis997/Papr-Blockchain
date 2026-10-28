#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "maisa.h"

using Hash = std::vector<std::uint8_t>; // dīgi muzīlai boūt std::array<std::uint8_t, 32>
using Money = std::uint32_t;

// TRANSAKCIJOS

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
};

// BLOKAI

class BlockHeader
{
public:
    Hash prev_hash;
    std::uint64_t timestamp;
    std::uint32_t version;
    Hash tx_root;
    std::uint64_t nonce;
    std::uint32_t difficulty; // kiek nulių pradžioj
};

class Block
{
    BlockHeader header;
    std::vector<Transaction> transactions; // šjp bus Merkle Tree struktūra
};

// USER

class User
{
    std::string name_;
    std::string public_key_;
    Money balance_;

public:
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

// // ACCOUNT BALANCES

// class AccountBalances
// {
//     std::unordered_map<std::string, Money> account_balances;

//     // public: ...
// };