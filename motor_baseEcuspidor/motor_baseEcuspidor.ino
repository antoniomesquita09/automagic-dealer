#include <Stepper.h>
#include <AFMotor.h>

const int stepsPerRevolution = 360; // change this to fit the number of steps per revolution
const int sensor = A15;
bool tem_carta = false;
bool carta_cuspida = false;
bool reverso = false;
int val;
int tempo_passado = 0;
int tempo_atual = 0;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 32, 34, 36, 38);

String texto;
AF_DCMotor motor(1);
AF_DCMotor motor2(2);

int limit = 6;
int totalPlayers = 4;
int discardAngle = 270;
int tabelAngle = 270;
int totalMove = 0;
int rotacao = 2000;

void setup()
{
  // motor base
  myStepper.setSpeed(100);
  Serial.begin(9600);

  //motor carta
  Serial1.begin(9600);
  motor.setSpeed(255);
  motor2.setSpeed(255);
  pinMode(sensor, INPUT);
}

String extract_after_space(String texto)
{
  int spaceIndex = texto.indexOf(' ');
  if (spaceIndex != -1)
  {
    String value = texto.substring(spaceIndex + 1);
    return value;
  }
}

void rotate(int angle)
{
  Serial.println("rotate: " + String(angle));
  myStepper.step(angle);

  totalMove += angle;

  Serial.println("total move: " + String(totalMove));
  delay(500);
}

void resetPosition()
{
  Serial.println(totalMove);
  rotate(-totalMove);
  delay(500);
}

void goToPlayer(int player)
{
  int totalAngle = 180;
  
  if (totalPlayers > 4)
  {
    totalAngle = 270;
    discardAngle = 315;
  }

  int playerAngle = (totalAngle/totalPlayers)*player;
  Serial.println("jogador: " + String(player));
  Serial.println("posicao: " + String(playerAngle));

  int stepAngle = (rotacao/360)*playerAngle;
  Serial.println("step angle: " + String(stepAngle));
  
  rotate(stepAngle-totalMove);
  cuspir();
}

void discard()
{

  int stepAngle = (rotacao/360)*discardAngle;
  rotate(stepAngle-totalMove);
  cuspir();
}

void distribui(int players)
{
  if (players > limit)
  {
    Serial.println("Qtd de jogadores maior que " + String(limit));
  }
  int totalRevolutionSteps = stepsPerRevolution * players;
  int initialTotalPlayers = players;
  // TODO: go to zero position
  myStepper.step(-totalRevolutionSteps + stepsPerRevolution);
  
    for (int i = 0; i < players; i++)
    {
      myStepper.step(stepsPerRevolution);
      delay(500);
    }
    // go back to zero position
    
    delay(500);
}

void cuspir() {
  //motor.run(BACKWARD);
  //motor2.run(FORWARD);
}

void loop()
{
  int tempo_atual = millis();
  // loop motor base
  if (Serial.available() > 0)
  {
    String text = Serial.readStringUntil('\n');
    text.trim();
    Serial.println(text);

    if (text.startsWith("START")) {
      text = extract_after_space(text);
      Serial.println("total de jogadores: " + text);
      totalPlayers = text.toInt();
    }
    else if (text.startsWith("DISTRIBUTE")) {
      carta_cuspida = false;
      tem_carta = false;
      text = extract_after_space(text);
      if (text == "TABLE") {
        goToPlayer(0);
        
      } else {
        int player = text.toInt();
        goToPlayer(player+1);
      }
    } else if (text.startsWith("DISCARD")) {
      Serial.println("DISCARD");
      discard();
    }
  }

  // loop motor carta
  val = analogRead(sensor);
  if (val < 70 && !tem_carta) {
    tem_carta = true;
    Serial.println("TEM CARTA");
  }

  if (tem_carta && val > 70) {
    Serial.println("CUSPIU CARTA");
    motor2.run(RELEASE);
    motor.run(RELEASE);
    tem_carta = false;
    
    reverso = true;
    motor.run(FORWARD);
    motor2.run(BACKWARD);
    tempo_passado = millis();
  }

  tempo_atual = millis();

  if (reverso && tempo_atual > tempo_passado + 200) {
    Serial.println("REVESO");
    motor2.run(RELEASE);
    motor.run(RELEASE);
    reverso = false;
  }
}
