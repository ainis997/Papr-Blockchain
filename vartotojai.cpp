#include <stdexcept>
#include <algorithm>

#include "vartotojai.h"
#include "pagalb_fjos.h"

void User::change_balance(int change)
{
    int new_balance = static_cast<int>(balance_) + change; // change > 0 => gavo lėšų; change < 0 => davė lėšų
    if (new_balance < 0)
        throw std::runtime_error("Invalid balance change.");
    balance_ = static_cast<Money>(new_balance);
}

Users::Users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator) // sugeneruot num userių
{
    users_.reserve(num);

    // balansui
    std::uniform_int_distribution<Money> dist(min_balance, max_balance);

    for (int i = 0; i < num; ++i)
    {
        std::string name = "User" + (i + 1);
        std::string public_key = generate_random_string(64, "0123456789abcdef", generator);
        Money balance = dist(generator);
        users_.push_back(User(name, public_key, balance));
    }
}

User &Users::get_user_by_pk(std::string pk)
{
    auto it = std::find_if(users_.begin(), users_.end(), [pk](const User &user)
                           { return user.public_key() == pk; });
    if (it != users_.end())
        return *it;
    throw std::runtime_error("User with public key " + pk + " not found");
}

User &Users::get_user_by_name(std::string name)
{
    auto it = std::find_if(users_.begin(), users_.end(), [name](const User &user)
                           { return user.name() == name; });
    if (it != users_.end())
        return *it;
    throw std::runtime_error("User with name " + name + " not found");
}

Money Users::calc_money_supply()
{
    Money sum = 0;
    for (const auto &user : users_)
    {
        sum += user.balance();
    }
    return sum;
}