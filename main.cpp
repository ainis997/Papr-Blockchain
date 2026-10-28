#include <random>
#include "klases.h"
#include "pagalb_fjos.h"

int main()
{
    std::random_device rd;
    std::mt19937 generator(rd()); // rd() — sėkla, rd() grąžina pseudo-random unsigned int tiesiai iš kompo

    std::vector<User> users = generate_users(1000, 100, 1000000, generator);

    std::vector<Transaction> mempool = generate_txs(10000, 10, 10000, users, generator); // tyčia netikrinama, susigeneruos ir klaidingų transakcijų (pvz. turi 100 bet moka 1000)

    return 0;
}