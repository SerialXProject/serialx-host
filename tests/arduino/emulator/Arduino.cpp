#include "Arduino.h"
#include <iomanip> // Per gestire i decimali
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

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

// std::this_thread::sleep_for è cross-platform (sostituisce Sleep() di Win32
// e usleep()/nanosleep() di POSIX con un'unica implementazione).
void delay(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

// --- Implementazione di PC_HardwareSerial indipendente dalla piattaforma ---
// (begin/end/available/read sono invece definiti nel file serial_port_win.cpp
// o serial_port_posix.cpp, scelto a compile-time in base al sistema operativo)

void PC_HardwareSerial::print(const char *s)
{
    // L'invio effettivo sulla porta seriale avviene qui sotto tramite
    // writeToPort(), implementata per piattaforma. Vedi serial_port_*.cpp.
    extern void writeToPort(void* portHandle, bool connected, const char* s);
    writeToPort(portHandle, connected, s);
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
void PC_HardwareSerial::println(char c)
{
    print(c);
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
