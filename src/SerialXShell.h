#pragma once
#include <Arduino.h>
#include "config.h"
#include "models/serialVariable.h"
#include "models/serialFunction.h"
#include "models/serialVirtualVariable.h"
#include "strings/Messages.h"

class SerialXShell {
public:
    SerialXShell(int baud = 9600, 
                 int maxVars = 20, 
                 int maxFuncs = 20,
                 int maxVirtualVars = 20,
                 const char* accessPassKey = nullptr,
                 const char* deviceName = "",
                 const char* softwareVersion = "");
    
    ~SerialXShell();
    
    static const int SerialX_MajorVersion;
    static const int SerialX_MinorVersion;
    static const int SerialXJIT_MajorVersion;
    static const int SerialXJIT_MinorVersion;
    
    bool IsCommunicationOpen() const { return isCommunicationOpen; }
    void startCommunication();
    void closeCommunication();
    void shellLoop();
    void handleCommand(const char* cmd);
    
    void addVariable(SerialVariable* var);
    void addFunction(SerialFunction* f);
    void addVirtualVariable(SerialVirtualVariable* vVar);
    
    // Expose for advanced features
    SerialVariable* findVariable(const char* name);
    SerialFunction* findFunction(const char* name);
    SerialVirtualVariable* findVirtualVariable(const char* name);
    int getVariableCount() const { return variableCount; }
    SerialVariable* getVariable(int idx) { return idx < variableCount ? variables[idx] : nullptr; }
    
private:
    int baudRate;
    bool isCommunicationOpen;

    char deviceName[32];
    char softwareVersion[32];
    
    // Access key: fixed buffer instead of String
    char accessKey[SERIALX_ACCESS_KEY_SIZE];
    bool isAccessUnlocked;
    
    // UNIFIED array for all variables (was 4 separate arrays)
    SerialVariable** variables;
    int variableCount;
    int maxVariables;
    
    // Virtual variables (getter functions)
    SerialVirtualVariable** virtualVariables;
    int virtualVariableCount;
    int maxVirtualVariables;
    
    // Functions
    SerialFunction** functions;
    int functionCount;
    int maxFunctions;
    
    // Input buffer: static, not dynamic String
    char inputBuffer[SERIALX_INPUT_BUFFER_SIZE];
    int inputPos;
    
    // Command handlers
    void getVariable(const char* cmd);
    void setVariable(const char* cmd);
    void runFunction(const char* cmd);
    void printHelp();
    void printInfo();
    void authenticate(const char* key);
    
#if SERIALX_ENABLE_BATCH_CMD
    void executeBatchCommand(const char* batchCmd);
#endif

#if SERIALX_ENABLE_JSON_OUTPUT
    void outputVariablesJSON(const char* varNames);
#endif

#if SERIALX_ENABLE_TIMESTAMP
    void printTimestamp();
    void printVariableWithTimestamp(const char* varName);
#endif
};
