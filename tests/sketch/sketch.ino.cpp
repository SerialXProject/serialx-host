// Questo file è pensato per essere modificato liberamente: è il contenuto
// di un normale sketch Arduino (.ino), solo con estensione .cpp per essere
// compilato direttamente da g++ sull'ambiente desktop.
//
// Se un giorno vuoi caricarlo su un Arduino reale, basta rinominarlo in
// .ino (e togliere questo commento se vuoi) e aprirlo con l'IDE Arduino:
// non contiene nulla di specifico per l'ambiente desktop.
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
