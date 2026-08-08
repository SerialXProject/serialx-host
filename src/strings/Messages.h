#pragma once

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
