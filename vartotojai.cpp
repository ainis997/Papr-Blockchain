#include <stdexcept>

#include "vartotojai.h"
#include "pagalb_fjos.h"

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

std::size_t Users::get_user_idx_by_name(std::string name)
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