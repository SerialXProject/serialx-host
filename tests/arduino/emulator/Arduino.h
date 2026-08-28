#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

// --- COMPATIBILITÀ STRUGGERI ARDUINO AVR / FLASH ---
#define PROGMEM
#define PSTR(s) (s) // Trasforma PSTR("exit") in "exit"
#define F(s) (s)    // Trasforma F("test") in "test"

// Mappa le funzioni stringa Flash sulle funzioni C standard del PC
#define strcmp_P(s1, s2) strcmp(s1, s2)
#define strncmp_P(s1, s2, n) strncmp(s1, s2, n)

class __FlashStringHelper;

class String {
private:
    std::string val;
public:
    String() : val("") {}
    String(const char* s) : val(s ? s : "") {}
    String(const std::string& s) : val(s) {}
    String(int n) : val(std::to_string(n)) {}
    String(long n) : val(std::to_string(n)) {}
    String(unsigned int n) : val(std::to_string(n)) {}
    String(unsigned long n) : val(std::to_string(n)) {}

    void trim();

    String& operator+=(char c) {
        val += c;
        return *this;
    }

    String& operator+=(const char* s) {
        val += s;
        return *this;
    }

    String& operator+=(const String& rhs) {
        val += rhs.val;
        return *this;
    }

    String operator+(char c) const {
        return String(val + c);
    }

    String operator+(const String& rhs) const {
        return String(val + rhs.val);
    }

    // Operatore + globale sicuro e senza ambiguità
    friend String operator+(const String& lhs, const String& rhs) {
        return String(lhs.val + rhs.val);
    }

    // Metodi di utilità per far funzionare la classe con la seriale e le funzioni standard
    const char* c_str() const { return val.c_str(); }
    operator std::string() const { return val; }

    // Per permettere il .length() o simili se usati nel codice
    size_t length() const { return val.length(); }
};

// --- TIPI DI DATI ARDUINO ---
typedef uint8_t byte;

// --- MACRO E COSTANTI HARDWARE ---
#define LED_BUILTIN 13
#define OUTPUT 1
#define INPUT 0
#define LOW 0
#define HIGH 1

void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
long random(int min, int max);
void delay(int ms);

// --- EMULAZIONE HARDWARE SERIAL AVANZATA (cross-platform) ---
//
// L'accesso reale alla porta seriale (Win32 vs POSIX/termios) è delegato
// a un'implementazione specifica per piattaforma: vedi
// serial_port_win.cpp / serial_port_posix.cpp. Questa classe conosce solo
// un handle opaco (void*), così l'header resta compilabile ovunque senza
// includere windows.h o termios.h qui.
class PC_HardwareSerial
{
private:
    void* portHandle; // opaco: implementazione reale nel .cpp specifico per piattaforma
    bool connected;

public:
    PC_HardwareSerial();
    ~PC_HardwareSerial();

    void begin(int baud);
    void end(); // Risolve Serial.end()
    int available();
    char read();

    // Overload completi per print
    void print(const char *s);
    void print(const std::string &s); // Risolve l'uso di String
    void print(char c);
    void print(int n);
    void print(unsigned int n);
    void print(long n);
    void print(unsigned long n);
    void print(double f);
    void print(double f, int digits); // Risolve la precisione decimali
    void print(const __FlashStringHelper *s) { print((const char *)s); }

    // Overload completi per println
    void println(); // Risolve Serial.println() vuoto
    void println(const char *s);
    void println(char c);
    void println(const std::string &s); // Risolve l'uso di String
    void println(int n);
    void println(unsigned int n);
    void println(long n);
    void println(unsigned long n);
    void println(double f);
    void println(double f, int digits); // Risolve la precisione decimali
    void println(const __FlashStringHelper *s) { println((const char *)s); }

    explicit operator bool() const;
    String readStringUntil(char terminator);
    void print(const String &s);
    void println(const String &s);
};

extern PC_HardwareSerial Serial;
