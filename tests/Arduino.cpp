#include "Arduino.h"
#include <iomanip> // Per gestire i decimali
#include <sstream>
#include <iostream>

using namespace std;

PC_HardwareSerial Serial;

void pinMode(int pin, int mode)
{
    cout << "[PIN MODE] Pin " << pin << " impostato a " << (mode == OUTPUT ? "OUTPUT" : "INPUT") << endl;
}
void digitalWrite(int pin, int val)
{
    cout << "[DIGITAL WRITE] Pin " << pin << " impostato a " << val << endl;
}
long random(int min, int max) { return max <= min ? min : min + (rand() % (max - min)); }
void delay(int ms) { Sleep(ms); }

PC_HardwareSerial::PC_HardwareSerial() : hSerial(INVALID_HANDLE_VALUE), connected(false) {}
PC_HardwareSerial::~PC_HardwareSerial() { end(); }

void PC_HardwareSerial::end()
{
    if (connected && hSerial != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSerial);
        connected = false;
        hSerial = INVALID_HANDLE_VALUE;
    }
}

void PC_HardwareSerial::begin(int baud)
{
    hSerial = CreateFileA("\\\\.\\COM4", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        DCB dcb = {0};
        dcb.DCBlength = sizeof(dcb);
        if (GetCommState(hSerial, &dcb))
        {
            dcb.BaudRate = baud;
            dcb.ByteSize = 8;
            dcb.StopBits = ONESTOPBIT;
            dcb.Parity = NOPARITY;
            if (SetCommState(hSerial, &dcb))
            {
                connected = true;
                cout << "[SERIAL] Connesso a COM10 (" << baud << " baud)" << endl;
                return;
            }
        }
    }
    cout << "[SERIAL ERROR] Impossibile aprire COM10." << endl;
}

int PC_HardwareSerial::available()
{
    if (!connected)
        return 0;
    DWORD errors;
    COMSTAT status;
    ClearCommError(hSerial, &errors, &status);
    return status.cbInQue;
}

char PC_HardwareSerial::read()
{
    char c = 0;
    DWORD bytesRead;
    if (connected)
    {
        ReadFile(hSerial, &c, 1, &bytesRead, NULL);
        cout << "[SERIAL] Read: " << c << endl;
    }
    return c;
}

void PC_HardwareSerial::print(const char *s)
{
    DWORD bytesWritten;
    if (connected)
    {
        WriteFile(hSerial, s, strlen(s), &bytesWritten, NULL);
        cout << "[SERIAL] Write: " << s << endl;
    }
}

void PC_HardwareSerial::print(const std::string &s) { print(s.c_str()); }
void PC_HardwareSerial::print(int n) { print(to_string(n).c_str()); }
void PC_HardwareSerial::print(char c) { print(string(1, c).c_str()); }
void PC_HardwareSerial::print(unsigned int n) { print(to_string(n).c_str()); }
void PC_HardwareSerial::print(long n) { print(to_string(n).c_str()); }
void PC_HardwareSerial::print(unsigned long n) { print(to_string(n).c_str()); }
void PC_HardwareSerial::print(double f) { print(f, 2); }

void PC_HardwareSerial::print(double f, int digits)
{
    stringstream ss;
    ss << std::fixed << std::setprecision(digits) << f;
    print(ss.str().c_str());
}

void PC_HardwareSerial::println() { print("\r\n"); }
void PC_HardwareSerial::println(const char *s)
{
    print(s);
    print("\r\n");
}
void PC_HardwareSerial::println(const string &s)
{
    print(s.c_str());
    print("\r\n");
}
void PC_HardwareSerial::println(int n)
{
    print(n);
    print("\r\n");
}
void PC_HardwareSerial::println(unsigned int n)
{
    print(n);
    print("\r\n");
}
void PC_HardwareSerial::println(long n)
{
    print(n);
    print("\r\n");
}
void PC_HardwareSerial::println(unsigned long n)
{
    print(n);
    print("\r\n");
}
void PC_HardwareSerial::println(double f)
{
    print(f);
    print("\r\n");
}
void PC_HardwareSerial::println(double f, int digits)
{
    print(f, digits);
    print("\r\n");
}