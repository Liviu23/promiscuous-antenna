# Promiscuous Antenna

Simulare pe o hartă tor a unor agenți — în principal iepuri și vulpi — afișată în terminal.
Agenții comunică prin anunțuri, *broadcasterii* anunță ce nevoi pot satisface
(hrană, ascunziș, loc liber pentru o plantă nouă), iar *receiverii* ascultă permanent și își aleg
acțiunea după ponderile propriilor nevoi. Starea se salvează în format JSON.

### Tema 0

- [X] Nume proiect (poate fi schimbat ulterior)
- [X] Scurtă descriere a temei alese, ce v-ați propus să implementați

## Tema 1

#### Cerințe
- [x] definirea a minim **3-4 clase** folosind compunere cu clasele definite de voi; moștenirile nu se iau în considerare aici
  - [`WorldMap`](include/WorldMap.h) conține un [`TorusGeometry`](include/TorusGeometry.h), un vector de [`Cell`](include/Cell.h) și, pentru fiecare entitate, un [`Footprint`](include/Footprint.h) și un [`Coord`](include/Coord.h)
  - [`Legend`](include/Legend.h) conține [`Glyph`](include/Glyph.h)-uri și [`LayerProfile`](include/LayerProfile.h)-uri
  - [`Viewport`](include/Viewport.h) conține un `TorusGeometry`
  - [`MapConsole`](app/MapConsole.h) conține un [`MapView`](app/MapView.h)
- [X] constructori de inițializare cu parametri pentru fiecare clasă
  - ex. `Coord(int x, int y)`, `TorusGeometry(int width, int height)`, `Footprint(std::vector<Coord>)`, `Cell(const LayerCapacity&)`, `WorldMap(const TorusGeometry&, const LayerCapacity&)`, `Glyph(...)`, `LayerProfile(...)`, `Legend(...)`, `Viewport(...)`
- [x] pentru o aceeași (singură) clasă: constructor de copiere, `operator=` de copiere, destructor
  - [`WorldMap`](src/WorldMap.cpp): `operator=` prin copy-and-swap; folosite în consolă la `salveaza` (copiere) și `restaureaza` / `importa` (atribuire)
<!-- - [ ] pentru o altă clasă: constructor de mutare, `operator=` de mutare, destructor -->
<!-- - [ ] pentru o altă clasă: toate cele 5 funcții membru speciale -->
- [x] `operator<<` pentru **toate** clasele pentru afișare (`std::ostream`) folosind compunere de apeluri cu `operator<<`
  - toate clasele din `include/`
  - lipsesc: `MapView` și `MapConsole` din `app/`, dar au modul propriu de afișare în consolă
- [x] cât mai multe `const` (unde este cazul) și funcții `private`
  - toate interogările sunt `const` (`anchorOf`, `entitiesInRange`, `look`, `worldAt`...)
  - sau sunt funcții private `WorldMap::requireRoom` / `occupy` / `vacate`, `Footprint::hasAnchor`, `Legend::glyphOf` / `padded`, `TorusGeometry::wrapAxis`
- [x] implementarea a minim 3 funcții membru publice pentru funcționalități netriviale specifice temei alese, dintre care cel puțin 1-2 funcții mai complexe
  - `WorldMap::moveTo` (verifică toate celulele noi înainte să schimbe ceva, inclusiv pentru amprente mari care se suprapun cu ele însele)
  - `WorldMap::entitiesInRange` (cine "aude" un broadcast pe tor, fără duplicate pentru entitățile pe mai multe celule) și `WorldMap::freeCellsInRange`
  - `TorusGeometry::area` / `shortestDelta` (distanțe și zone pe tor)
  - `Legend::look` (ce se desenează după profil)
  - `WorldMap::saveToFile`
- [x] scenariu de utilizare **cu sens** a claselor definite:
  - [`MapConsole`](app/MapConsole.cpp) rulează comenzile din [`tastatura.txt`](tastatura.txt): vulpea trece peste marginea hărții, aude iepurii ascunși în tufiș, mănâncă unul; starea se salvează, se restaurează și se exportă în JSON
  - crearea de obiecte și apelarea tuturor funcțiilor membru publice în main
  - vor fi adăugate în fișierul `tastatura.txt` DOAR exemple de date de intrare de la tastatură (dacă există) pentru a acoperi cât mai multe cazuri; dacă aveți nevoie de date din fișiere, creați alte fișiere separat
- [ ] minim 52-60% din codul propriu să fie C++, adică să aveți mai mult cod decât ce e inclus în template; testele nu se pun, nu vă luați după procentajul din interfața web; detalii la ore
- [x] `.gitattributes` configurat corect (nu trebuie făcute modificări aici)
- [x] tag de `git`: de exemplu `v0.1`
- [x] serviciu de integrare continuă (CI) cu **toate bifele**; exemplu: GitHub Actions
- [ ] code review #1 2 proiecte

## Tema 2

#### Cerințe
- [x] separarea codului din clase în `.h` (sau `.hpp`) și `.cpp`
  - câte o clasă per pereche `include/X.h` + `src/X.cpp`; citirea/scrierea JSON e separată în `src/WorldMapJson.cpp` și `src/LegendJson.cpp`
- [ ] moșteniri:
  - minim o clasă de bază și **3 clase derivate** din aceeași ierarhie; cele 3 derivate moștenesc aceeași clasă de bază
  - ierarhia trebuie să fie cu bază proprie, nu derivată dintr-o clasă predefinită
  - [ ] funcții virtuale (pure) apelate prin pointeri de bază din clasa care conține atributul de tip pointer de bază
    - minim o funcție virtuală va fi **specifică temei** (i.e. nu simple citiri/afișări sau preluate din biblioteci i.e. draw/update/render)
    - constructori virtuali (clone): sunt necesari, dar nu se consideră funcții specifice temei
    - afișare virtuală, interfață non-virtuală
  - [ ] apelarea constructorului din clasa de bază din constructori din derivate
  - [ ] clasă cu atribut de tip pointer la o clasă de bază cu derivate; aici apelați funcțiile virtuale prin pointer de bază, eventual prin interfața non-virtuală din bază
    - [ ] suprascris cc/op= pentru copieri/atribuiri corecte, copy and swap
    - [ ] `dynamic_cast`/`std::dynamic_pointer_cast` pentru downcast cu sens
    - [ ] smart pointers (recomandat, opțional)
- [x] excepții
  - [x] ierarhie proprie cu baza `std::exception` sau derivată din `std::exception`; minim **3** clase pentru erori specifice distincte
    - [`Errors.h`](include/Errors.h): `SimulationError` (derivată din `std::runtime_error`) cu `ConfigError`, `CellFullError`, `UnknownEntityError`, `DuplicateEntityError`, `StorageError`
    - clasele de excepții trebuie să trateze categorii de erori distincte (exemplu de erori echivalente: citire fișiere cu diverse extensii)
  - [x] utilizare cu sens: de exemplu, `throw` în constructor (sau funcție care întoarce un obiect), `try`/`catch` în `main`
    - `throw` în constructorii `TorusGeometry`, `Footprint`, `LayerProfile`, `Legend`, `Viewport` și în `loadFromFile`; `catch` în [`main`](app/main.cpp) (oprire cu mesaj) și în `MapConsole::run` (comanda eșuează, harta rămâne neschimbată)
  - această ierarhie va fi complet independentă de ierarhia cu funcții virtuale
- [x] funcții și atribute `static`
  - atribute: `TorusGeometry::minSide`, `MapView::statusRows`; funcții: `WorldMap::loadFromFile`, `Legend::loadFromFile`, `EntityKinds::loadFromFile`, `TorusGeometry::offsetOf`
- [x] STL
  - `std::vector`, `std::array`, `std::map`, `std::unordered_map`, `std::optional`, `std::ranges::sort` / `unique` / `find`, `std::filesystem` (în teste)
- [x] cât mai multe `const`
- [ ] funcții *de nivel înalt*, de eliminat cât mai mulți getters/setters/funcții low-level
- [ ] minim 75-78% din codul propriu să fie C++; testele nu se pun, nu vă luați după procentajul din interfața web
- [ ] la sfârșit: commit separat cu adăugarea unei noi clase derivate fără a modifica restul codului, **pe lângă cele 3 derivate deja adăugate** din aceeași ierarhie
  - noua derivată nu poate fi una existentă care a fost ștearsă și adăugată din nou
  - noua derivată va fi integrată în codul existent (adică va fi folosită, nu adăugată doar ca să fie)
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.2`
- [ ] code review #2 2 proiecte

## Tema 3

#### Cerințe
- [ ] 2 șabloane de proiectare (design patterns)
- [ ] o clasă șablon cu sens; minim **2 instanțieri**
  - [x] preferabil și o funcție șablon (template) cu sens; minim 2 instanțieri
    - `nameOf` / `valueOf` din [`MapTypes.cpp`](src/MapTypes.cpp), instanțiate pentru `Layer` și `Direction`
- [ ] minim 80-90% din codul propriu să fie C++; testele nu se pun, nu vă luați după procentajul din interfața web
<!-- - [ ] o specializare pe funcție/clasă șablon -->
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.3` sau `v1.0`
- [ ] code review #3 2 proiecte

## Utilizare

Programul pornește o consolă care citește comenzi (în CI, din `tastatura.txt`).
Harta inițială vine din [`assets/map.json`](assets/map.json), iar simbolurile și culorile din [`assets/legend.json`](assets/legend.json).

Hărți mai mari, de încărcat cu `importa`:

| Fișier | Mărime | Ce conține |
|---|---|---|
| [`assets/padure.json`](assets/padure.json) | 48x24 | pâlcuri dese de tufișuri, iepuri ascunși în ele, vulpi în luminișuri |
| [`assets/pajiste.json`](assets/pajiste.json) | 80x30 | câmpuri mari de iarbă, colonii de iepuri în jurul vizuinilor |
| [`assets/arhipelag.json`](assets/arhipelag.json) | 120x40 | insule de iarbă, unele peste marginile torului; maxim 2 animale pe celulă |

Exemplu: `importa assets/arhipelag.json`, apoi `fereastra 80 16`, `centreaza 2 35` și `arata`: insula din jurul celulei (2, 35) apare întreagă, deși trece peste marginea din stânga și peste cea de jos a hărții.

| Comandă | Ce face |
|---|---|
| `harta`, `catalog`, `legenda` | afișează harta, tipul fiecărei entități, legenda |
| `celula x y` | ce se află într-o celulă, pe fiecare strat |
| `muta id x y`, `pas id DIR` | mută o entitate (DIR: `N NE E SE S SW W NW`); peste margine se intră pe partea cealaltă |
| `distanta x1 y1 x2 y2`, `vecini x y 4\|8` | geometria torului |
| `aude id raza`, `liber x y raza strat` | cine e în raza unei entități; unde mai e loc pe un strat |
| `sterge id` | scoate o entitate de pe hartă |
| `salveaza`, `restaureaza` | copie a hărții în memorie și revenire la ea |
| `exporta fisier`, `importa fisier` | salvează / încarcă starea în JSON, în același format ca `assets/map.json` |
| `arata`, `profil`, `centreaza x y`, `fereastra coloane randuri` | desenează harta; schimbă profilul de afișare și fereastra |
| `interactiv [secunde]` | harta pe tot ecranul (ftxui) |
| `stop` | ieșire |

În modul interactiv: săgețile mută fereastra cu o celulă, `w a s d` cu patru celule, `Tab` schimbă profilul, `q` / `Esc` revine în consolă.

Exemplu de fișier salvat cu `exporta`:

```json
{
  "width": 20,
  "height": 10,
  "capacity": {"ground":1,"object":1,"animal":3},
  "kinds": {
    "bush": {"layer":"object","footprint":[[0,0],[1,0],[0,1],[1,1]]},
    "rabbit": {"layer":"animal","footprint":[[0,0]]}
  },
  "entities": [
    {"id":1,"kind":"bush","x":19,"y":9},
    {"id":11,"kind":"rabbit","x":0,"y":0}
  ]
}
```

## Instrucțiuni de compilare

Proiectul este configurat cu CMake.

Instrucțiuni pentru terminal:

1. Pasul de configurare
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
# sau ./scripts/cmake.sh configure
```

Sau pe Windows cu GCC folosind Git Bash:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
# sau ./scripts/cmake.sh configure -g Ninja
```

Pentru a configura cu ASan, avem opțiunea `-DUSE_ASAN=ON` (nu merge pe Windows cu GCC):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON
# sau ./scripts/cmake.sh configure -e "-DUSE_ASAN=ON"
```


La acest pas putem cere să generăm fișiere de proiect pentru diverse medii de lucru.


2. Pasul de compilare
```sh
cmake --build build --config Debug --parallel 6
# sau ./scripts/cmake.sh build
```

Cu opțiunea `parallel` specificăm numărul de fișiere compilate în paralel.

3. Pasul de testare
```sh
ctest --test-dir build -C Debug --output-on-failure
# sau ./scripts/cmake.sh test
```

Testele sunt **condiție, nu punctaj**: CI verde include și testele. Nu se punctează nici numărul
lor, nici acoperirea (coverage). Testați logica de domeniu din `src/`, prin interfața publică din
`include/`, fără `std::cin` și fără căi absolute. Ce e greu de testat — UI complex, evenimente
aleatoare, un server real — rămâne în `app/` și se verifică prin smoke tests și la demo.

4. Pasul de instalare (opțional)
```sh
cmake --install build --config Debug --prefix install_dir
# sau ./scripts/cmake.sh install
```

Vezi și [`scripts/cmake.sh`](scripts/cmake.sh).

Observație: folderele `build/` și `install_dir/` sunt adăugate în fișierul `.gitignore` deoarece
conțin fișiere generate și nu ne ajută să le versionăm.

## Instrucțiuni pentru a rula executabilul

Există mai multe variante:

1. Din directorul de build (implicit `build`). Executabilul se află la locația `./build/promiscuous_antenna_main` după ce a fost rulat pasul de compilare al proiectului (`./scripts/cmake.sh build` - pasul 2 de mai sus).

```sh
./build/promiscuous_antenna_main
```

2. Din directorul `install_dir`. Executabilul se află la locația `./install_dir/bin/promiscuous_antenna_main` după ce a fost rulat pasul de instalare (`./scripts/cmake.sh install` - pasul 4 de mai sus).

```sh
./install_dir/bin/promiscuous_antenna_main
```

3. Rularea programului folosind Valgrind se poate face executând script-ul `./scripts/run_valgrind.sh` din rădăcina proiectului. Pe Windows acest script se poate rula folosind WSL (Windows Subsystem for Linux). Valgrind se poate rula în modul interactiv folosind: `RUN_INTERACTIVE=true ./scripts/run_valgrind.sh`

Implicit, nu se rulează interactiv, iar datele pentru `std::cin` sunt preluate din fișierul `tastatura.txt`.

```sh
RUN_INTERACTIVE=true ./scripts/run_valgrind.sh
# sau
./scripts/run_valgrind.sh
```

Valgrind rulează și testele.

4. Pentru a rula executabilul folosind ASan, este nevoie ca la pasul de configurare (vezi mai sus) să fie activat acest sanitizer. Ar trebui să meargă pe macOS și Linux. Pentru Windows, ar merge doar cu MSVC (nerecomandat).

Comanda este aceeași ca la pasul 1 sau 2. Nu merge combinat cu Valgrind.

```sh
./build/promiscuous_antenna_main
./build/promiscuous_antenna_test
# sau
./install_dir/bin/promiscuous_antenna_main
./install_dir/bin/promiscuous_antenna_test
```

## License

The project is licensed under [AGPLv3](LICENSE).

The [template repository](https://github.com/mcmarius/oop-template) itself is licensed under [Unlicense](LICENSE.template).

## Resurse

- [nlohmann/json](https://github.com/nlohmann/json) v3.12.0, licență MIT — citirea hărții din `assets/map.json` (vendored în `ext/private/json/`)
- FTXUI
  <!-- renovate: datasource=github-tags depName=ArthurSonzogni/ftxui versioning=loose -->
  - Versiune: [v7.0.3](https://github.com/ArthurSonzogni/ftxui/releases/tag/v7.0.3) (MIT) — afișarea hărții în terminal (`app/MapView`), adusă cu FetchContent
