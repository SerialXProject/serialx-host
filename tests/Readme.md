# Test dell'ambiente desktop

Questa cartella contiene gli stub dell'API Arduino (`Arduino.h`/`Arduino.cpp`) e un programma `main.cpp` che consente di compilare ed eseguire la libreria SerialX Shell su un PC desktop. Questo approccio è utile per lo sviluppo e il debug in quanto offre un logging completo dell'istanza mock di Serial, cosa non sempre facile da ottenere su un Arduino reale.

## Configurazione dell'ambiente

### Windows: `com0com`

Su Windows, per simulare una porta seriale hardware è necessario installare un emulatore di porta seriale virtuale come `com0com`. Questo software crea una coppia di porte seriali virtuali interconnesse (es. COM3 <-> COM4). Potrai configurare il tuo ambiente di test per usare una di queste porte virtuali, e il tuo terminale seriale (o un programma come PuTTY) per connettersi all'altra.

1.  Scarica e installa `com0com` (o un'alternativa simile).
2.  Configura una coppia di porte seriali virtuali.
3.  Quando esegui `build/main.exe`, la libreria SerialX Shell si connetterà automaticamente a `SERIALX_DEBUG_PORT` (definita in `config.h`, di default `COM0`). Assicurati che una delle porte create con `com0com` corrisponda a questo valore (es. `COM0` che punta a `COM3`).
4.  Usa un terminale seriale per connetterti all'altra porta della coppia (es. `COM4`). In questo modo, potrai inviare comandi all'istanza mock di SerialX Shell e ricevere le risposte.

### Linux: `socat` o `tty0tty`

Su Linux, puoi ottenere un comportamento simile usando `socat` o `tty0tty` per creare coppie di pseudoterminali.

**Con `socat`:**

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

Questo comando stamperà due percorsi di pseudoterminali (es. `/dev/pts/X` e `/dev/pts/Y`). Puoi configurare il tuo programma di test per connettersi a uno e il tuo terminale seriale all'altro.

**Con `tty0tty`:**

`tty0tty` è un modulo del kernel che crea coppie di dispositivi `/dev/ttySXX`. Dovrai installarlo e caricarlo.

## Utilizzo

Dopo aver configurato le porte seriali virtuali:

1.  Compila il progetto per l'ambiente desktop usando `build.ps1` (o il comando `g++` equivalente menzionato in `AGENTS.md`). Questo genererà `build/main.exe` (su Windows) o `build/main` (su Linux).
2.  Esegui il programma compilato: `pwsh -Command "& \"${workspaceFolder}/build/main$([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows) ? '.exe' : '')\""` o manualmente `build/main.exe`.
3.  Connettiti con un terminale seriale all'altra porta virtuale e inizia a inviare comandi (es. `h`, `gi temperature`). L'output del programma `main.exe` mostrerà il logging dettagliato delle interazioni seriali.
