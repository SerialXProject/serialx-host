#pragma once

#if defined(ARDUINO)
#include "Arduino.h"
#else
#include <string>
#include <cstdint>
#endif

struct SerialVariable {
    const char* name;              // Pointer to name string
    void* variable;                // Generic pointer to actual variable
    char type;                     // Type: 'b'=bool, 'i'=int ...
    bool canWrite;                 // Read/Write permission
    
#if SERIALX_SUPPORT_BOOL
    SerialVariable(const char* n, bool& var, bool write = true)
        : name(n), variable(&var), type('b'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_INT    
    SerialVariable(const char* n, int& var, bool write = true)
        : name(n), variable(&var), type('i'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_FLOAT    
    SerialVariable(const char* n, float& var, bool write = true)
        : name(n), variable(&var), type('f'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_STRING && defined(ARDUINO)
    SerialVariable(const char* n, String& var, bool write = true)
    : name(n), variable(&var), type('s'), canWrite(write) {}
#elif SERIALX_SUPPORT_STRING
    SerialVariable(const char* n, std::string& var, bool write = true)
    : name(n), variable(&var), type('s'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_CHAR
    SerialVariable(const char* n, char& var, bool write = true)
        : name(n), variable(&var), type('c'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_CHARSTRING
    SerialVariable(const char* n, char* var, bool write = true)
        : name(n), variable((void*)var), type('C'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_UINT8
    SerialVariable(const char* n, uint8_t& var, bool write = true)
        : name(n), variable(&var), type('u'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_UINT16
    SerialVariable(const char* n, uint16_t& var, bool write = true)
        : name(n), variable(&var), type('w'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_UINT32
    SerialVariable(const char* n, uint32_t& var, bool write = true)
        : name(n), variable(&var), type('d'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_LONG
    SerialVariable(const char* n, long& var, bool write = true)
        : name(n), variable(&var), type('l'), canWrite(write) {}
#endif

#if SERIALX_SUPPORT_DOUBLE
    SerialVariable(const char* n, double& var, bool write = true)
        : name(n), variable(&var), type('D'), canWrite(write) {}
#endif
};
