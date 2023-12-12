#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <JKSButton.h>
#include <EEPROM.h>
#include <LinkedList.h>

struct Instruction 
{
  bool is_table;
  short card_amount;
};

struct Game
{
  short max_players;
  short min_players;
  char name[20];
  short excluded_cards[52];
  struct Instruction instructions[20];
  short total_instructions;
  short total_excluded_cards;
};

LinkedList<Game> game_list;

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);

JKSButton next_button, detail_button, back_button, play_button, add_player_button, dec_player_button;

int screen_index = 0;
short total_items;
bool game_list_page = true;
bool game_detail_page = false;
bool game_config_page = false;
bool game_playing_page = false;
bool game_over_page = false;
short players_count = 0;
int instruction_index = 0;
int player_to_distribute_index = 0;
int player_to_distribute_cards_count = 0;
String CARD_ALLOWED_RESPONSE = "CARD_ALLOWED_RESPONSE";
String CARD_ALLOWED_RESPONSE_VALUE_TRUE = "TRUE";

// enum page { GAME_LIST, GAME_DETAIL, GAME_CONFIG, GAME_PLAYING };
// page current_page = GAME_LIST;

// HELPER FUNCTIONS

void clean()
{
  tela.fillScreen(TFT_BLACK);
}

String extract_after_space(String texto)
{
  int spaceIndex =  texto.indexOf(' ');
  if (spaceIndex != -1)
  {
    String value = texto.substring(spaceIndex + 1);
    return value;
  }
}

void check_card_allowed()
{
  String card_allowed_request_message = "CARD_ALLOWED_REQUEST";
  Serial.println(card_allowed_request_message);
}

void next_instruction() {
  Game cur_game = game_list.get(screen_index);

  Instruction cur_instruction = cur_game.instructions[instruction_index];

  player_to_distribute_index = 0;
  player_to_distribute_cards_count= 0;

  if (instruction_index < cur_game.total_instructions)
  {
    instruction_index++;
    setup_is_playing_screen();
  }
  else
  {
    setup_game_over_screen();
  }
}

// If card allowed response is TRUE
void distribute_card(bool should_distribute)
{
  Game cur_game = game_list.get(screen_index);

  // In case card is not allowed, should discard
  if (!should_distribute)
  {
    Serial.println("DISCARD");
    return;
  }

  Instruction cur_instruction = cur_game.instructions[instruction_index];

  if (cur_instruction.is_table) // Distribute to table
  {
    Serial.println("DISTRIBUTE TABLE");
    if (player_to_distribute_cards_count < cur_instruction.card_amount - 1) // The table should receive one more card
    {
      player_to_distribute_cards_count++;
      check_card_allowed();
    }
    else // The table already received all cards, move to the next instruction 
    {
      next_instruction();
    }
  }
  else // Distribute to player
  {
    String distribute_player_message = "DISTRIBUTE " + String(player_to_distribute_index);

    Serial.println(distribute_player_message);

    player_to_distribute_cards_count++;

    if (player_to_distribute_cards_count < cur_instruction.card_amount) // Current player should receive one more card
    {
      check_card_allowed();
    }
    else if (player_to_distribute_index < players_count - 1) // Current player received all cards from instruction, move to next player
    {
      player_to_distribute_index++;
      player_to_distribute_cards_count = 0;
      check_card_allowed();
    }
    else // All players received cards, move to next instruction
    {
      next_instruction();
    }
  }
}

void send_excluded_cards()
{
  Game cur_game = game_list.get(screen_index);
  String excluded_cards_message = "EXCLUDED_CARDS";

  if (cur_game.total_excluded_cards == 0)
  {
    return;
  }

  for (int i = 0; i < cur_game.total_excluded_cards; i++) {
    excluded_cards_message = excluded_cards_message + " " + String(cur_game.excluded_cards[i]);
  }

  Serial.println(excluded_cards_message);
}

void start_game()
{
  Game cur_game = game_list.get(screen_index);
  String start_game_message = "START " + String(cur_game.name) + " " + String(players_count);
  Serial.println(start_game_message);
  setup_is_playing_screen();
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

  String players_count_message = "Jogadores:" + String(players_count);

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

  String players_count_message = "Jogadores:" + String(players_count);

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
  Instruction i1, i2, i3, i4;

  i1.card_amount = 2;
  i1.is_table = false;
  
  i2.card_amount = 3;
  i2.is_table = true;
  
  i3.card_amount = 1;
  i3.is_table = true;
    
  i4.card_amount = 1;
  i4.is_table = true;

  game1.min_players = 2;
  game1.max_players = 4;
  strcpy(game1.name, "Poker");

  game1.instructions[0] = i1;
  game1.instructions[1] = i2;
  game1.instructions[2] = i3;
  game1.instructions[3] = i4;

  game1.total_instructions = 4;
  game1.total_excluded_cards = 0;

  game2.min_players = 2;
  game2.max_players = 6;
  strcpy(game2.name, "buraco");

  game3.min_players = 4;
  game3.max_players = 4;
  strcpy(game3.name, "truco");

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

// SETUP SCREENS FUNCTIONS

void setup_game_list_screen()
{
  clean();

  game_list_page = true;
  game_detail_page = false;
  game_config_page = false;
  game_playing_page = false;
  game_over_page = false;

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

void setup_game_detail_screen()
{
  clean();

  game_list_page = false;
  game_detail_page = true;
  game_config_page = false;
  game_playing_page = false;
  game_over_page = false;

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

void setup_config_game_screen()
{
  clean();

  game_list_page = false;
  game_detail_page = false;
  game_config_page = true;
  game_playing_page = false;
  game_over_page = false;

  Game cur_game = game_list.get(screen_index);

  String players_count_message = "Jogadores: " + String(players_count);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(cur_game.name);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);

  // add players count button
  dec_player_button.init(&tela, &touch, 60, 160, 100, 80, TFT_WHITE, TFT_ORANGE, TFT_BLACK, "-", 2);
  dec_player_button.setPressHandler(dec_players_count);

  // dec players count button
  add_player_button.init(&tela, &touch, 170, 160, 100, 80, TFT_WHITE, TFT_BLUE, TFT_WHITE, "+", 2);
  add_player_button.setPressHandler(add_players_count);

  // go foward button
  play_button.init(&tela, &touch, 170, 260, 100, 80, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Jogar", 2);
  play_button.setPressHandler(start_game);

  // go foward button
  back_button.init(&tela, &touch, 60, 260, 100, 80, TFT_WHITE, TFT_RED, TFT_WHITE, "Voltar", 2);
  back_button.setPressHandler(setup_game_detail_screen);
}

void setup_is_playing_screen()
{
  clean();

  game_list_page = false;
  game_detail_page = false;
  game_config_page = false;
  game_playing_page = true;
  game_over_page = false;

  Game cur_game = game_list.get(screen_index);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(cur_game.name);

  String turn_message = "Rodada: " + String(instruction_index);

  tela.setCursor(10, 50);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(turn_message);

  send_excluded_cards();

  // distribute next cards button
  play_button.init(&tela, &touch, 120, 250, 200, 100, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Distribuir", 2);
  play_button.setPressHandler(check_card_allowed);
}

void setup_game_over_screen()
{
  clean();

  game_list_page = false;
  game_detail_page = false;
  game_config_page = false;
  game_playing_page = false;
  game_over_page = true;

  Game cur_game = game_list.get(screen_index);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print("Game over");

  instruction_index = 0;
  player_to_distribute_index = 0;
  player_to_distribute_cards_count = 0;

  // distribute next cards button
  play_button.init(&tela, &touch, 120, 250, 200, 100, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Replay", 2);
  play_button.setPressHandler(setup_is_playing_screen);
}

void setup()
{
  Serial.begin(9600);

  // Uncomment bellow to seed eeprom (udate seed_eeprom func with your custom games)
  seed_eeprom();

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
  if (game_detail_page)
  { // Game details page
    back_button.process();
    play_button.process();
  }
  else if (game_config_page)
  { // Game config page
    back_button.process();
    play_button.process();
    add_player_button.process();
    dec_player_button.process();
  }
  else if (game_list_page)
  { // Game list page
    next_button.process();
    detail_button.process();
  }
  else if (game_playing_page)
  { // Game playing page
    play_button.process();
  }
  else if (game_over_page)
  { // Game over page
    play_button.process();
  }
  else
  {
    Serial.println("[ERROR] Unknown page status");
  }

  // Read card allowed response
  if (Serial.available() > 0) {
    String texto = Serial.readStringUntil('\n');
    texto.trim();
    if (texto.length() > 0) {
      if (texto.startsWith(CARD_ALLOWED_RESPONSE)) {
        String card_allowed_value = extract_after_space(texto);
        card_allowed_value.toUpperCase();
        bool should_distribute = card_allowed_value.startsWith(CARD_ALLOWED_RESPONSE_VALUE_TRUE);
        distribute_card(should_distribute);
      }
    }
  }
}

void clean_pos(int x1, int y1, int x2, int y2)
{
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
}