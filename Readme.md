![SerialX Icon](banner.png)

# SerialX Host
![SerialX Host](https://img.shields.io/badge/SerialX_Host-v1.2.0-blue)

![C++](https://img.shields.io/badge/C++-11-00599C?logo=cplusplus&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-Serial-00979D?logo=arduino&logoColor=white)
![Net](https://img.shields.io/badge/Net-TCP-00599C)

Una libreria Arduino **leggera e veloce** per debug remoto, controllo e test di dispositivi embedded via porta seriale. Permette di leggere/scrivere variabili e richiamare funzioni tramite comandi testuali senza dover ricompilare il codice. 

Oltre all'utilizzo tramite porte seriali virtualizzate e arduino emulato, la libreria supporta anche la comunicazione di rete tramite il nuovo protocollo **Tcp/Net**, espandendone l'uso ai programmi C++ Desktop (non più *Arduino-Only*).

**📖 Documentazione completa:** [SerialX Docs](https://serialxproject.github.io/serialx-docs)

## ⚡ Quick Start

### Setup Minimo (Esempio Arduino)
```cpp
#include "SerialXShell.h"

int counter = 0;
SerialXShell shell(9600, 10, 5);

void reset() {
    counter = 0;
}

int getTemp() {
    return 5;
}

void setup() {
    shell.startCommunication();
    shell.addVariable(new SerialVariable("counter", counter, true));
    shell.addVirtualVariable(new SerialVirtualVariable("temp", getTemp));
    shell.addFunction(new SerialFunction("reset", reset));
}

void loop() {
    shell.shellLoop();
}
```

### Comandi da Seriale
```
h                    # Help - Lista variabili
gb led               # Leggi bool
gi counter           # Leggi int
sb led 1             # Scrivi bool
r reset              # Esegui funzione
```
* Questo è un vero e proprio linguaggio chiamato SerialX_JIT (leggi la documentazione per comprenderne l'uso e i comandi)

## 📦 Installazione

**PowerShell (Consigliato):**
```powershell
.\install.ps1
```

Consulta la [documentazione completa](serialxproject.github.io/serialx-docs) per altri modi

## 🔐 Autenticazione (Opzionale)
```cpp
SerialXShell shell(9600, 10, 5, "myKey");  // Con password
```
Da seriale: `akey myKey` prima di altri comandi.

## ⚙️ Configurazione

Modifica `src/config.h` per:
- Limiti variabili/funzioni
- Disabilitare features (batch, JSON, timestamp, logging)
- Ottimizzazioni memoria

Vedi documentazione completa per dettagli.

## 🐛 Problemi Comuni

**Variabile non si aggiorna:** Deve essere **globale**, non locale in `setup()`
