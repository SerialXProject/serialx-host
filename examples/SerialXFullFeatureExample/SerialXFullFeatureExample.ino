#include <Arduino.h>
#include "SerialXShell.h"

// ===== GLOBAL VARIABLES =====
bool alarm_enabled = false;
int sensor_value = 0;
float temperature = 20.5;
uint8_t light_level = 128;
char location[32] = "Lab";

// ===== CALLBACK FUNCTION =====
void onAlarmToggle() {
    if (alarm_enabled) {
        Serial.println(">>> ALARM ENABLED");
    } else {
        Serial.println(">>> ALARM DISABLED");
    }
}

// ===== SENSOR FUNCTIONS =====
void readSensors() {
    sensor_value = analogRead(A0);
    temperature = 20.0 + (sensor_value / 204.8);  // Scale to 20-25°C
    light_level = (sensor_value >> 2) & 0xFF;    // Scale to 0-255
}

void resetSystem() {
    alarm_enabled = false;
    sensor_value = 0;
    temperature = 20.5;
    light_level = 128;
}

// ===== SHELL INSTANCE =====
// Features abilitate in config.h:
// - BATCH_CMD
// - JSON_OUTPUT
// - TIMESTAMP
SerialXShell shell(9600, 12, 6, 3, "secure123");

// ===== SETUP =====
void setup() {
    delay(100);
    
    // Start shell communication
    shell.startCommunication();
    
    // Register variables
    shell.addVariable(new SerialVariable("alarm", alarm_enabled, true));
    shell.addVariable(new SerialVariable("sensor", sensor_value, false));
    shell.addVariable(new SerialVariable("temp", temperature, false));
    shell.addVariable(new SerialVariable("light", light_level, false));
    shell.addVariable(new SerialVariable("location", location, true));
    
    // Register functions
    shell.addFunction(new SerialFunction("read", readSensors));
    shell.addFunction(new SerialFunction("reset", resetSystem));
}

// ===== LOOP =====
void loop() {
    shell.shellLoop();
    delay(50);
}

/*
COMANDI DISPONIBILI:

1. BASIC COMMANDS:
   h                        - Help completo
   akey secure123           - Autenticazione
   
2. VARIABILI:
   gb alarm                 - Get bool
   gi sensor                - Get int
   gf temp                  - Get float
   gu light                 - Get uint8
   gs location              - Get string
   
   sb alarm 1               - Set bool
   ss location "Kitchen"    - Set string

3. FUNZIONI:
   r read                   - Leggi sensori
   r calibrate              - Calibra
   r reset                  - Reset

4. BATCH COMMANDS:
   B gi sensor | gf temp | gu light
   - Esegui 3 GET in una volta
   
   B r read | J sensor temp light
   - Leggi sensori + output JSON

5. JSON OUTPUT:
   J sensor temp light
   - Risposta: {"sensor":512,"temp":22.5,"light":200}
   
   J alarm location
   - Risposta: {"alarm":true,"location":"Lab"}

6. TIMESTAMP:
   T                        - Ottieni timestamp in ms
   - Risposta: 145234
   
   TG temp                  - Leggi temp con timestamp
   - Risposta: 145234:22.5

ESEMPI COMPLESSI:

1. Leggi tutto con timestamp:
   TG sensor | TG temp | TG light
   
2. Combina batch + JSON:
   B r read | J sensor temp light alarm
   
4. Flusso di controllo:
   B sb alarm 1 | r read | J alarm temp sensor

*/
