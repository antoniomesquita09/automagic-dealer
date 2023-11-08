#include <Stepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int limit = 6;

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(60);
  // initialize the serial port:
  Serial.begin(9600);
}

void distribui(int players) {
  for (int i = 0; i < players; i++) {
    myStepper.step(stepsPerRevolution);
    delay(500);
  }
}

void loop() {
  if (Serial.available() > 0) {
    String text = Serial.readStringUntil('\n');
    text.trim();
    Serial.println("Jogadores: " + text);
    int players = text.toInt();

    if (players > limit) {
      Serial.println("Qtd de jogadores maior que " + String(limit));
    } else {
      distribui(players);
    }

    // if (command == clockwise) {
    //   myStepper.step(stepsPerRevolution);
    // } else if (command == counterclockwise) {
    //   myStepper.step(-stepsPerRevolution);
    // }
  }

  // delay(500);
  delay(500);
}

