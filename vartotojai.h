#pragma once

#include <string>
#include <random>
#include <map>
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

    std::string name() const
    {
        return name_;
    }

    std::string public_key() const
    {
        return public_key_;
    }

    Money balance() const
    {
        return balance_;
    }

    void change_balance(int change);
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

    User &get_user_by_pk(std::string pk);

    User &get_user_by_name(std::string name);

    Money calc_money_supply();
};

class BalanceState // klasė laikyt ir manipuliuot laikiniem userių balansam (kad tikrint bloko transakcijų validumą)
{
    std::map<std::string, Money> users_balances;

public:
    BalanceState(Users &users)
    {
        for (User &u : users.users())
        {
            users_balances.insert({u.public_key(), u.balance()});
        }
    }

    Money get_user_balance(std::string &public_key)
    {
        if (!users_balances.contains(public_key))
            throw std::runtime_error("User not found.");
        return users_balances.at(public_key);
    }

    void change_balance(std::string public_key, int change) // change > 0 => gavo lėšų; change < 0 => davė lėšų
    {
        if (users_balances.contains(public_key))
        {
            int new_balance = static_cast<int>(users_balances.at(public_key)) + change; // change > 0 => gavo lėšų; change < 0 => davė lėšų
            if (new_balance < 0)
                throw std::runtime_error("Invalid balance change.");
            users_balances.at(public_key) = static_cast<Money>(new_balance);
        }
    }
};