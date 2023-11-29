#include <EEPROM.h>

void writeStringToEEPROM(int addr, const String &inputData) {
  int len = inputData.length();
  EEPROM.write(addr, len); // Escreve o comprimento da string
  for (int i = 0; i < len; i++) {
    Serial.write(inputData[i]);
    EEPROM.write(addr + 1 + i, inputData[i]); // Escreve os caracteres da string
  }
}

void setup() {
}

void loop() {
  if (Serial.available() > 0) {
    String inputData = Serial.readString();
    writeStringToEEPROM(0, inputData); // Escreve na EEPROM começando do endereço 0
  }
}
