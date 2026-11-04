#pragma once

#include <ctime>
#include <set>

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
    Block(Hash prev_hash, std::vector<Transaction> &&transactions)
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

    void print_txs(Users &users) const;
};

class Blockchain
{
    std::vector<Block> chain_;
    std::set<Hash> confirmed_tx_ids_; // skirta greitai tikrint ar transakcija jau nebuvo pakliuvus blockchainan
    Users &users_;                    // balansam, siuntėjam/gavėjam tikrint; Nuoroda users vektoriun main() funkcijoj
    Mempool &mempool_;                // nuoroda mempoolan — reikia negalimom transakcijom šalint

public:
    Blockchain(Users &users, Mempool &mempool) : users_(users), mempool_(mempool) // konstruktorius, sukuria users nuorodą ir sukuria genesis block
    {
        chain_.push_back(create_genesis_block());
    }

    Block create_genesis_block();

    const Block &get_block(std::size_t idx) const
    {
        return chain_.at(idx); // jeigu idx >= chain_.size(), mes errorą
    }

    Hash get_last_block_hash();

    bool is_block_valid(Block block, BalanceState balances); // nors tikrinimą darys ir mineris (prieš sukurdamas kandidatinį bloką), bet reik daryt ir galutinį pertikrinimą prieš patį pridėjimą

    void append(Block block) // pridėt reikia tik validavus!
    {
        chain_.push_back(block);

        // confirmed_tx_ids_ papildymas — reikia dubliuotom transakcijom išvengt
        for (const auto &tx : block.transactions())
        {
            confirmed_tx_ids_.insert(tx.transaction_id());
        }
    }

    std::size_t height()
    {
        return chain_.size();
    }

    void present_block_data(std::size_t idx); // atspausdins idx-tojo bloko duomenis

    // bendra statistika

    std::size_t get_num_txs();

    std::vector<Transaction> get_txs_by_user(std::string public_key);
};
