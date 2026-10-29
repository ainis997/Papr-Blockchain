#include <random>
#include "klases.h"
#include "pagalb_fjos.h"

int main()
{
    std::random_device rd;
    std::mt19937 generator(rd()); // rd() — sėkla, rd() grąžina pseudo-random unsigned int tiesiai iš kompo

    Users users(1000, 100, 1000000, generator);

    Mempool mempool(users, 10000, 10, 10000, generator); // tyčia netikrinama, susigeneruos ir klaidingų transakcijų (pvz. turi 100 bet moka 1000)

    // blockchaino pradžia (konstruktorius iškart sukuria ir genesis bloką)
    Blockchain blockchain(users);

    // atsirenkam iki 100 valid transakcijų
    std::vector<Transaction> candidate_txs = mempool.collect_txs(100, generator);
    // ...
    // 1. Iš fondo atsitiktinai, be pasikartojimų pasirink iki 100 tinkamų transakcijų. Paskutinis blokas gali būti mažesnis.
    // 2. Jų taikymą tikrink laikinoje būsenos kopijoje, sąrašo tvarka. Kandidato kūrimas neturi keisti tikrųjų balansų.
    // 3. Apskaičiuok tx_root, suformuok antraštę ir ieškok tinkamo nonce.
    // 4. Iš naujo patikrink iškastą bloką. Tik tada pridėk jį prie grandinės, pritaikyk balansų pokyčius ir pašalink
    // įtrauktas transakcijas iš fondo.
    // 5. Kartok, kol fondas ištuštės arba neliks priimtinų transakcijų. Struktūriškai netinkamas atmesk su priežastimi;
    // laikinai netinkamoms apibrėžk atidėjimo arba atmetimo taisyklę. Jų nelaikyk įtrauktomis ir nesuk
    // nesibaigiančio ciklo.

    return 0;
}