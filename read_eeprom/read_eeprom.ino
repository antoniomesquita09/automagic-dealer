#include <EEPROM.h>
#include <LinkedList.h>


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
short total_items;

void setup() {
 Serial.begin(9600);

  // Uncomment bellow to seed eeprom (udate seed_eeprom func with your custom games)
  // seed_eeprom();

  // Read from EEPROM
  EEPROM.get(0, total_items);
  Serial.println("eeprom total items response:");
  Serial.println(total_items);
}

void loop() {
}