#pragma once

#include <string>
#include <random>
#include "konstantos.h"

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

    Users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator); // sugeneruot num userių

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
    std::size_t get_user_idx_by_name(std::string &name);
};