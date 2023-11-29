#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <JKSButton.h>
#include <EEPROM.h>
#include <LinkedList.h>

struct Game {
 short max_players;
 short min_players;
 char name[20];
};

LinkedList<Game> game_list;

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);

JKSButton next_button, detail_button, back_button;

int screen_index = 0;
short total_items;
bool detail_page = false;

void clean() {
  tela.fillScreen(TFT_BLACK);
}

void setup_game_screen() {
  clean();

  detail_page = false;

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print("DEALER");

  Game cur_game = game_list.get(screen_index);

  // game click button
  detail_button.init(&tela, &touch, 120, 130, 200, 100, TFT_WHITE, TFT_GREEN, TFT_BLACK, cur_game.name, 2);
  detail_button.setPressHandler(setup_detail_screen);
  
  // go foward button
  next_button.init(&tela, &touch, 120, 250, 200, 100, TFT_WHITE, TFT_BLUE, TFT_WHITE, "Proximo", 2);
  next_button.setPressHandler(next_game_screen);
}

void setup_detail_screen(JKSButton &botaoPressionado) {
  clean();

  detail_page = true;

  Game cur_game = game_list.get(screen_index);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(cur_game.name);

  // go foward button
  back_button.init(&tela, &touch, 120, 250, 200, 100, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  back_button.setPressHandler(setup_game_screen);
}

void next_game_screen() {
  if (screen_index == total_items - 1) {
    screen_index = 0;
  } else {
    screen_index++;
  }
  setup_game_screen();
}

void seed_eeprom() {
  Game game1, game2, game3;

  game1.min_players = 4;
  game1.max_players = 4;
  strcpy(game1.name, "truco");

  game2.min_players = 2;
  game2.max_players = 6;
  strcpy(game2.name, "buraco");

  game3.min_players = 2;
  game3.max_players = 8;
  strcpy(game3.name, "poker");

  LinkedList<Game> game_list_seed;

  game_list_seed.add(game1);
  game_list_seed.add(game2);
  game_list_seed.add(game3);

  short total_seed_items = game_list_seed.size();

  // Write to EEPROM
  EEPROM.put(0, total_items);

  for(int i = 0; i < total_items; i++) {
    EEPROM.put(sizeof(total_seed_items) + (sizeof(Game) * i), game_list_seed.get(i));
  }
}

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

  tela.begin(tela.readID());
  setup_game_screen();
}


void loop() {
  if (detail_page) { // Details page
    back_button.process();
  } else { // Game page
    next_button.process();
    detail_button.process();
  }
}

void clean_pos(int x1, int y1, int x2, int y2) {
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
}