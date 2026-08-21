// Implementazione Win32 dell'accesso alla porta seriale per PC_HardwareSerial.
// Compilato SOLO su Windows (vedi build.ps1). Su Linux/macOS viene compilato
// serial_port_posix.cpp al suo posto: l'interfaccia pubblica in Arduino.h
// resta identica su entrambe le piattaforme.
#include "Arduino.h"
#include <windows.h>
#include <iostream>
#include <cstdlib>

using namespace std;

// Nome della porta configurabile via variabile d'ambiente SERIALX_TEST_PORT,
// ad es. SERIALX_TEST_PORT=COM4. Se non impostata, usa COM4 come default
// storico di questo progetto.
static std::string resolvePortName()
{
    const char* envPort = std::getenv("SERIALX_TEST_PORT");
    if (envPort && envPort[0] != '\0') {
        return std::string(envPort);
    }
    return "COM4";
}

PC_HardwareSerial::PC_HardwareSerial() : portHandle(INVALID_HANDLE_VALUE), connected(false) {}
PC_HardwareSerial::~PC_HardwareSerial() { end(); }

void PC_HardwareSerial::end()
{
    if (connected && portHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle((HANDLE)portHandle);
        connected = false;
        portHandle = INVALID_HANDLE_VALUE;
    }
}

void PC_HardwareSerial::begin(int baud)
{
    std::string portName = resolvePortName();
    std::string fullPath = "\\\\.\\" + portName;

    HANDLE h = CreateFileA(fullPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE)
    {
        DCB dcb = {0};
        dcb.DCBlength = sizeof(dcb);
        if (GetCommState(h, &dcb))
        {
            dcb.BaudRate = baud;
            dcb.ByteSize = 8;
            dcb.StopBits = ONESTOPBIT;
            dcb.Parity = NOPARITY;
            if (SetCommState(h, &dcb))
            {
                portHandle = (void*)h;
                connected = true;
                cout << "[SERIAL] Connesso a " << portName << " (" << baud << " baud)" << endl;
                return;
            }
        }
        CloseHandle(h);
    }
    cout << "[SERIAL ERROR] Impossibile aprire " << portName
         << ". Imposta SERIALX_TEST_PORT per usarne un'altra." << endl;
}

int PC_HardwareSerial::available()
{
    if (!connected)
        return 0;
    DWORD errors;
    COMSTAT status;
    ClearCommError((HANDLE)portHandle, &errors, &status);
    return status.cbInQue;
}

char PC_HardwareSerial::read()
{
    char c = 0;
    DWORD bytesRead;
    if (connected)
    {
        ReadFile((HANDLE)portHandle, &c, 1, &bytesRead, NULL);
        cout << "[SERIAL] Read: " << c << endl;
    }
    return c;
}

// Usata da PC_HardwareSerial::print() in Arduino.cpp per inviare byte
// sulla porta reale, indipendentemente dalla piattaforma.
void writeToPort(void* portHandle, bool connected, const char* s)
{
    if (!connected)
        return;
    DWORD bytesWritten;
    WriteFile((HANDLE)portHandle, s, (DWORD)strlen(s), &bytesWritten, NULL);
    cout << "[SERIAL] Write: " << s << endl;
}
