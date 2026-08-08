#include <Arduino.h>
#include "SerialXShell.h"

SerialXShell shell(9600, 4, 5, "test");

bool bool_var = false;
int int_num = 0;
float float_num = 0;
String string = "ciao";

void ActiveLed() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void DisactiveLed() {
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  shell.startCommunication();

  shell.addVariable(new SerialVariable("bool_var", bool_var, true));
  shell.addVariable(new SerialVariable("int_num", int_num, true));
  shell.addVariable(new SerialVariable("float_num", float_num, true));
  shell.addVariable(new SerialVariable("string", string, true));
  shell.addFunction(new SerialFunction("led_on", ActiveLed));
  shell.addFunction(new SerialFunction("led_off", DisactiveLed));
}

void loop() {
  shell.shellLoop();
}
