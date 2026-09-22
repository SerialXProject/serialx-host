#include "SerialXShell.h"
#include <iostream>
#include <string>

using namespace std;

// Variabili di stato
bool bool_var = false;
int int_num = 0;
float float_num = 0;
string str_var = "ciao"; // Usiamo std::string al posto della classe String di Arduino

// Funzioni simulate (sul Pi Zero qui potrai pilotare i GPIO reali)
void ActiveLed() {
    cout << "[LED] Acceso\n";
}

void DisactiveLed() {
    cout << "[LED] Spento\n";
}

int main() {
    // 1. Inizializzazione della Shell (passiamo la porta TCP es. 8080 al posto del baudrate)
    // I parametri numerici successivi gestiscono le dimensioni massime di variabili, funzioni, ecc.
    SerialXShell shell(8080, 20, 20, 20, "test");

    // 2. Configurazione iniziale (equivalente del setup() di Arduino)
    // (Qui non serve pinMode/digitalWrite di Arduino, gestirai i pin con librerie Linux dedicate se ti servono)
    
    // Avvia la comunicazione di rete (apre il socket TCP Server e si mette in ascolto)
    shell.startCommunication();

    // Registrazione di variabili e funzioni
    shell.addVariable(new SerialVariable("bool_var", &bool_var, true));
    shell.addVariable(new SerialVariable("int_num", &int_num, true));
    shell.addVariable(new SerialVariable("float_num", &float_num, true));
    shell.addVariable(new SerialVariable("string", &str_var, true));
    
    shell.addFunction(new SerialFunction("led_on", ActiveLed));
    shell.addFunction(new SerialFunction("led_off", DisactiveLed));

    cout << "SerialX-Net avviato con successo. In attesa di comandi...\n";

    // 3. Loop principale (equivalente del loop() di Arduino)
    while (true) {
        shell.shellLoop(); // Gestisce la ricezione dei comandi dal socket di rete in modo continuo
    }

    // Chiusura (anche se il while(true) non ci arriverà mai, pulisce la risorsa)
    shell.closeCommunication();
    return 0;
}