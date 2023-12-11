#include <LinkedList.h>

struct Game {
  char name[20];
  short min_players;
  short max_players;
};

LinkedList<Game> game_list;
Game currentGame;

void setup() {
  Serial.begin(9600);
}

String extract_value(String texto) {
  int spaceIndex =  texto.indexOf(' ');
  if (spaceIndex != -1) {
    String value = texto.substring(spaceIndex + 1);
    return value;
  }
}

void loop() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil("\n");
    message.trim();
    if (message.length() > 0) {
        
      if (message.startsWith("name")) {
      String nameValue = extract_value(message);
      nameValue.toCharArray(currentGame.name, 20);
      } else if (message.startsWith("minPlayers")) {
        Serial.println(extract_value(message));
        currentGame.min_players = (short)extract_value(message).toInt();
      } else if (message.startsWith("maxPlayers")) {
        currentGame.max_players = (short)extract_value(message).toInt();
      } else if (message.startsWith("endgame")) {
        game_list.add(currentGame);
      }
    }
  }
}
