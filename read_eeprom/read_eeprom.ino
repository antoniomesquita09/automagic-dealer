#include <EEPROM.h>
#include <LinkedList.h>


struct Game {
 short max_players;
 short min_players;
 char name[20];
};
LinkedList<Game> game_list;
short total_items;

void setup() {
 Serial.begin(9600);

  // Uncomment bellow to seed eeprom (udate seed_eeprom func with your custom games)
  // seed_eeprom();

  // Read from EEPROM
  EEPROM.get(0, total_items);
  Serial.println("eeprom total items response:");
  Serial.println(total_items);

  for(int j = 0; j < total_items; j++) {
    Game tmp_game;
    EEPROM.get(sizeof(total_items) + (sizeof(Game) * j), tmp_game);
    game_list.add(tmp_game);
  }

  for(int k = 0; k < game_list.size(); k++) {
    game_list.get(k);
  }

Game game;
  for(int k = 0; k < game_list.size(); k++) {
    game = game_list.get(k);
    Serial.println(game.name);
    Serial.println(game.min_players);
    Serial.println(game.max_players);
  }
}

void loop() {
}