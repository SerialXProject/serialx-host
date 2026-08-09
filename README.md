![SerialX Icon](banner.png)

# SerialX Shell

Una libreria Arduino **leggera e veloce** per debug remoto, controllo e test di dispositivi embedded via porta seriale. Leggi/scrivi variabili e richiama funzioni tramite comandi testuali senza ricompilare.

**📖 Documentazione completa:** [SerialX Docs](https://serialx.github.io/serialx-docs)

## ⚡ Quick Start

### Setup Minimo
```cpp
#include "SerialXShell.h"

int counter = 0;
SerialXShell shell(9600, 10, 5);

void reset()
{
    counter = 0;
}

int getTemp()
{
    return 5;
}

void setup() {
    shell.startCommunication();
    shell.addVariable(new SerialVariable("counter", counter, true));
    shell.addVirtualVariable(new SerialVirtualVariable("temp", getTemp))
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
