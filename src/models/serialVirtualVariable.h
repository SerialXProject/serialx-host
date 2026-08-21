#pragma once

#if defined(ARDUINO)
#include "Arduino.h"
#else
#include <string>
#include <cstdint>
#endif

// Virtual Variable - read-only variable backed by a getter function
// Allows exposing computed/dynamic values as if they were actual variables
struct SerialVirtualVariable {
    const char* name;              // Pointer to name string
    void* getter;                  // Generic pointer to getter function
    char type;                     // Type: 'b'=bool, 'i'=int ...
    
#if SERIALX_SUPPORT_BOOL
    SerialVirtualVariable(const char* n, bool (*g)())
        : name(n), getter((void*)g), type('b') {}
#endif

#if SERIALX_SUPPORT_INT    
    SerialVirtualVariable(const char* n, int (*g)())
        : name(n), getter((void*)g), type('i') {}
#endif

#if SERIALX_SUPPORT_FLOAT    
    SerialVirtualVariable(const char* n, float (*g)())
        : name(n), getter((void*)g), type('f') {}
#endif

#if SERIALX_SUPPORT_STRING && defined(ARDUINO)
    SerialVirtualVariable(const char* n, String (*g)())
        : name(n), getter((void*)g), type('s') {}
#elif SERIALX_SUPPORT_STRING
    SerialVirtualVariable(const char* n, std::string (*g)())
        : name(n), getter((void*)g), type('s') {}
#endif

#if SERIALX_SUPPORT_CHAR
    SerialVirtualVariable(const char* n, char (*g)())
        : name(n), getter((void*)g), type('c') {}
#endif

#if SERIALX_SUPPORT_CHARSTRING
    SerialVirtualVariable(const char* n, char* (*g)())
        : name(n), getter((void*)g), type('C') {}
#endif

#if SERIALX_SUPPORT_UINT8
    SerialVirtualVariable(const char* n, uint8_t (*g)())
        : name(n), getter((void*)g), type('u') {}
#endif

#if SERIALX_SUPPORT_UINT16
    SerialVirtualVariable(const char* n, uint16_t (*g)())
        : name(n), getter((void*)g), type('w') {}
#endif

#if SERIALX_SUPPORT_UINT32
    SerialVirtualVariable(const char* n, uint32_t (*g)())
        : name(n), getter((void*)g), type('d') {}
#endif

#if SERIALX_SUPPORT_LONG
    SerialVirtualVariable(const char* n, long (*g)())
        : name(n), getter((void*)g), type('l') {}
#endif

#if SERIALX_SUPPORT_DOUBLE
    SerialVirtualVariable(const char* n, double (*g)())
        : name(n), getter((void*)g), type('D') {}
#endif
};
