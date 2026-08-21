// Bootstrap dell'ambiente desktop: monta lo sketch (tests/sketch/) sopra lo
// stub Arduino (tests/emulator/) e lo esegue in un loop infinito, come farebbe
// il bootloader di un vero Arduino dopo setup()/loop().
//
// Questo file non contiene logica applicativa: se vuoi cambiare cosa fa il
// "firmware" di prova, modifica tests/sketch/sketch.ino.cpp, non questo file.
#include "sketch.h"

int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
