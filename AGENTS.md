# AGENTS.md — SerialX Shell (libreria Arduino)

Istruzioni operative per agenti AI che lavorano su questo repository.
Per una panoramica funzionale della libreria vedi `README.md`; questo file
si concentra su come muoversi nel codice, cosa NON rompere, e come
validare le modifiche.

## Cos'è questo progetto

SerialX Shell è una libreria C++ per Arduino (header-only per i modelli,
.cpp/.h per la classe principale) che espone variabili globali e funzioni
del firmware come comandi testuali via porta seriale (linguaggio
"SerialX_JIT"). Zero dipendenze oltre `Arduino.h`. Target: microcontrollori
con poca RAM (AVR compreso), quindi ogni scelta di design privilegia
memoria statica su allocazioni dinamiche e `String`.

## Struttura del repository

```
src/
  SerialXShell.h          # Interfaccia pubblica della classe
  SerialXShell.cpp         # Implementazione (parser comandi, get/set, help...)
  config.h                 # Flag di compilazione: tipi supportati, feature, buffer
  models/
    serialVariable.h       # Struct SerialVariable: wrappa un puntatore a variabile reale
    serialFunction.h        # Struct SerialFunction: nome + puntatore a funzione void()
    serialVirtualVariable.h # Struct SerialVirtualVariable: nome + puntatore a getter
  strings/
    Messages.h              # Stringhe di risposta (PROGMEM), versioni SERIAL/JIT
examples/                  # Sketch .ino dimostrativi (Base, Complete, FullFeature)
tests/
  emulator/
    Arduino.h / Arduino.cpp       # Stub minimale dell'API Arduino per compilare su PC
    serial_port_win.cpp            # Accesso porta seriale reale (Windows)
    serial_port_posix.cpp          # Accesso porta seriale reale (Linux/macOS)
  sketch/
    sketch.ino.cpp / sketch.h      # "Sketch" di prova: setup()/loop(), identico a un .ino
  runner/
    main.cpp                       # Bootstrap desktop: chiama setup() poi loop() in ciclo
build.ps1                  # Compila tests/ + src/*.cpp in build/main.exe (PowerShell, g++;
                              # sceglie automaticamente serial_port_win.cpp o _posix.cpp)
install.ps1                # Installa la libreria nella cartella Arduino Libraries
library.properties          # Metadati per Arduino Library Manager
```

Non esiste una toolchain CMake/Make: la build "desktop" passa da `build.ps1`
che invoca direttamente `g++`. Se lavori in un ambiente senza PowerShell,
replica lo stesso comando g++ a mano (vedi sotto).

## Come si compila e testa

Non esiste un vero Arduino collegato in questo ambiente: il modo per
validare le modifiche è compilare ed eseguire il target "desktop" in
`tests/`, che fornisce stub di `Serial`, `pinMode`, `digitalWrite`, ecc.
in `tests/emulator/Arduino.h/.cpp`. L'accesso reale alla porta seriale è
cross-platform: `serial_port_win.cpp` su Windows, `serial_port_posix.cpp`
(termios) su Linux/macOS — `build.ps1` sceglie quello giusto in automatico.
La porta usata a runtime si imposta con la variabile d'ambiente
`SERIALX_TEST_PORT` (default `COM4` su Windows, `/dev/pts/4` su Linux/macOS).

Comando equivalente a `build.ps1` su Linux/macOS (usalo se non hai `pwsh`):

```bash
mkdir -p build
g++ -std=c++17 -Isrc -Itests/emulator -Itests/sketch \
  tests/runner/main.cpp tests/sketch/sketch.ino.cpp \
  tests/emulator/Arduino.cpp tests/emulator/serial_port_posix.cpp \
  src/SerialXShell.cpp -o build/main
```

Su Windows sostituisci `serial_port_posix.cpp` con `serial_port_win.cpp`.

`tests/sketch/sketch.ino.cpp` registra un set di variabili di ogni tipo
supportato, due funzioni (`led_on`/`led_off`) e una variabile virtuale
(`sensor_read`), poi entra in un loop che legge comandi. È il file da
modificare per verificare a mano che un comando (es. `gi temperature`,
`sb led_state 1`, `r led_on`, `h`) si comporti come atteso dopo una
modifica — è l'unico file di `tests/` pensato per essere editato,
il contenuto di `tests/emulator/` e `tests/runner/` resta stabile.

**Non esistono test automatici (unit test/asserzioni)** in questo repo:
la verifica è manuale via l'eseguibile compilato. Se aggiungi una feature
non toccata da `tests/sketch/sketch.ino.cpp`, valuta se estendere quel file
per coprirla (registrando la nuova variabile/funzione), così chi verrà dopo
può riprodurre il test a mano.

## Il flag system in `config.h`

Ogni tipo di dato e ogni feature "opzionale" è controllato da una macro
`SERIALX_SUPPORT_*` / `SERIALX_ENABLE_*`, con guard `#if` sia nei modelli
(`models/*.h`) sia nell'implementazione (`SerialXShell.cpp`). Regole da
rispettare quando aggiungi un tipo o una feature:

1. Il costruttore corrispondente in `serialVariable.h` (e, se ha senso,
   `serialVirtualVariable.h`) va avvolto nello stesso `#if SERIALX_SUPPORT_X`.
2. Il branch `case 'x':` nei tre punti dove si fa lo switch sul tipo
   (`getVariable` reale, `getVariable` virtuale, `setVariable`) va avvolto
   nello stesso `#if`, altrimenti si rompe la build quando il flag è a 0.
3. Il carattere-tipo (`type`) usato nella struct deve essere univoco tra
   tutti i tipi esistenti — sono lettere singole case-sensitive
   (`b,i,f,s,c,C,u,w,d,l,D`). Non riusare una lettera già occupata.
4. Aggiorna `printInfo()` in `SerialXShell.cpp` per includere la nuova
   lettera nell'elenco "Types supported" quando il flag è attivo.
5. Se il flag è `SERIALX_ENABLE_*` (feature, non tipo), il guard va anche
   nell'header `SerialXShell.h` per dichiarare/nascondere il metodo privato
   corrispondente.

Ogni volta che introduci un nuovo `SERIALX_ENABLE_*`/`SERIALX_SUPPORT_*`,
verifica che sia effettivamente `#define`-ato in `config.h` (con un valore
di default, 0 o 1) prima di usarlo in `#if` altrove: un flag referenziato
ma mai definito produce un ramo di codice silenziosamente escluso dalla
build, spesso incompleto (membri privati mancanti, buffer mai dichiarati).
Il repo ha già avuto questo problema in passato con un flag di logging poi
rimosso interamente: non reintrodurre feature "a metà" dietro un `#if` non
supportato da un define reale.

## Convenzioni del parser di comandi

Il protocollo SerialX_JIT è posizionale e a caratteri fissi, non tokenizzato
con parsing generico. Occhio a questi dettagli quando modifichi
`handleCommand`/`getVariable`/`setVariable`:

- Comando 1 carattere = operazione (`g`, `s`, `r`, `h`, `i`, `a`...).
- Per get/set: `cmd[1]` è la lettera-tipo, `cmd[2]` deve essere `' '`
  (variabile reale) o `'v'` (variabile virtuale, solo per `get`), il nome
  parte da `cmd[3]`.
- `runFunction` viene chiamato con `cmd + 2`, cioè si assume che dopo `r`
  ci sia esattamente uno spazio e poi il nome — non c'è validazione di
  `cmd[1] == ' '` come invece avviene per get/set. Se tocchi `runFunction`,
  valuta se allineare questa asimmetria o documentarla esplicitamente.
- L'autenticazione (`SERIALX_ENABLE_AUTH`) intercetta il comando **prima**
  dello switch principale: se `accessKey` è impostata e la sessione non è
  sbloccata, ogni comando diverso da `akey <chiave>` viene rifiutato con
  `MSG_NOT_AUTHORIZED`. Di conseguenza il `case 'a':` dentro lo switch in
  `handleCommand` è irraggiungibile per `akey` quando l'auth è attiva (il
  controllo `#if SERIALX_ENABLE_AUTH` sopra lo switch lo intercetta già);
  resta raggiungibile solo se in futuro si aggiungono altri comandi che
  iniziano per `a`. Non è un bug bloccante, ma non aggiungere logica critica
  in quel branch senza verificarne la raggiungibilità.
- I buffer sono statici a dimensione fissa (`SERIALX_INPUT_BUFFER_SIZE`,
  `SERIALX_VAR_NAME_SIZE`, `SERIALX_ACCESS_KEY_SIZE`): qualunque nuova
  stringa che scrivi deve usare `strncpy` con truncation esplicita e
  terminatore `'\0'` manuale, mai `strcpy`/`sprintf` senza limite, per non
  reintrodurre overflow su target con poca RAM.

## Stile e vincoli specifici per target embedded

- Non introdurre `String` di Arduino se evitabile: il codice esistente sta
  progressivamente sostituendo `String` con `char[]` a dimensione fissa
  (vedi commento "was 4 separate arrays" / "Input buffer: static, not
  dynamic String" nel costruttore). Se aggiungi un nuovo tipo di variabile,
  preferisci buffer fissi.
- Le stringhe costanti di output vanno in `strings/Messages.h` e stampate
  con la macro `PRINTLN_PROGMEM` (PROGMEM su AVR risparmia RAM tenendo le
  stringhe in flash). Non introdurre nuovi literal `Serial.println("...")`
  sparsi nel `.cpp` per messaggi ripetuti/fissi: aggiungili a `Messages.h`.
- Le architetture dichiarate in `library.properties`
  (`avr,sam,samd,esp8266,esp32,stm32,all`) implicano che il codice deve
  restare portabile: evita API specifiche di una sola piattaforma senza
  guardarle con `#ifdef`.

## Versionamento

- `SerialXShell::MajorVersion` / `MinorVersion` sono costanti in
  `SerialXShell.cpp` (riga iniziale del file).
- `library.properties` ha un campo `version` separato, usato da Arduino
  Library Manager.
- Questi due numeri **non sono sincronizzati automaticamente**: se una
  modifica è abbastanza rilevante da giustificare un bump di versione,
  aggiorna entrambi i punti manualmente e verifica che siano coerenti tra
  loro prima di proporre il cambio.

## Cosa NON fare

- Non aggiungere allocazioni dinamiche (`new`/`malloc`) nel percorso di
  esecuzione del loop seriale (`shellLoop`, `handleCommand` e derivati):
  l'unica allocazione dinamica accettata è quella una tantum nel
  costruttore di `SerialXShell` per gli array di puntatori.
  Aggiungere/registrare variabili avviene di norma in `setup()`, non nel
  loop.
- Non rimuovere i guard `#if SERIALX_*` esistenti per "semplificare": sono
  lì per permettere build ridotte su microcontrollori con poca flash.
- Non modificare la firma pubblica di `SerialXShell` (costruttore,
  `addVariable`/`addFunction`/`addVirtualVariable`) senza aggiornare tutti
  e tre gli esempi in `examples/` e `tests/sketch/sketch.ino.cpp`, che la
  usano direttamente.
- Non committare `build/main.exe` se lo rigeneri: è già tracciato nel
  repo, ma è un artefatto di build — se lo aggiorni per test locali va
  bene, ma non è codice sorgente da revisionare.
- Non reintrodurre un sistema di logging comandi "abbozzato" dietro un
  flag non definito: se serve il logging, va progettato e implementato per
  intero (define in `config.h`, membri privati dedicati in
  `SerialXShell.h`, buffer circolare in `SerialXShell.cpp`), non lasciato
  a metà.

## Checklist minima prima di proporre una modifica

1. Compila con il comando g++ sopra (o `build.ps1`) senza warning nuovi.
2. Esegui `build/main` (o `build/main.exe`) e prova a mano i comandi
   `h`, `i`, un `g<tipo>`/`s<tipo>` sul tipo che hai toccato, `r` su una
   funzione registrata.
3. Se hai toccato `config.h`, ricompila anche con il flag rilevante messo
   a `0` per assicurarti che i guard `#if` non lascino riferimenti rotti.
4. Se hai aggiunto un tipo/feature, aggiorna `README.md` (sezione comandi)
   e, se pertinente, uno degli `.ino` in `examples/`.
