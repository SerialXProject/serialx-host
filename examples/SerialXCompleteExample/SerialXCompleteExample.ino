#include <Arduino.h>
#include "SerialXShell.h"

SerialXShell shell(9600, 11, 3, 1, "secure", "SerialXExample", "1.0.0");

bool led_state = false;
int temperature = 25;
float humidity = 45.5;
String text = "Hello, SerialX!";
char char_var = 'A';
char char_array[20] = "Arduino Shell";
uint8_t byte_var = 255;
uint16_t word_var = 65535;
uint32_t dword_var = 4294967295;
long long_var = 1234567890;
double double_var = 3.1415926535;

void turnLedOn() {
  led_state = true;
  digitalWrite(LED_BUILTIN, HIGH);
}

void turnLedOff() {
  led_state = false;
  digitalWrite(LED_BUILTIN, LOW);
}

// Simula lettura sensori
int sensorRead() {
  return 20 + random(0, 15);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Start serial communication
  shell.startCommunication();
  
  // Register variables
  shell.addVariable(new SerialVariable("led_state", led_state, true));
  shell.addVariable(new SerialVariable("temperature", temperature, true));
  shell.addVariable(new SerialVariable("humidity", humidity, true));
  shell.addVariable(new SerialVariable("text", text, true));
  shell.addVariable(new SerialVariable("char_var", char_var, true));
  shell.addVariable(new SerialVariable("char_array", char_array, true));
  shell.addVariable(new SerialVariable("byte_var", byte_var, true));
  shell.addVariable(new SerialVariable("word_var", word_var, true));
  shell.addVariable(new SerialVariable("dword_var", dword_var, true));
  shell.addVariable(new SerialVariable("long_var", long_var, true));
  shell.addVariable(new SerialVariable("double_var", double_var, true));
  
  // Register functions
  shell.addFunction(new SerialFunction("led_on", turnLedOn));
  shell.addFunction(new SerialFunction("led_off", turnLedOff));

  // Register Virtual Variable
  shell.addVirtualVariable(new SerialVirtualVariable("sensor_read", sensorRead));
}

void loop() {
    shell.shellLoop();
    
    // Other operations can run here
    delay(100);
}

/*
SERIAL COMMANDS AVAILABLE:

Authentication:
  akey secure              - Unlock with access key
  isAuthActive            - Check if auth is enabled

Help:
  h                        - Show all variables and functions

Get variables:
  gb led_state            - Get bool
  gi temperature          - Get int
  gf humidity             - Get float
  gc char_var             - Get char
  gs text                 - Get string
  gu byte_var             - Get uint8_t
  gw word_var             - Get uint16_t
  gd dword_var            - Get uint32_t
  gl long_var             - Get long
  gD double_var           - Get double

Set variables:
  sb led_state 1          - Set bool (1 = true, 0 = false)
  si temperature 30       - Set int
  sf humidity 50.5        - Set float
  ss device_name "Mega"   - Set string

Run functions:
  r led_on                - Execute function
  r led_off
  r sensor_read
*/
