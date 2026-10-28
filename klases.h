#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime> // paprasčiau
#include "maisa.h"

using Hash = std::vector<std::uint8_t>; // dīgi muzīlai boūt std::array<std::uint8_t, 32>
using Money = std::uint32_t;

// KONSTANTOS

constexpr std::uint32_t Version = 1;
constexpr std::uint32_t Difficulty = 3; // pirmi 3 — nuliai

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
        Hash tx_root,
        std::uint64_t nonce,
        std::uint32_t difficulty // kiek nulių pradžioj
        ) : prev_hash_(prev_hash), timestamp_(timestamp), version_(version), tx_root_(tx_root), nonce_(nonce), difficulty_(difficulty)
    {
    }
};

class Block
{
    BlockHeader header_;
    std::vector<Transaction> transactions_; // šjp bus Merkle Tree struktūra

public:
    Block(BlockHeader header, std::vector<Transaction> transactions) : header_(header), transactions_(transactions) {}
};

class Blockchain
{
    std::vector<Block> chain_;
    std::vector<User> &users_; // balansam, siuntėjam/gavėjam tikrint; Nuoroda users vektoriun main() funkcijoj

public:
    Blockchain(std::vector<User> &users) : users_(users) // konstruktorius, sukuria users nuorodą ir sukuria genesis block
    {
        chain_.push_back(create_genesis_block());
    }

    Block create_genesis_block()
    {
        BlockHeader block_header(
            Hash(32, 0),  // 32 nulių vektorius
            std::time(0), // time(0) grąžina dab. laiką, išreikštą sekundėm nuo 1970-01-01
            Version,
            str_i_hasha(" "),
            0,
            Difficulty);
        Block genesis_block(block_header, std::vector<Transaction>()); // paduodam tuščią transakcijų vektorių

        return genesis_block;
    }

    bool validateBlock(Block block)
    {
        // visa logika...
    }

    void append(Block block)
    {
        // pirma validate tada pridėt
    }
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