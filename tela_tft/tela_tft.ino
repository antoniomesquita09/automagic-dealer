#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <JKSButton.h>
#include <EEPROM.h>
#include <LinkedList.h>

struct Game
{
  short max_players;
  short min_players;
  char name[20];
};

LinkedList<Game> game_list;

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);

JKSButton next_button, detail_button, back_button, play_button, add_player_button, dec_player_button;

int screen_index = 0;
short total_items;
bool detail_page = false;
bool config_page = false;
short players_count = 0;

void clean()
{
  tela.fillScreen(TFT_BLACK);
}

void setup_game_list_screen()
{
  clean();

  detail_page = false;
  config_page = false;

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print("DEALER");

  Game cur_game = game_list.get(screen_index);

  // game click button
  detail_button.init(&tela, &touch, 120, 130, 200, 100, TFT_WHITE, TFT_GREEN, TFT_BLACK, cur_game.name, 2);
  detail_button.setPressHandler(setup_game_detail_screen);

  // go foward button
  next_button.init(&tela, &touch, 120, 250, 200, 100, TFT_WHITE, TFT_BLUE, TFT_WHITE, "Proximo", 2);
  next_button.setPressHandler(next_game_screen);
}

void setup_game_detail_screen(JKSButton &botaoPressionado)
{
  clean();

  detail_page = true;
  config_page = false;

  Game cur_game = game_list.get(screen_index);
  players_count = cur_game.min_players;

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(cur_game.name);

  String max_message = "Maximo: " + String(cur_game.max_players);
  String min_message = "Minimo: " + String(cur_game.min_players);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(min_message);

  tela.setCursor(10, 110);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(max_message);

  // go foward button
  play_button.init(&tela, &touch, 170, 250, 100, 80, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Jogar", 2);
  play_button.setPressHandler(setup_config_game_screen);

  // go foward button
  back_button.init(&tela, &touch, 60, 250, 100, 80, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  back_button.setPressHandler(setup_game_list_screen);
}

void setup_config_game_screen(JKSButton &botaoPressionado)
{
  clean();

  detail_page = false;
  config_page = true;

  Game cur_game = game_list.get(screen_index);

  String players_count_message = "Jodagores: " + String(players_count);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(cur_game.name);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);

  // add players count button
  add_player_button.init(&tela, &touch, 60, 160, 100, 80, TFT_WHITE, TFT_BLUE, TFT_WHITE, "Adicionar", 2);
  add_player_button.setPressHandler(add_players_count);

  // dec players count button
  dec_player_button.init(&tela, &touch, 170, 160, 100, 80, TFT_WHITE, TFT_ORANGE, TFT_BLACK, "Diminuir", 2);
  dec_player_button.setPressHandler(dec_players_count);

  // go foward button
  play_button.init(&tela, &touch, 170, 260, 100, 80, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Jogar", 2);
  play_button.setPressHandler(start_game);

  // go foward button
  back_button.init(&tela, &touch, 60, 260, 100, 80, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  back_button.setPressHandler(setup_game_detail_screen);
}

void start_game()
{
  Game cur_game = game_list.get(screen_index);
  String start_game_message = "START " + String(cur_game.name) + " " + String(players_count);
  Serial.println(start_game_message);
}

void add_players_count()
{
  Game cur_game = game_list.get(screen_index);

  if (players_count == cur_game.max_players)
  {
    return;
  }

  players_count++;

  tela.fillRect(130, 50, 100, 40, TFT_BLACK);

  String players_count_message = "Jodagores:" + String(players_count);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);
}

void dec_players_count()
{
  Game cur_game = game_list.get(screen_index);

  if (players_count == cur_game.min_players)
  {
    return;
  }

  players_count--;

  tela.fillRect(130, 50, 100, 40, TFT_BLACK);

  String players_count_message = "Jodagores:" + String(players_count);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);
}

void next_game_screen()
{
  if (screen_index == total_items - 1)
  {
    screen_index = 0;
  }
  else
  {
    screen_index++;
  }
  setup_game_list_screen();
}

void seed_eeprom()
{
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
  EEPROM.put(0, total_seed_items);

  for (int i = 0; i < total_seed_items; i++)
  {
    EEPROM.put(sizeof(total_seed_items) + (sizeof(Game) * i), game_list_seed.get(i));
  }
}

void setup()
{
  Serial.begin(9600);

  // Uncomment bellow to seed eeprom (udate seed_eeprom func with your custom games)
  // seed_eeprom();

  // Read from EEPROM
  EEPROM.get(0, total_items);
  Serial.println("eeprom total items response:");
  Serial.println(total_items);

  for (int j = 0; j < total_items; j++)
  {
    Game tmp_game;
    EEPROM.get(sizeof(total_items) + (sizeof(Game) * j), tmp_game);
    game_list.add(tmp_game);
  }

  for (int k = 0; k < game_list.size(); k++)
  {
    game_list.get(k);
  }

  tela.begin(tela.readID());
  setup_game_list_screen();
}

void loop()
{
  if (detail_page)
  { // Details page
    back_button.process();
    play_button.process();
  }
  else if (config_page)
  { // Config page
    back_button.process();
    play_button.process();
    add_player_button.process();
    dec_player_button.process();
  }
  else
  { // Game page
    next_button.process();
    detail_button.process();
  }
}

void clean_pos(int x1, int y1, int x2, int y2)
{
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
}