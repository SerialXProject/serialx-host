#include "SerialXShell.h"
#include <string.h>
#include <ctype.h>

const int SerialXShell::SerialX_MajorVersion = 1;
const int SerialXShell::SerialX_MinorVersion = 6;
const int SerialXShell::SerialXJIT_MajorVersion = 1;
const int SerialXShell::SerialXJIT_MinorVersion = 6;

// Constructor (forse usare define)
SerialXShell::SerialXShell(int baud, int maxVars, int maxFuncs, int maxVirtualVars, const char* accessPassKey, const char* deviceNameCustom, const char* softwareVersionCustom)
    : communication(baud), isCommunicationOpen(false), isAccessUnlocked(false),
      variableCount(0), maxVariables(maxVars),
      virtualVariableCount(0), maxVirtualVariables(maxVirtualVars),
      functionCount(0), maxFunctions(maxFuncs),
      inputPos(0) {
    
    // Allocate SINGLE unified array for all variables
    variables = new SerialVariable*[maxVariables];
    virtualVariables = new SerialVirtualVariable*[maxVirtualVariables];
    functions = new SerialFunction*[maxFunctions];
    
    const char* devSrc = deviceNameCustom ? deviceNameCustom : "Arduino";
    strncpy(deviceName, devSrc, sizeof(deviceName) - 1);
    deviceName[sizeof(deviceName) - 1] = '\0';

    const char* swSrc = softwareVersionCustom ? softwareVersionCustom : "Custom";
    strncpy(softwareVersion, swSrc, sizeof(softwareVersion) - 1);
    softwareVersion[sizeof(softwareVersion) - 1] = '\0';

    // Copy access key safely
    accessKey[0] = '\0';
    if (accessPassKey) {
        strncpy(accessKey, accessPassKey, SERIALX_ACCESS_KEY_SIZE - 1);
        accessKey[SERIALX_ACCESS_KEY_SIZE - 1] = '\0';
    }
}

SerialXShell::~SerialXShell() {
    delete[] variables;
    delete[] virtualVariables;
    delete[] functions;
}

void SerialXShell::startCommunication() {
    if (isCommunicationOpen) return;
    
    communication.begin();
    communication.open();

    isCommunicationOpen = true;
    isAccessUnlocked = false;
    inputPos = 0;
    
    PRINTLN_PROGMEM(Strings::MSG_READY);
}

void SerialXShell::closeCommunication() {
    if (!isCommunicationOpen) return;
    
    communication.close();
    isAccessUnlocked = false;
    isCommunicationOpen = false;
}

void SerialXShell::addVariable(SerialVariable* var) {
    if (variableCount < maxVariables && var) {
        variables[variableCount++] = var;
    }
}

void SerialXShell::addFunction(SerialFunction* f) {
    if (functionCount < maxFunctions && f) {
        functions[functionCount++] = f;
    }
}

void SerialXShell::addVirtualVariable(SerialVirtualVariable* vVar) {
    if (virtualVariableCount < maxVirtualVariables && vVar) {
        virtualVariables[virtualVariableCount++] = vVar;
    }
}

// ===== LOOKUP METHODS =====
SerialVariable* SerialXShell::findVariable(const char* name) {
    if (!name) return nullptr;
    
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i]->name, name) == 0) {
            return variables[i];
        }
    }
    return nullptr;
}

SerialFunction* SerialXShell::findFunction(const char* name) {
    if (!name) return nullptr;
    
    for (int i = 0; i < functionCount; i++) {
        if (strcmp(functions[i]->name, name) == 0)
            return functions[i];
    }
    return nullptr;
}

SerialVirtualVariable* SerialXShell::findVirtualVariable(const char* name) {
    if (!name) return nullptr;
    
    for (int i = 0; i < virtualVariableCount; i++) {
        if (strcmp(virtualVariables[i]->name, name) == 0) {
            return virtualVariables[i];
        }
    }
    return nullptr;
}

// ===== COMMAND HANDLERS =====
void SerialXShell::handleCommand(const char* cmd) {
    if (!cmd || strlen(cmd) == 0) return;

#if defined(ARDUINO_ARCH_AVR)
    if (strcmp_P(cmd, PSTR("exit")) == 0)
#else
    if (strcmp(cmd, "exit") == 0)
#endif
    {
        closeCommunication();
        return;
    }

#if defined(ARDUINO_ARCH_AVR)
    if (strcmp_P(cmd, PSTR("i")) == 0)
#else
    if (strcmp(cmd, "i") == 0) 
#endif
    { // Info
        printInfo();
        return;
    }

#if defined(ARDUINO_ARCH_AVR)
    if (strcmp_P(cmd, PSTR("isAuthActive")) == 0) 
#else
    if (strcmp(cmd, "isAuthActive") == 0) 
#endif
    {
#if SERIALX_ENABLE_AUTH
    communication.sendLine(accessKey[0] != '\0' ? "1" : "0");
#else
    communication.sendLine("0");
#endif
    return;
}
    
// Check authentication FIRST
#if SERIALX_ENABLE_AUTH
    if (accessKey[0] != '\0' && !isAccessUnlocked) {
    #if defined(ARDUINO_ARCH_AVR)
        if (strncmp_P(cmd, PSTR("akey "), 5) == 0) 
    #else
        if (strncmp(cmd, "akey ", 5) == 0)
    #endif
        {
            authenticate(cmd + 5);
            return;
        }
        PRINTLN_PROGMEM(Strings::MSG_NOT_AUTHORIZED);
        return;
    }
#endif

    // Compact command parser using single character
    char op = cmd[0];
    
    switch (op) {
        case 'g':  // GET
            getVariable(cmd);
            break;
        case 's':  // SET
            setVariable(cmd);
            break;
        case 'r':  // RUN function
            runFunction(cmd + 2);
            break;
        case 'h':  // HELP
            printHelp();
            break;
        case 'i': // INFO
            printInfo();
            break;
#if SERIALX_ENABLE_AUTH
        case 'a':  // AUTH
            if (strncmp(cmd, "akey ", 5) == 0) {
                authenticate(cmd + 5);
            }
            break;
#endif
#if SERIALX_ENABLE_BATCH_CMD
        case 'B':  // BATCH
            if (cmd[1] == ' ') {
                executeBatchCommand(cmd + 2);
            }
            break;
#endif
#if SERIALX_ENABLE_JSON_OUTPUT
        case 'J':  // JSON output
            if (cmd[1] == ' ') {
                outputVariablesJSON(cmd + 2);
            }
            break;
#endif
#if SERIALX_ENABLE_TIMESTAMP
        case 'T':  // TIMESTAMP
            if (strlen(cmd) == 1) {
                printTimestamp();
            } else if (cmd[1] == 'G' && cmd[2] == ' ') {
                printVariableWithTimestamp(cmd + 3);
            }
            break;
#endif
        default:
            PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_CMD);
    }
}

void SerialXShell::getVariable(const char* cmd) {
    // Format: g<type> <name>        (for real variables)
    //         g<type>v<name>       (for virtual variables)
    // Example: gi myInt              (real variable)
    //          givtemperature       (virtual variable)
    
    if (strlen(cmd) < 4 || (cmd[2] != ' ' && cmd[2] != 'v')) {
        PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
        return;
    }
    
    // Check if it's a virtual variable request (cmd[1] == 'v')
    if (cmd[2] == 'v') {
        // Format: g<type>v<name>
        
        char type = cmd[1];
        const char* name = cmd + 3;
        
        SerialVirtualVariable* vVar = findVirtualVariable(name);
        if (!vVar) {
            PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_VAR);
            return;
        }
        
        // Type mismatch check
        if (vVar->type != type) {
            PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
            return;
        }
        
        // Call getter function based on type and output value
        switch (vVar->type) {
#if SERIALX_SUPPORT_BOOL
            case 'b': {
                bool (*getter)() = (bool (*)())vVar->getter;
                communication.sendLine(getter() ? 1 : 0);
                break;
            }
#endif
#if SERIALX_SUPPORT_INT
            case 'i': {
                int (*getter)() = (int (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_FLOAT
            case 'f': {
                float (*getter)() = (float (*)())vVar->getter;
                communication.sendLine(getter(), 2);
                break;
            }
#endif
#if SERIALX_SUPPORT_STRING
            case 's': {
                StringType (*getter)() = (StringType (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_CHAR
            case 'c': {
                char (*getter)() = (char (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_UINT8
            case 'u': {
                uint8_t (*getter)() = (uint8_t (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_UINT16
            case 'w': {
                uint16_t (*getter)() = (uint16_t (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_UINT32
            case 'd': {
                uint32_t (*getter)() = (uint32_t (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_LONG
            case 'l': {
                long (*getter)() = (long (*)())vVar->getter;
                communication.sendLine(getter());
                break;
            }
#endif
#if SERIALX_SUPPORT_DOUBLE
            case 'D': {
                double (*getter)() = (double (*)())vVar->getter;
                communication.sendLine(getter(), SERIALX_DOUBLE_PRECISION);
                break;
            }
#endif
        }
        return;
    }
    
    // Otherwise, search for real variable
    char type = cmd[1];
    const char* name = cmd + 3;
    
    SerialVariable* var = findVariable(name);
    if (!var) {
        PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_VAR);
        return;
    }
    
    // Type mismatch check
    if (var->type != type) {
        PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
        return;
    }
    
    // Output value based on type
    switch (type) {
#if SERIALX_SUPPORT_BOOL
        case 'b': {
            bool* val = (bool*)var->variable;
            communication.sendLine(*val ? 1 : 0);
            break;
        }
#endif
#if SERIALX_SUPPORT_INT
        case 'i': {
            int* val = (int*)var->variable;
            communication.sendLine(*val);
            break;
        }
#endif
#if SERIALX_SUPPORT_FLOAT
        case 'f': {
            float* val = (float*)var->variable;
            communication.sendLine(*val, 2);
            break;
        }
#endif
#if SERIALX_SUPPORT_STRING
    case 's': {
        StringType* val = (StringType*)var->variable;
        communication.sendLine(*val); // Corretto: usa l'oggetto String di Arduino
        break;
    }
#endif
#if SERIALX_SUPPORT_CHAR
    case 'c': {
        char* val = (char*)var->variable;
        communication.sendLine(*val);
        break;
    }
#endif
#if SERIALX_SUPPORT_CHARSTRING
        case 'C': {
            char* val = (char*)var->variable;
            communication.sendLine(val);
            break;
        }
#endif
#if SERIALX_SUPPORT_UINT8
        case 'u': {
            uint8_t* val = (uint8_t*)var->variable;
            communication.sendLine(*val);
            break;
        }
#endif
#if SERIALX_SUPPORT_UINT16
        case 'w': {
            uint16_t* val = (uint16_t*)var->variable;
            communication.sendLine(*val);
            break;
        }
#endif
#if SERIALX_SUPPORT_UINT32
        case 'd': {
            uint32_t* val = (uint32_t*)var->variable;
            communication.sendLine(*val);
            break;
        }
#endif
#if SERIALX_SUPPORT_LONG
        case 'l': {
            long* val = (long*)var->variable;
            communication.sendLine(*val);
            break;
        }
#endif
#if SERIALX_SUPPORT_DOUBLE
        case 'D': {
            double* val = (double*)var->variable;
            communication.sendLine(*val, 2);
            break;
        }
#endif
    }
}

void SerialXShell::setVariable(const char* cmd) {
    // Format: s<type> <name> <value>
    // Example: si myInt 42
    
    if (strlen(cmd) < 5 || cmd[2] != ' ') {
        PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
        return;
    }
    
    char type = cmd[1];
    const char* rest = cmd + 3;
    
    // Find space between name and value
    const char* space = strchr(rest, ' ');
    if (!space) {
        PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
        return;
    }
    
    // Extract name
    int nameLen = space - rest;
    if (nameLen >= SERIALX_VAR_NAME_SIZE) {
        PRINTLN_PROGMEM(Strings::MSG_INVALID_FORMAT);
        return;
    }
    
    char nameBuf[SERIALX_VAR_NAME_SIZE];
    strncpy(nameBuf, rest, nameLen);
    nameBuf[nameLen] = '\0';
    
    const char* value = space + 1;
    
    SerialVariable* var = findVariable(nameBuf);
    if (!var) {
        PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_VAR);
        return;
    }
    
    if (!var->canWrite) {
        PRINTLN_PROGMEM(Strings::MSG_READ_ONLY);
        return;
    }
    
    // Set value based on type
    switch (type) {
#if SERIALX_SUPPORT_BOOL
        case 'b': {
            bool* val = (bool*)var->variable;
            *val = atoi(value) != 0;
            break;
        }
#endif
#if SERIALX_SUPPORT_INT
        case 'i': {
            int* val = (int*)var->variable;
            *val = atoi(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_FLOAT
        case 'f': {
            float* val = (float*)var->variable;
            *val = atof(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_STRING
case 's': {  
    StringType* val = (StringType*)var->variable;
    *val = StringType(value);
    break;
}
#endif
#if SERIALX_SUPPORT_CHAR
case 'c': {
    char* val = (char*)var->variable;
    *val = value[0];
    break;
}
#endif
#if SERIALX_SUPPORT_CHARSTRING
case 'C': {
    char* val = (char*)var->variable;
    strncpy(val, value, SERIALX_VAR_NAME_SIZE - 1);
    val[SERIALX_VAR_NAME_SIZE - 1] = '\0';
    break;
}
#endif
#if SERIALX_SUPPORT_UINT8
        case 'u': {
            uint8_t* val = (uint8_t*)var->variable;
            *val = (uint8_t)atoi(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_UINT16
        case 'w': {
            uint16_t* val = (uint16_t*)var->variable;
            *val = (uint16_t)atoi(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_UINT32
        case 'd': {
            uint32_t* val = (uint32_t*)var->variable;
            *val = (uint32_t)atol(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_LONG
        case 'l': {
            long* val = (long*)var->variable;
            *val = atol(value);
            break;
        }
#endif
#if SERIALX_SUPPORT_DOUBLE
        case 'D': {
            double* val = (double*)var->variable;
            *val = atof(value);
            break;
        }
#endif
    }
    
    PRINTLN_PROGMEM(Strings::MSG_OK);
}

void SerialXShell::runFunction(const char* name) {
    if (!name || strlen(name) == 0) {
        PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_CMD);
        return;
    }
    
    SerialFunction* func = findFunction(name);
    if (!func) {
        PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_CMD);
        return;
    }
    
    func->func();
    PRINTLN_PROGMEM(Strings::MSG_OK);
}

void SerialXShell::printHelp() {
    
    // List variables
    for (int i = 0; i < variableCount; i++) {
        SerialVariable* var = variables[i];
        char buf[64];
        snprintf(buf, sizeof(buf), "%c%s%s",
        var->type,
        (var->canWrite ? "x " : "o "),
        var->name);
        communication.sendLine(StringType(buf));
    }
    
    // List virtual variables (read-only)
    for (int i = 0; i < virtualVariableCount; i++) {
        SerialVirtualVariable* vVar = virtualVariables[i];
        char buf[64];
        snprintf(buf, sizeof(buf), "%c%s%s",
            vVar->type,
            "v ",  // Virtual variables are always read-only
            vVar->name);
        communication.sendLine(StringType(buf));
    }
    
    // List functions
    char buf[64];
    for (int i = 0; i < functionCount; i++) {
        snprintf(buf, sizeof(buf), "r %s", functions[i]->name);
        communication.sendLine(StringType(buf));
    }
}

void SerialXShell::authenticate(const char* key) {
    if (!key) {
        PRINTLN_PROGMEM(Strings::MSG_AUTH_FAIL);
        return;
    }
    
    if (strcmp(key, accessKey) == 0) {
        isAccessUnlocked = true;
        PRINTLN_PROGMEM(Strings::MSG_AUTH_SUCCESS);
    } else {
        PRINTLN_PROGMEM(Strings::MSG_AUTH_FAIL);
    }
}

void SerialXShell::printInfo() {
    char buf[128];

    snprintf(buf, sizeof(buf), "Device: %s", deviceName);
    communication.sendLine(StringType(buf));

    snprintf(buf, sizeof(buf), "Software: %s", softwareVersion);
    communication.sendLine(StringType(buf));

    snprintf(buf, sizeof(buf), "SerialX Version: %d.%d",
             SerialXShell::SerialX_MajorVersion,
             SerialXShell::SerialX_MinorVersion);
    communication.sendLine(StringType(buf));

    snprintf(buf, sizeof(buf), "JIT Version: %d.%d",
             SerialXShell::SerialXJIT_MajorVersion,
             SerialXShell::SerialXJIT_MinorVersion);
    communication.sendLine(StringType(buf));

    char types[64];
    size_t pos = 0;
    pos += snprintf(types + pos, sizeof(types) - pos, "Types supported: ");

#if SERIALX_SUPPORT_BOOL
    types[pos++] = 'b';
#endif
#if SERIALX_SUPPORT_INT
    types[pos++] = 'i';
#endif
#if SERIALX_SUPPORT_FLOAT
    types[pos++] = 'f';
#endif
#if SERIALX_SUPPORT_STRING
    types[pos++] = 's';
#endif
#if SERIALX_SUPPORT_CHAR
    types[pos++] = 'c';
#endif
#if SERIALX_SUPPORT_CHARSTRING
    types[pos++] = 'C';
#endif
#if SERIALX_SUPPORT_UINT8
    types[pos++] = 'u';
#endif
#if SERIALX_SUPPORT_UINT16
    types[pos++] = 'w';
#endif
#if SERIALX_SUPPORT_UINT32
    types[pos++] = 'd';
#endif
#if SERIALX_SUPPORT_LONG
    types[pos++] = 'l';
#endif
#if SERIALX_SUPPORT_DOUBLE
    types[pos++] = 'D';
#endif

    communication.sendLine(StringType(types));
}

// ===== ADVANCED FEATURES =====
#if SERIALX_ENABLE_BATCH_CMD
void SerialXShell::executeBatchCommand(const char* batchCmd) {
    if (!batchCmd) return;
    
    char cmdBuf[SERIALX_INPUT_BUFFER_SIZE];
    const char* ptr = batchCmd;
    int cmdCount = 0;
    
    while (*ptr && cmdCount < 5) {
        const char* pipe = strchr(ptr, '|');
        int len = pipe ? (pipe - ptr) : strlen(ptr);
        
        if (len > 0 && len < SERIALX_INPUT_BUFFER_SIZE) {
            strncpy(cmdBuf, ptr, len);
            cmdBuf[len] = '\0';
            
            // Trim leading spaces
            char* start = cmdBuf;
            while (*start && isspace(*start)) start++;
            
            char* end = start + strlen(start) - 1;
            while (end > start && isspace(*end)) {
                *end = '\0';
                end--;
            }
            
            if (strlen(start) > 0) {
                handleCommand(start);
            }
        }
        
        ptr = pipe ? (pipe + 1) : "";
        cmdCount++;
    }
}
#endif

#if SERIALX_ENABLE_JSON_OUTPUT
void SerialXShell::outputVariablesJSON(const char* varNames) {
    if (!varNames) return;
    
    Serial.print("{");
    
    char nameBuf[SERIALX_VAR_NAME_SIZE];
    const char* ptr = varNames;
    bool first = true;
    int varCount = 0;
    
    while (*ptr && varCount < 5) {
        const char* space = strchr(ptr, ' ');
        int len = space ? (space - ptr) : strlen(ptr);
        
        if (len > 0 && len < SERIALX_VAR_NAME_SIZE) {
            strncpy(nameBuf, ptr, len);
            nameBuf[len] = '\0';
            
            SerialVariable* var = findVariable(nameBuf);
            if (var) {
                if (!first) Serial.print(",");
                
                Serial.print("\"");
                Serial.print(var->name);
                Serial.print("\":");
                
                // Output value as JSON
                switch (var->type) {
                    case 'b': {
                        bool* val = (bool*)var->variable;
                        Serial.print(*val ? "true" : "false");
                        break;
                    }
                    case 'i': {
                        int* val = (int*)var->variable;
                        Serial.print(*val);
                        break;
                    }
                    case 'f': {
                        float* val = (float*)var->variable;
                        Serial.print(*val, 2);
                        break;
                    }
                    case 's': {
                        char* val = (char*)var->variable;
                        Serial.print("\"");
                        Serial.print(val);
                        Serial.print("\"");
                        break;
                    }
                }
                
                first = false;
            }
        }
        
        ptr = space ? (space + 1) : "";
        varCount++;
    }
    
    communication.sendLine("}");
}
#endif

#if SERIALX_ENABLE_TIMESTAMP
void SerialXShell::printTimestamp() {
    communication.sendLine(millis());
}

void SerialXShell::printVariableWithTimestamp(const char* varName) {
    if (!varName) return;
    
    SerialVariable* var = findVariable(varName);
    if (!var) {
        PRINTLN_PROGMEM(Strings::MSG_UNKNOWN_VAR);
        return;
    }
    
    Serial.print(millis());
    Serial.print(":");
    
    switch (var->type) {
        case 'b': {
            bool* val = (bool*)var->variable;
            communication.sendLine(*val ? 1 : 0);
            break;
        }
        case 'i': {
            int* val = (int*)var->variable;
            communication.sendLine(*val);
            break;
        }
        case 'f': {
            float* val = (float*)var->variable;
            communication.sendLine(*val, 2);
            break;
        }
        case 's': {
            char* val = (char*)var->variable;
            communication.sendLine(val);
            break;
        }
    }
}
#endif

// ===== MAIN LOOP =====
void SerialXShell::shellLoop() {
    if (!isCommunicationOpen) return;
    if (!communication.isClientConnected()) return;

    StringType line;
    if (communication.readLine(line)) {
        if (line.length() > 0) {
            handleCommand(line.c_str());
        }
    }
    /*
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (inputPos > 0) {
                inputBuffer[inputPos] = '\0';
                handleCommand(inputBuffer);
                inputPos = 0;
            }
        } else if (inputPos < SERIALX_INPUT_BUFFER_SIZE - 1) {
            inputBuffer[inputPos++] = c;
        }
    }
    */
}
