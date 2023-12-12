#include <LinkedList.h>

struct Instruction {
  bool is_table;
  short card_amount;
}

struct Game {
  short max_players;
  short min_players;
  char name[20];
  short excluded_cards[52];
  short total_excluded_cards;
  struct Instruction instructions[20];
  short total_instructions;
 }

LinkedList<Game> game_list;
Game currentGame;

void setup() {
  Serial.begin(9600);
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delim)) {
        tokens.push_back(token);
    }

    return tokens;
}

void convertToShortArray(const std::string& message, short output[], int max_size) {
    std::vector<std::string> tokens = split(message, ' ');
    int current_index = 0;

    for (size_t i = 1; i < tokens.size() && current_index < max_size; ++i) {
        output[current_index++] = static_cast<short>(std::stoi(tokens[i]));
    }
}

void extractExcluded(const std::string& message, short output[], int& total_cards) {
    const int max_size = 52;
    std::istringstream iss(message);
    std::string token;
    total_cards = 0;

    // Skip the first token (assumed to be "excludedCards")
    iss >> token;

    while (iss >> token && total_cards < max_size) {
        try {
            short num = static_cast<short>(std::stoi(token));
            output[total_cards++] = num;
        } catch (const std::invalid_argument& e) {
            // Handle the case where the token is not a number
            continue;
        } catch (const std::out_of_range& e) {
            // Handle the case where the number is too large for a short
            continue;
        }
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
        currentGame.excluded_cards = excluded_cards;
        currentGame.total_excluded_cards = total_excluded_cards;
      }
       else if (message.startsWith("endgame")) {
        game_list.add(currentGame);
      }
    }
  }
}
