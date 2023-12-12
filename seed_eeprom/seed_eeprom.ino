#include <LinkedList.h>
#include <EEPROM.h>

struct Instruction {
  bool is_table;
  short card_amount;
};

struct Game {
  short max_players;
  short min_players;
  char name[20];
  short excluded_cards[52];
  short total_excluded_cards;
  struct Instruction instructions[20];
  short total_instructions;
 };

LinkedList<Game> game_list;
Game currentGame;

void setup() {
  Serial.begin(9600);
}

void extractExcluded(const String& message, short output[], int& total_cards) {
    const int max_size = 52;
    total_cards = 0;
    int startIndex = 0;
    int endIndex = 0;

    // Skip the first word (assumed to be "excludedCards")
    endIndex = message.indexOf(' ', startIndex);
    startIndex = endIndex + 1;

    while (startIndex < message.length() && total_cards < max_size) {
        endIndex = message.indexOf(' ', startIndex);
        if (endIndex == -1) {
            endIndex = message.length();
        }

        String numberStr = message.substring(startIndex, endIndex);
        short number = numberStr.toInt();
        output[total_cards++] = number;

        startIndex = endIndex + 1;
    }
}


String extract_after_space(String texto) {
  int spaceIndex =  texto.indexOf(' ');
  if (spaceIndex != -1) {
    String value = texto.substring(spaceIndex + 1);
    return value;
  }
}

String extract_before_space(String texto) {
  int spaceIndex = texto.indexOf(' ');
  if (spaceIndex != -1) {
    String value = texto.substring(0, spaceIndex);
    return value;
  } else {
    return texto; // Return the whole string if no space is found
  }
}


void loop() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil("\n");
    message.trim();
    if (message.length() > 0) {

      if (message.startsWith("name")) {
      String nameValue = extract_after_space(message);
      nameValue.toCharArray(currentGame.name, 20);
      }
      else if (message.startsWith("minPlayers")) {
        currentGame.min_players = (short)extract_after_space(message).toInt();
      }
      else if (message.startsWith("maxPlayers")) {
        currentGame.max_players = (short)extract_after_space(message).toInt();
      }
      else if (message.startsWith("instructions")) {
        short i = 0;
        while (!message.startsWith("endinstructions")) {
          message = Serial.readStringUntil("\n");
          if (message.startsWith("table")) {
            currentGame.instructions[i].is_table = true;
          } else {
            currentGame.instructions[i].is_table = false;
          }
          currentGame.instructions[i].card_amount = (short)extract_after_space(message).toInt();
        }
      }
      else if (message.startsWith("excludedCards")) {
        short excluded_cards[52];
        int total_excluded_cards;
        extractExcluded(message, excluded_cards, total_excluded_cards);
        for (int i = 0; i < total_excluded_cards; i++) {
          currentGame.excluded_cards[i] = excluded_cards[i];
        }
        currentGame.total_excluded_cards = total_excluded_cards;
      }
      else if (message.startsWith("endgame")) {
        game_list.add(currentGame);
      }
      else if (message.startsWith("cambiodesligo")) {
        short total_seed_items = game_list.size();
        EEPROM.put(0, total_seed_items);
        for (int i = 0; i < total_seed_items; i++) {
          EEPROM.put(sizeof(total_seed_items) + (sizeof(Game) * i), game_list.get(i));
        }
      }
    }
  }
}
