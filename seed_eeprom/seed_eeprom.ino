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
    Serial.println(message);
    if (message.length() > 0) {
        
      if (message.startsWith("name")) {
      String nameValue = extract_value(message);
      Serial.println(nameValue);
      nameValue.toCharArray(currentGame.name, 20);
      } else if (message.startsWith("minPlayers")) {
        Serial.println(extract_value(message));
        currentGame.min_players = (short)extract_value(message).toInt();
      } else if (message.startsWith("maxPlayers")) {
        currentGame.max_players = (short)extract_value(message).toInt();
      } else if (message.startsWith("endgame")) {
        game_list.add(currentGame);
      } else if (message.startsWith("print")) {
        for (int i=0; i<game_list.size(); i++) {
          Game game = game_list.get(i);
          String name_msg = "Name " +  String(game.name);
          Serial.println(name_msg);
          String min_msg = "Min " + String(game.min_players);
          Serial.println(min_msg);
          String max_msg = "Max " + String(game.max_players); 
          Serial.println(max_msg);
        }

      }


      }
    }
  }
