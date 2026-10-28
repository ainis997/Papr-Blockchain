#include "pagalb_fjos.h"
#include "klases.h"

std::vector<User> generate_users(std::uint32_t num, Money min_balance, Money max_balance, std::mt19937 &generator)
{
    std::vector<User> users;
    users.reserve(num);

    // balansui
    std::uniform_int_distribution<Money> dist(100, 1000000);

    for (int i = 0; i < num; ++i)
    {
        std::string name = "User" + (i + 1);
        std::string public_key = generate_random_string(64, "0123456789abcdef", generator);
        Money balance = dist(generator);
        users.push_back(User(name, public_key, balance));
    }

    return users;
}

std::string generate_random_string(const std::size_t length, const std::string &chars, std::mt19937 &generator)
{
    std::uniform_int_distribution<std::size_t> dist(0, chars.size() - 1);
    std::string random_str;
    random_str.reserve(length);

    for (int i = 0; i < length; ++i)
    {
        random_str += chars[dist(generator)];
    }

    return random_str;
}