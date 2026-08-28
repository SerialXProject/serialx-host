#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include "SerialXShell.h"

using namespace std;

SerialXShell shell(9600, 11, 3, 1, "secure", "SerialXExample", "1.0.0");

bool led_state = false;
int temperature = 25;
float humidity = 45.5;
string text = "Hello, SerialX!";
char char_var = 'A';
char char_array[20] = "Arduino Shell";
uint8_t byte_var = 255;
uint16_t word_var = 65535;
uint32_t dword_var = 4294967295;
long long_var = 1234567890;
double double_var = 3.1415926535;

void turnLedOn() {
  cout << "LED ON\n";
}

void turnLedOff() {
  cout << "LED OFF\n";
}

// Simula lettura sensori con C++ standard
int sensorRead() {
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 15);
  return 20 + dis(gen);
}

int main() {
  
  // Start communication
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

  // Il server deve restare attivo per tutta la durata della connessione TCP.
  while (shell.IsCommunicationOpen())
  {
    shell.shellLoop();
  }

  cout << "Test terminato correttamente.\n";
  return 0;
}
