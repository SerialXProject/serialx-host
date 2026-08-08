#pragma once
#include <Arduino.h>

struct SerialFunction {
    const char* name;
    void (*func)();
    
    SerialFunction(const char* n, void (*f)())
        : name(n), func(f) {}
};
