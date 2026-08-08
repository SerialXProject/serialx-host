# CLAUDE.md — SerialX Shell

Questo file è il punto di ingresso per Claude Code su questo repository.
Le istruzioni operative dettagliate (build, test, convenzioni del parser,
regole sui flag di `config.h`) sono in **`AGENTS.md`**: leggilo prima di
modificare `src/`. Questo file aggiunge solo contesto rapido e preferenze
specifiche per le sessioni con Claude.

## Riassunto in una riga

Libreria Arduino C++ (no dipendenze oltre `Arduino.h`) che espone
variabili e funzioni del firmware come comandi testuali via seriale.
Nessuna toolchain di build "vera": si valida compilando `tests/main.cpp`
con g++ per un target desktop (stub Arduino in `tests/Arduino.h/.cpp`).

## Prima di iniziare

- Leggi `AGENTS.md` per: struttura repo, comando di compilazione esatto,
  regole sui flag `SERIALX_SUPPORT_*`/`SERIALX_ENABLE_*`, e le insidie note
  del parser di comandi (formato posizionale a caratteri fissi).
- Non assumere di avere un Arduino fisico collegato: tutta la verifica in
  questo ambiente passa dal binario compilato in `build/`.

## Come preferisco che Claude lavori qui

- Quando modifichi `SerialXShell.cpp`/`.h`, mostrami sempre il comando g++
  con cui hai verificato che compila, e l'output di un comando di prova
  reale (es. `gi temperature` dopo aver toccato il ramo `int`), non solo
  "dovrebbe funzionare".
- Se una modifica tocca un flag in `config.h`, ricompila mentalmente (o
  davvero, se hai il tool a disposizione) anche con quel flag disattivato,
  per controllare che i guard `#if` non lascino codice orfano. Prima di
  usare un nuovo `SERIALX_ENABLE_*`/`SERIALX_SUPPORT_*` in un `#if`,
  verifica che sia davvero `#define`-ato in `config.h`: un flag "fantasma"
  produce codice morto e incompleto — è già successo in passato con un
  sistema di logging poi rimosso interamente, non ripetere l'errore.
- Le risposte/spiegazioni per chi userà la libreria (README, commenti nei
  file `.ino` di esempio) le voglio in italiano, coerente con lo stile
  attuale di `README.md` e dei commenti nel codice.
- Non introdurre `String` di Arduino in punti nuovi: il codice sta
  migrando verso buffer `char[]` statici per motivi di RAM su AVR: nuovo
  codice deve seguire quella direzione, non quella vecchia.
- Se aggiungi un tipo di variabile o una feature opzionale, tocca *tutti*
  i punti elencati in `AGENTS.md` (modello, tre switch nel `.cpp`,
  `printInfo`, eventualmente header) — è facile dimenticarne uno e avere
  una build che compila solo con certi flag.

## Cosa evitare

- Non proporre di sostituire il sistema di flag a `#define` con qualcosa
  di più "moderno" (template, `constexpr`, ecc.) a meno che non venga
  chiesto esplicitamente: il design attuale è intenzionale per poter
  disattivare interi rami di codice a compile-time su target con poca
  flash.
- Non toccare `LICENSE`, `icon.png`, `banner.png` a meno di richiesta
  esplicita.
- Non fidarti ciecamente dei commenti nel codice quando descrivono lo
  stato di una feature (es. "Not implemented yet" su
  `SERIALX_SHOW_VALUES_IN_HELP` in `config.h`): verificane lo stato reale
  nel `.cpp` prima di dire all'utente se una feature è disponibile o no.
- Non reintrodurre logging comandi o altre feature "a metà" dietro flag
  non definiti in `config.h` — se richiesto, va implementato per intero
  (vedi `AGENTS.md`, sezione "Cosa NON fare").
