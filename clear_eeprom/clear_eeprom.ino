#include <EEPROM.h>

void setup() {
  // Inicia a comunicação serial para acompanhamento
  Serial.begin(9600);

  Serial.println("Limpando EEPROM...");

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0); // Escreve 0 em cada posição da EEPROM
  }

  Serial.println("EEPROM limpa.");
}

void loop() {
  // Não é necessário código no loop para esta tarefa
}
