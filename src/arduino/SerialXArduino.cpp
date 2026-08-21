#ifdef ARDUINO // Serial Arduino
#include "SerialXCommunication.h"
#include <Arduino.h>

// Su Arduino, il parametro "port" viene interpretato come baud rate (es. 9600)
SerialXCommunication::SerialXCommunication(int baudRate)
    : port_(baudRate), backlog_(0), server_fd_(-1), client_fd_(0) {} // client_fd_ = 0 simula che il client è sempre "connesso" sulla seriale

SerialXCommunication::~SerialXCommunication() {
    destroy();
}

bool SerialXCommunication::begin() {
    // Inizializza la seriale hardware di Arduino con il baud rate passato
    Serial.begin(port_);
    // Attesa opzionale per la porta seriale (utile su schede come Leonardo/Micro)
    while (!Serial) {
        ; 
    }
    return true;
}

bool SerialXCommunication::open() {
    // Sulla seriale fisica la connessione è sempre aperta, 
    // quindi restituiamo semplicemente true.
    return true;
}

bool SerialXCommunication::readLine(String& outLine) {
    if (!Serial.available()) {
        return false;
    }

    // Legge la stringa fino al carattere di fine linea ('\n')
    String s = Serial.readStringUntil('\n');
    s.trim(); // Rimuove eventuali spazi o CRLF extra ('\r')
    
    outLine = s;
    return outLine.length() > 0;
}

bool SerialXCommunication::sendLine(const String& line) {
    // Invia la riga aggiungendo il ritorno a capo
    Serial.println(line.c_str());
    return true;
}

bool SerialXCommunication::sendLine() {
    Serial.println();
    return true;
}

bool SerialXCommunication::sendLine(int value) { Serial.println(value); return true; }
bool SerialXCommunication::sendLine(long value) { Serial.println(value); return true; }
bool SerialXCommunication::sendLine(unsigned int value) { Serial.println(value); return true; }
bool SerialXCommunication::sendLine(unsigned long value) { Serial.println(value); return true; }

bool SerialXCommunication::sendLine(float value, int decimals) {
    Serial.println(value, decimals);
    return true;
}

bool SerialXCommunication::sendLine(double value, int decimals) {
    Serial.println(value, decimals);
    return true;
}

bool SerialXCommunication::sendLine(char value) {
    Serial.println(value);
    return true;
}

void SerialXCommunication::close() {
    // Sulla seriale fisica non chiudiamo la porta hardware, 
    // ma possiamo segnare lo stato se necessario.
}

void SerialXCommunication::destroy() {
    // Eventuale pulizia per Arduino (es. Serial.end())
    Serial.end();
}
#endif
