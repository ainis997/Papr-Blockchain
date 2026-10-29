#pragma once

#include <ctime>

#include "konstantos.h"
#include "transakcijos.h"

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
    );
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

    Hash prev_hash()
    {
        return header_.prev_hash_;
    }

    std::uint64_t timestamp()
    {
        return header_.timestamp_;
    }

    std::uint32_t version()
    {
        return header_.version_;
    }

    Hash tx_root()
    {
        return header_.tx_root_;
    }

    std::uint64_t nonce()
    {
        return header_.nonce_;
    }

    std::uint32_t difficulty()
    {
        return header_.difficulty_;
    }

    std::vector<Transaction> transactions()
    {
        return transactions_;
    }

    std::size_t num_transactions()
    {
        return transactions_.size();
    }

    Hash block_hash();

    void incr_nonce()
    {
        ++header_.nonce_;
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

    Block create_genesis_block();

    Hash get_last_block_hash();

    bool is_block_valid(Block block); // nors tikrinimą darys ir mineris (prieš sukurdamas kandidatinį bloką), bet reik daryt ir galutinį pertikrinimą prieš patį pridėjimą

    bool append(Block block)
    {
        if (is_block_valid(block))
        {
            chain_.push_back(block);
            return true;
        }
        return false;
    }

    std::size_t height()
    {
        return chain_.size();
    }

    void present_block_data(std::size_t idx);
};
