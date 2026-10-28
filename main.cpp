#include <random>
#include "klases.h"
#include "pagalb_fjos.h"

int main()
{
    std::random_device rd;
    std::mt19937 generator(rd()); // rd() — sėkla, rd() grąžina pseudo-random unsigned int tiesiai iš kompo

    std::vector<User> users = generate_users(1000, 100, 1000000, generator);

    std::vector<Transaction> mempool;

    return 0;
}