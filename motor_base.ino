#include <Stepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int limit = 6;
int qtd_cartas = 3;

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(60);
  // initialize the serial port:
  Serial.begin(9600);
}

void distribui(int players, int totalCards) {
  if (players > limit) {
    Serial.println("Qtd de jogadores maior que " + String(limit));
  }

  // TODO: go to zero position

  int totalRevolutionSteps = stepsPerRevolution * players;
  int initialTotalPlayers = players;

  for (int j = 0; j < qtd_cartas; j ++) {
    for (int i = 0; i < players; i++) {
      myStepper.step(stepsPerRevolution);
      delay(500);
    }
    // go back to zero position
    myStepper.step(-totalRevolutionSteps + stepsPerRevolution);
    players = initialTotalPlayers - 1;
    delay(500);
  }
}

void loop() {
  if (Serial.available() > 0) {
    String text = Serial.readStringUntil('\n');
    text.trim();
    Serial.println("Jogadores: " + text);
    int players = text.toInt();

    distribui(players, qtd_cartas);
  }
  delay(500);
}