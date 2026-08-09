# Test dell'ambiente desktop

Questa cartella permette di compilare ed eseguire la libreria SerialX Shell
su un PC desktop (Windows, Linux o macOS), senza un Arduino reale collegato.
Utile per lo sviluppo e il debug: offre un logging completo dell'istanza
mock di `Serial`, cosa non sempre facile da ottenere su un Arduino reale.

## Struttura

```
tests/
  emulator/               # Stub dell'API Arduino: non si tocca quasi mai
    Arduino.h              # Interfaccia (Serial, pinMode, digitalWrite, ...)
    Arduino.cpp             # Parti comuni a tutte le piattaforme
    serial_port_win.cpp     # Accesso alla porta seriale reale (Windows)
    serial_port_posix.cpp   # Accesso alla porta seriale reale (Linux/macOS)
  sketch/
    sketch.ino.cpp          # Lo "sketch" (firmware) di arduino
  runner/
    main.cpp                 # Bootstrap desktop: chiama setup() poi loop() in ciclo. Non contiene logica applicativa.
```

Solo `serial_port_win.cpp` **oppure** `serial_port_posix.cpp` viene
compilato, in base al sistema operativo: se ne occupa `build.ps1`
automaticamente.

## Configurazione dell'ambiente

Serve un emulatore di porta seriale virtuale, che crea una coppia di porte
interconnesse: una la userà l'eseguibile compilato da questa cartella,
l'altra il tuo terminale seriale (es. PuTTY, minicom, screen).

### Windows: `com0com`

1.  Scarica e installa [`com0com`](https://sourceforge.net/projects/com0com/)
    (o un'alternativa simile).
2.  Configura una coppia di porte seriali virtuali (es. `COM3` <-> `COM4`).
3.  Imposta la variabile d'ambiente `SERIALX_TEST_PORT` con il nome della
    porta che userà l'eseguibile, ad es.:
    ```powershell
    $env:SERIALX_TEST_PORT = "COM4"
    ```
    Se non la imposti, viene usato `COM4` come default.
4.  Usa un terminale seriale per connetterti all'altra porta della coppia
    (es. `COM3`). In questo modo potrai inviare comandi all'istanza mock di
    SerialX Shell e ricevere le risposte.

### Linux / macOS: `socat`

Su Linux e macOS puoi ottenere lo stesso comportamento utilizzando **`socat`**, uno strumento versatile che crea coppie di pseudoterminali collegati tra loro:

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

Se non lo hai già installato sul tuo sistema, puoi farlo facilmente con il package manager della tua distro:

* **Ubuntu / Debian:**
```bash
sudo apt install socat
```

* **Arch Linux:**
```bash
sudo pacman -S socat
```

L'esecuzione del comando stamperà a schermo due percorsi di pseudoterminali (ad esempio `/dev/pts/3` e `/dev/pts/4`).

A questo punto, ti basterà impostare uno dei due percorsi come porta seriale per il tuo eseguibile:

```bash
export SERIALX_TEST_PORT=/dev/pts/4
```

Se non la imposti, viene usato `/dev/pts/4` come default — adatta comunque
al percorso stampato da `socat`, che cambia ad ogni esecuzione.

Connettiti poi all'altro pseudoterminale (es. `/dev/pts/3`) con un terminale
seriale, ad esempio:

```bash
minicom -D /dev/pts/3
# oppure
screen /dev/pts/3
```

(Su Linux è disponibile anche `tty0tty`, un modulo del kernel che crea
coppie di dispositivi `/dev/ttySXX`, come alternativa a `socat`.)

## Utilizzo

Dopo aver configurato le porte seriali virtuali e la variabile
`SERIALX_TEST_PORT`:

1.  Compila con `build.ps1` (richiede PowerShell — su Linux/macOS si
    installa con `pwsh`, disponibile nei repository delle principali
    distribuzioni). Genera `build/main.exe` su Windows, `build/main` su
    Linux/macOS.
    ```powershell
    pwsh ./build.ps1
    ```
2.  Esegui l'eseguibile compilato:
    ```powershell
    ./build/main        # Linux/macOS
    ./build/main.exe     # Windows
    ```
3.  Connettiti con un terminale seriale all'altra porta virtuale e inizia a
    inviare comandi (es. `h`, `gi temperature`). L'output del programma
    mostrerà il logging dettagliato delle interazioni seriali.

## Modificare lo sketch di prova

Per aggiungere/rimuovere variabili, funzioni o variabili virtuali di prova,
modifica solo `tests/sketch/sketch.ino.cpp` — è un normale sketch Arduino,
non serve toccare `tests/emulator/` né `tests/runner/`.
