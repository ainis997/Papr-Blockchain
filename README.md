# Supaprastinta blokų grandinė

## 1. Projekto aprašymas

Programa realizuoja supaprastintą blokų grandinės modelį (C++ programavimo kalba).

Programoje generuojami vartotojai ir jų pradiniai balansai, kuriamos transakcijos, iš jų formuojami kandidatiniai blokai, kurie kasami pagal _Proof of Work_ principus. Iškasti blokai po patikrinimo įtraukiami į blokų grandinę.

## 2. Paleidimas

### Paleidimas

Paleidimo failas sukuriamas tokia komanda (nurodytas pvz. — naudojant g++ kompiliatorių):

```
g++ -std=c++20 main.cpp maisa.cpp pagalb_fjos.cpp blokai.cpp vartotojai.cpp transakcijos.cpp -o main
```

<sub>Čia "main" — programos pavadinimas (gali būti bet koks).</sub>

Programa paleidžiama komanda `./main` (jeigu programos pavadinimas — main.exe).

## 3. Duomenų modelis ir programos struktūra

Programa naudoja sąskaitomis (angl. _account-based_) paremtą modelį: kiekvienas vartotojas turi balansą, o transakcijos perveda tam tikrą lėšų kiekį iš vieno vartotojo kitam.

### Vartotojai (`Users`, `User`)

Programos pradžioje sugeneruojama 1 000 vartotojų. Kiekvienas vartotojas turi vardą, viešą raktą ir pradinį balansą.

Pradiniai balansai generuojami intervale nuo 100 iki 1 000 000.

Vartotojų duomenis valdo `Users` klasė, kuri taip pat leidžia gauti vartotoją pagal jo identifikatorių ir apskaičiuoti bendrą visų vartotojų lėšų kiekį.

### Transakcijos (`Transaction`)

Transakcija aprašo lėšų pervedimą iš vieno vartotojo kitam. Ją sudaro:

- `sender` – siuntėjo identifikatorius;
- `receiver` – gavėjo identifikatorius;
- `amount` – pervedamų lėšų kiekis;
- `sequence` – transakcijos eilės numeris;
- `transaction_id` – pagal kitus transakcijos duomenis apskaičiuota unikali transakcijos maiša.

Transakcijų validumui tikrinti taikomos šios sąlygos:

- Siuntėjas ir gavėjas turi egzistuoti.
- Siuntėjas ir gavėjas turi būti skirtingi.
- Transakcijos suma turi būti teigiama.
- Siuntėjas turi turėti pakankamai lėšų transakcijai įvykdyti.
- Transakcijos ID turi atitikti jos duomenis.
- Transakcija negali būti įtraukta į blokų grandinę daugiau nei vieną kartą.

### Transakcijų telkinys (`Mempool`)

`Mempool` saugo sugeneruotas transakcijas, kurios dar nėra įtrauktos į blokų grandinę.

Programos pradžioje sugeneruojama 10 000 transakcijų. Jų sumos generuojamos intervale nuo 10 iki 10 000. Sąmoningai generuojamos ir netinkamos transakcijos, pavyzdžiui, tokios, kurių siuntėjo balanso nepakanka nurodytai sumai pervesti.

Formuojant kandidatinį bloką iš telkinio atrenkama iki 100 validžių transakcijų. Sėkmingai priėmus bloką, jo transakcijos pašalinamos iš `Mempool`.

Pasibaigus programos darbui, atskirai išvedamos:

- atmestos transakcijos (`rejected_txs`);
- telkinyje likusios transakcijos (`txs`).

### Blokai (`Block`)

Kiekviename bloke saugoma antraštė ir į jį įtrauktų transakcijų sąrašas.

Bloko antraštę sudaro:

- `prev_hash` – ankstesnio bloko maiša;
- `timestamp` – bloko sukūrimo laikas;
- `version` – bloko versija;
- `tx_root` – bloko transakcijų šakninė maiša;
- `nonce` – kasimo metu keičiamas skaitiklis;
- `difficulty` – kasimo sudėtingumas.

Bloko transakcijų sąrašą sudaro iš `Mempool` atrinktos validžios transakcijos. Bloko maiša apskaičiuojama naudojant jo antraštės duomenis.

### Blokų grandinė (`Blockchain`)

`Blockchain` valdo blokų grandinę ir užtikrina, kad į ją būtų įtraukiami tik validūs blokai.

Sukūrus `Blockchain` objektą, automatiškai sukuriamas pradinis genesis blokas. Kiti blokai susiejami su ankstesniais blokais, jų antraštėse nurodant ankstesnio bloko maišą.

Kiekvienas iškastas kandidatinis blokas perduodamas `Blockchain` objektui per `append()` funkciją. Ji patikrina bloką ir prideda į blokų grandinę, jei blokas validus.

Jeigu blokas atmetamas, jo transakcijos nepašalinamos iš `Mempool`, o programa bando sudaryti kitą kandidatinį bloką.

Priėmus bloką, pašalinamos į jį įtrauktos transakcijos iš telkinio ir atnaujinami atitinkamų vartotojų balansai.

### Kasimo procesas

Kol `Mempool` lieka validžių transakcijų, atliekami šie veiksmai:

1. Iš `Mempool` atrenkama iki 100 validžių transakcijų.
2. Pagal paskutinio blokų grandinės bloko maišą sukuriamas naujas kandidatinis blokas.
3. Pradedamas kasimo laiko matavimas.
4. Keičiamas bloko `nonce` ir perskaičiuojama jo maiša, kol ji atitinka nustatytą sudėtingumo sąlygą.
5. Iškastas blokas perduodamas `Blockchain` objektui patikrinti.
6. Priėmus bloką, pašalinamos jo transakcijos iš `Mempool` ir atnaujinami vartotojų balansai.
7. Konsolėje išvedami priimto bloko duomenys ir jo kasimo statistika.

Kiekvieno bloko kasimo metu atskirai matuojama trukmė ir skaičiuojamas bandymų, kurių prireikė tinkamai maišai rasti, skaičius.

## 4. Maišos algoritmas ir darbo įrodymas

Maišoms apskaičiuoti naudojama vėlyviausia savo kurto maišos algoritmo versija.

Maišos ilgis: **256 bitai**.

Darbo įrodymo principu paremtas kasimas atliekamas keičiant bloko antraštės `nonce` reikšmę ir kiekvieną kartą perskaičiuojant bloko maišą.

Maiša laikoma tinkama, kai atitinka `is_valid_hash()` funkcijoje nustatytą sąlygą:

- maiša turi prasidėti trimis nuliais šešioliktainėje sistemoje (`000`).

### Transakcijų šakninė maiša (`tx_root`)

Transakcijų šakninė maiša sudaroma iš bendrai sujungtų transakcijų id maišos rezultato.

## 5. Atsitiktinių duomenų generavimas

Vartotojų ir transakcijų generavimui naudojamas `std::mt19937` pseudoatsitiktinių skaičių generatorius.

Generatorius inicializuojamas fiksuota sėkla:

```cpp
const int Seed = 123456789;
```

Naudojama sėklos reikšmė: **123456789**.

Fiksuota sėkla leidžia pakartotinai generuoti tokią pačią pseudoatsitiktinių skaičių seką, todėl, esant tokiai pačiai programos logikai, galima atkartoti vartotojų ir transakcijų generavimą.

## 6. Programos rezultatai

### Vieno priimto bloko pavyzdys

Kiekvieną kartą sėkmingai iškasus ir priėmus bloką, konsolėje pateikiami jo duomenys ir kasimo statistika.

Pavyzdys:

```
Accepted block:
height:        107
transactions:  99
prev_hash:     000f08fabc206707f1374577561bb885e172690c3f9ae21bdb34ee758c262948
tx_root:       688f3d0b6cd8bc716294acef87c14e444e4b19dbeda6feeb28e4638bd55055b4
hash:          000070d798f73afa7d9681df75560a44b33c55b5dfaab4cc7f70c78031bdafb9
difficulty:    3
nonce:         2384
attempts:      2385
duration:      0.0568644 s
```

### Atmestos ir likusios transakcijos

Pasibaigus kasimo procesui, atskirai išvedamos atmestos ir `Mempool` likusios transakcijos kartu su jų duomenimis.

### Galutinė statistika

Programos pabaigoje pateikiama bendra statistika:

```text
GENERAL STATS:
Users:                        1000
Blocks:                       111
Accepted transactions:        10383
Rejected transactions:        0
Remaining transactions:       57
Initial money supply:         492482203
Final money supply:           492482203
Total duration:               140.501
```

Joje pateikiama:

- vartotojų skaičius;
- priimtų blokų skaičius;
- į blokų grandinę įtrauktų transakcijų skaičius;
- pašalintų netinkamų transakcijų skaičius;
- `Mempool` likusių transakcijų skaičius;
- pradinis bendras lėšų kiekis;
- galutinis bendras lėšų kiekis;
- bendras programos vykdymo laikas.

Kaip matyti, bendra pinigų suma nepakito.

Visos likusios transakcijos nepridėtos dėl to, jog siuntėjas norėjo mokėti daugiau nei turi sąskaitoje pinigų.

Pvz.:

```
sender:                  User486
sender balance:          7601
receiver:                User498
receiver balance:        142265
amount:                  8768
sequence:                8090
id:                      9f069699740646c29a9b2a7630149fca4e3de8c4847b1a3dd0901d7511e3884b
```
