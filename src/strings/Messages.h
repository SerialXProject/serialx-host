#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#else
// PROGMEM non esiste fuori da AVR/Arduino: la neutralizziamo
#ifndef PROGMEM
#define PROGMEM
#endif
#include <cstring>
#endif

namespace Strings {
    // System messages
    const char MSG_READY[] PROGMEM           = "Mini shell ready";
    const char MSG_OK[] PROGMEM              = "OK";
    const char MSG_ERROR[] PROGMEM           = "E|";
    const char MSG_UNKNOWN_CMD[] PROGMEM     = "E|Unknown command";
    const char MSG_UNKNOWN_VAR[] PROGMEM     = "E|Variable not found";
    const char MSG_AUTH_FAIL[] PROGMEM       = "E|AccessKey not valid";
    const char MSG_AUTH_SUCCESS[] PROGMEM    = "OK|Session authenticated";
    const char MSG_NOT_AUTHORIZED[] PROGMEM  = "E|Access not authorized";
    const char MSG_INVALID_FORMAT[] PROGMEM  = "E|Invalid format";
    const char MSG_READ_ONLY[] PROGMEM       = "E|Read-only";
}

// Copia una stringa (in PROGMEM su AVR, in RAM altrove) in un buffer locale.
// Ritorna il buffer stesso per comodità d'uso inline.
inline char* loadProgmemString(const char* src, char* dest, size_t destSize) {
#if defined(ARDUINO_ARCH_AVR)
    strncpy_P(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
#else
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
#endif
    return dest;
}

// Helper macro for Arduino boards
#if defined(ARDUINO_ARCH_AVR)
  #define PRINTLN_PROGMEM(s) Serial.println((__FlashStringHelper*)(s))
  #define PRINT_PROGMEM(s) Serial.print((__FlashStringHelper*)(s))
#elif defined(ARDUINO)
  #define PRINTLN_PROGMEM(s) Serial.println(s)
  #define PRINT_PROGMEM(s) Serial.print(s)
#else
  #define PRINTLN_PROGMEM(s) communication.sendLine(s)
#endif

/*#pragma once

namespace Strings {
  // System messages
  const char MSG_READY[] PROGMEM           = "Mini shell ready";
  const char MSG_OK[] PROGMEM              = "OK";
  const char MSG_ERROR[] PROGMEM           = "E|";
  const char MSG_UNKNOWN_CMD[] PROGMEM     = "E|Unknown command";
  const char MSG_UNKNOWN_VAR[] PROGMEM     = "E|Variable not found";
  const char MSG_AUTH_FAIL[] PROGMEM       = "E|AccessKey not valid";
  const char MSG_AUTH_SUCCESS[] PROGMEM    = "OK|Session authenticated";
  const char MSG_NOT_AUTHORIZED[] PROGMEM  = "E|Access not authorized";
  const char MSG_INVALID_FORMAT[] PROGMEM  = "E|Invalid format";
  const char MSG_READ_ONLY[] PROGMEM       = "E|Read-only";
}

// Helper macro for Arduino boards
#if defined(ARDUINO_ARCH_AVR)
  #define PRINTLN_PROGMEM(s) Serial.println((__FlashStringHelper*)(s))
  #define PRINT_PROGMEM(s) Serial.print((__FlashStringHelper*)(s))
#else
  #define PRINTLN_PROGMEM(s) Serial.println(s)
  #define PRINT_PROGMEM(s) Serial.print(s)
#endif
*/