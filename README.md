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

Vienam bloko aukščiui sudaromi penki kandidatiniai blokai. Kiekvienam kandidatui transakcijos atrenkamos nepriklausomai, tikrinimui naudojant atskiras laikinų balansų būsenų kopijas. Visų kandidatų pradinė balansų būsena yra vienoda, atitinkanti tuo metu patvirtintą blokų grandinės būseną.

Atrenkant transakcijas, jos tikrinamos ir vykdomos nurodyta tvarka laikinoje balansų būsenoje. Taip užtikrinama, kad viena transakcija negalėtų išleisti daugiau lėšų, nei liko po ankstesnių tame pačiame bloke esančių transakcijų.

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

Bloko transakcijų sąrašą sudaro iš `Mempool` atrinktos validžios transakcijos.

Bloke saugomos visos į jį įtrauktos transakcijos (`std::vector<Transaction>`). Merkle Tree sudaromas iš šių transakcijų ID, išlaikant jų tvarką bloke.

Merkle Tree naudojamas bloko transakcijų šakninei maišai (`tx_root`) apskaičiuoti. Pats Merkle Tree bloke nesaugomas – saugoma tik jo šakninė maiša. Prireikus medis sudaromas iš naujo, pavyzdžiui, tikrinant bloko validumą.

Viso bloko maiša apskaičiuojama naudojant jo antraštės duomenis.

### Blokų grandinė (`Blockchain`)

`Blockchain` valdo blokų grandinę ir užtikrina, kad į ją būtų įtraukiami tik validūs blokai.

Sukūrus `Blockchain` objektą, automatiškai sukuriamas pradinis genesis blokas. Kiti blokai susiejami su ankstesniais blokais, jų antraštėse nurodant ankstesnio bloko maišą.

Prieš įtraukiant bloką į grandinę, tikrinama:

- Ar bloko ankstesnio bloko maiša (`prev_hash`) atitinka paskutinio grandinės bloko maišą.
- Ar bloko versija ir transakcijų skaičius yra leistini.
- Ar iš bloko transakcijų apskaičiuota Merkle šaknis sutampa su antraštėje nurodyta `tx_root`.
- Ar bloko maiša atitinka nustatytą darbo įrodymo (`Proof of Work`) sudėtingumą.
- Ar visos transakcijos yra validžios ir gali būti įvykdytos nurodyta tvarka.
- Ar transakcijų ID dar nėra įtraukti į blokų grandinę ir nėra pasikartojančių transakcijų pačiame bloke.

Bloko validavimas atliekamas naudojant laikiną balansų būseną. Tik sėkmingai patikrinus visą bloką, jis įtraukiamas į grandinę ir atnaujinami vartotojų balansai.

Atmetus bloką, blokų grandinė, vartotojų balansai ir transakcijų telkinys nekeičiami.

### Kasimo procesas

Kiekvienam naujam blokui sudaromi penki kandidatiniai
blokai, kurie varžosi dėl teisės būti įtraukti į blokų grandinę.

Kiekvienam bloko aukščiui atliekami šie veiksmai:

1. Užfiksuojama dabartinė blokų grandinės būsena ir paskutinio bloko maiša (`prev_hash`).
2. Iš `Mempool` atrenkamos transakcijos ir sudaromi penki kandidatiniai blokai. Visi jie naudoja tos pačios pradinės balansų būsenos kopijas ir tą pačią ankstesnio bloko maišą.
3. Kiekvienas kandidatas kasamas atskirai, keičiant jo `nonce` reikšmę ir ieškant maišos, atitinkančios PoW sudėtingumo reikalavimus.
4. Kiekvienam kandidatui matuojamas kasimo laikas ir skaičiuojamas maišos skaičiavimo bandymų skaičius.
5. Iš sėkmingai iškastų kandidatų išrenkamas nugalėtojas. Nugalėtoju laikomas greičiausiai iškastas kandidatas.
6. Nugalėtojo blokas dar kartą validuojamas ir, jei yra validus, įtraukiamas į blokų grandinę.
7. Pašalinamos nugalėtojo transakcijos iš `Mempool` ir atnaujinami vartotojų balansai.
8. Pralaimėjusių kandidatų blokai atmetami, o jų transakcijos lieka `Mempool`.
9. Konsolėje pateikiami priimto bloko duomenys ir visų penkių kandidatų kasimo rezultatai.

Jeigu per nustatytą kasimo laiką (1 s) nė vienas kandidatas neiškasamas, padidinamas kasimo laiko biudžetas (ne daugiau nei iki 5 s) ir kandidatai bando toliau, tęsdami nuo jau išbandytų nonce reikšmių.

## 4. Maišos algoritmas ir darbo įrodymas

Maišoms apskaičiuoti naudojama vėlyviausia savo kurto maišos algoritmo versija.

Maišos ilgis: **256 bitai**.

Darbo įrodymo principu paremtas kasimas atliekamas keičiant bloko antraštės `nonce` reikšmę ir kiekvieną kartą perskaičiuojant bloko maišą.

Maiša laikoma tinkama, kai atitinka `is_valid_hash()` funkcijoje nustatytą sąlygą:

- maiša turi prasidėti trimis nuliais šešioliktainėje sistemoje (`000`).

### Transakcijų šakninė maiša (`tx_root`)

Transakcijų šakninė maiša apskaičiuojama naudojant Merkle medžius.

Merkle medžių lapai yra bloke esančių transakcijų ID maišos, išdėstytos tokia pačia tvarka kaip transakcijos bloke.

Kiekvienas vidinis medžio mazgas apskaičiuojamas sujungiant kairiojo ir dešiniojo vaikų maišų baitus ir gautus duomenis paduodant į naudojamą maišos algoritmą. Jeigu tam tikrame medžio lygyje lieka neporuotas mazgas, jis suporuojamas su savo kopija.

## 5. Testavimas

Programoje atliekami Merkle Tree ir blokų bei transakcijų
validavimo testai.

### Merkle Tree testai

Tikrinamas Merkle šaknies apskaičiavimo teisingumas, naudojant 1, 2, 3 ir 5 transakcijų ID.
Taip pat tikrinama, ar Merkle šaknis pasikeičia:

- pakeitus transakcijos turinį ir perskaičiavus jos ID;
- pakeitus skirtingų transakcijų tvarką bloke.

Testų rezultatai pateikiami konsolėje, nurodant, kurie
testai sėkmingai įvykdyti, o kurie nepavyko:

```
[PASS] Merkle tree: 1 transaction
[PASS] Merkle tree: 2 transactions
[PASS] Merkle tree: 3 transactions
[PASS] Merkle tree: 5 transactions
[PASS] Transaction ID changes after content modification
[PASS] Merkle root changes after content modification
[PASS] Merkle root changes after transaction reordering
```

## 6. Atsitiktinių duomenų generavimas

Vartotojų ir transakcijų generavimui naudojamas `std::mt19937` pseudoatsitiktinių skaičių generatorius.

Generatorius inicializuojamas fiksuota sėkla:

```cpp
const int Seed = 123456789;
```

Naudojama sėklos reikšmė: **123456789**.

Fiksuota sėkla leidžia pakartotinai generuoti tokią pačią pseudoatsitiktinių skaičių seką, todėl, esant tokiai pačiai programos logikai, galima atkartoti vartotojų ir transakcijų generavimą. Tačiau blokų kasimo trukmė ir pasirinkti nugalėtojai nebūtinai bus vienodi kiekvieno paleidimo metu, tam įtakos turi kompiuterio apkrova ir pan.

## 7. Programos rezultatai

### Kandidatinių blokų kasimo duomenys

Kiekvienai į tą patį blokų grandinės aukštį pretenduojančių kandidatinių blokų grupei pateikiami jų kasimo duomenys.

Pavyzdys:

```
Candidate blocks:
Candidate      Mining attempts     Mining duration     Result
1              1491                0.0327923           mined
2              6830                0.151339            mined
3              4623                0.102258            mined
4              11116               0.247714            mined
5              1495                0.032711            mined
```

### Vieno priimto bloko pavyzdys

Kiekvieną kartą priėmus bloką nugalėtoją, konsolėje pateikiami jo duomenys ir kasimo statistika.

Pavyzdys:

```
Accepted block:
height:        103
transactions:  96
prev_hash:     000b6d4789cb03465c8bce72420075e4887ce2dea0139482ed7830784d632efe
tx_root:       90e228984425450d0e71f011c04e9ac2e1b7bea4411042b55a76828f012fb19b
hash:          00081ecb36bebe0f73bba6bd25bdb6ce8d29c599d994e410143abb8bda6ec15b
difficulty:    3
nonce:         1494
attempts:      1495
duration:      0.032711 s
```

### Atmestos ir likusios transakcijos

Pasibaigus kasimo procesui, atskirai išvedamos atmestos ir `Mempool` likusios transakcijos kartu su jų duomenimis.

### Galutinė statistika

Programos pabaigoje pateikiama bendra statistika:

```text
GENERAL STATS:
Users:                        1000
Blocks:                       106
Accepted transactions:        9962
Rejected transactions:        0
Remaining transactions:       38
Initial money supply:         492482203
Final money supply:           492482203
Total duration:               186.776
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

Pateiktame programos vykdymo pavyzdyje likusios transakcijos nebuvo įvykdytos, nes jų siuntėjų balansai buvo mažesni už reikalaujamas pervesti sumas.

Pvz.:

```
sender:                  User130
sender balance:          2642
receiver:                User275
receiver balance:        511119
amount:                  8860
sequence:                9946
id:                      556042e7ab2cf1482ac58d80feac0083da0b3a73484b7ee38e66ac2983527162
```

## 8. Konkurencijos simuliacijos apribojimai

Penkių kandidatinių blokų konkurencija šioje programoje imituoja tam tikrus blokų kasimo konkurencijos principus, tačiau nėra tikras lygiavertis peer-to-peer, P2P konsensuso mechanizmas.

Pagrindiniai apribojimai:

- Programa veikia viename kompiuteryje ir viename procese.
- Visus penkis kandidatus sukuria ir kasa ta pati programa.
- Kandidatai naudoja tą pačią pradinę blokų grandinės būseną ir tą pačią ankstesnio bloko maišą.
- Nėra atskirų tinklo mazgų, kurie savarankiškai saugotų blokų grandinę ir siūlytų savo blokus.

Todėl šiame darbe realizuota penkių kandidatinių blokų kasimo konkurencijos simuliacija, o ne decentralizuotas konsensusas tarp nepriklausomų tinklo mazgų.
