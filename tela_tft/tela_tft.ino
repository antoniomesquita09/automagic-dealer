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

LinkedList<Game> game_list_to_save;
Game current_game_to_save;
short current_game_to_save_instruction_count = 0;

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);

JKSButton next_button, detail_button, back_button, play_button, add_player_button, dec_player_button;

int game_index = 0;
short total_items;
bool game_list_page = true;
bool game_detail_page = false;
bool game_config_page = false;
bool game_playing_page = false;
bool game_over_page = false;
short total_players = 0;
int instruction_index = 0;
int current_player_index = 0;
int current_player_cards_received = 0;
String CARD_ALLOWED_RESPONSE = "CARD_ALLOWED_RESPONSE";
String CARD_ALLOWED_RESPONSE_VALUE_TRUE = "TRUE";
String DISCARD_EVENT_MESSAGE = "DISCARD";
String CARD_ALLOWED_REQUEST_MESSAGE = "CARD_ALLOWED_REQUEST";
String DISTRIBUTE_TABLE_MESSAGE = "DISTRIBUTE TABLE";

bool is_distributing = false;

// enum page { GAME_LIST, GAME_DETAIL, GAME_CONFIG, GAME_PLAYING };
// page current_page = GAME_LIST;

// HELPER FUNCTIONS

void extract_excluded(const String &message, short output[], int &total_cards)
{
  const int max_size = 52;
  total_cards = 0;
  int startIndex = 0;
  int endIndex = 0;

  // Skip the first word (assumed to be "excludedCards")
  endIndex = message.indexOf(' ', startIndex);
  startIndex = endIndex + 1;

  while (startIndex < message.length() && total_cards < max_size)
  {
    endIndex = message.indexOf(' ', startIndex);
    if (endIndex == -1)
    {
      endIndex = message.length();
    }

    String numberStr = message.substring(startIndex, endIndex);
    short number = numberStr.toInt();
    output[total_cards++] = number;

    startIndex = endIndex + 1;
  }
}

String extract_before_space(String texto)
{
  int spaceIndex = texto.indexOf(' ');
  if (spaceIndex != -1)
  {
    String value = texto.substring(0, spaceIndex);
    return value;
  }
  else
  {
    return texto;
  }
}

void clean()
{
  tela.fillScreen(TFT_BLACK);
}

void clean_pos(int x1, int y1, int x2, int y2)
{
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
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

void update_distribution_message() {
  tela.fillRect(10, 120, 100, 40, TFT_BLACK);

  Instruction current_instruction = current_game.instructions[instruction_index];

  String current_distribution_message = "Para ";

  if (current_instruction.is_table) {
    current_distribution_message = current_distribution_message + "a mesa";
  } else {
    current_distribution_message = current_distribution_message + " jogador(a) " + String(current_player_index + 1);
  }

  tela.setCursor(10, 120);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
  tela.print(current_distribution_message);
}

void hide_distribute_button()
{
  tela.fillRect(20, 200, 200, 100, TFT_BLACK);
}

void check_card_allowed()
{
  Serial.println(CARD_ALLOWED_REQUEST_MESSAGE);
  is_distributing = true;
  hide_distribute_button();
}

bool is_current_instruction_the_last()
{
  Game current_game = game_list.get(game_index);
  return instruction_index == current_game.total_instructions - 1;
}

void update_instruction()
{
  tela.fillRect(130, 80, 100, 40, TFT_BLACK);

  String turn_message = "Distribuicao: " + String(instruction_index + 1);

  tela.setCursor(10, 80);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
  tela.print(turn_message);
}

void next_instruction()
{
  Game current_game = game_list.get(game_index);

  current_player_index = 0;
  current_player_cards_received = 0;

  is_distributing = false;

  if (is_current_instruction_the_last()) // When is the last instruction
  {
    instruction_index = 0;
    setup_game_over_screen();
  }
  else // Go to next instruction
  {
    instruction_index++;
    update_instruction();
  }
}

bool is_current_distribution_done()
{
  Game current_game = game_list.get(game_index);
  Instruction current_instruction = current_game.instructions[instruction_index];
  return current_player_cards_received >= current_instruction.card_amount; // Indexed by 0
}

bool is_current_player_the_last()
{
  return current_player_index == total_players - 1; // Indexed by 1
}

// Card distribute event consumer
void distribute_card(bool should_distribute)
{
  Game current_game = game_list.get(game_index);

  // In case card is not allowed, should discard
  if (!should_distribute)
  {
    Serial1.println(DISCARD_EVENT_MESSAGE);
    check_card_allowed();
    return;
  }

  Instruction current_instruction = current_game.instructions[instruction_index];

  if (current_instruction.is_table) // Distribute to table
  {
    Serial1.println(DISTRIBUTE_TABLE_MESSAGE);

    current_player_cards_received++;

    // Move to the next instruction || Distribute one more card
    if (is_current_distribution_done()) // The table should receive one more card
    {
      next_instruction();
    }
    else // The table already received all cards, move to the next instruction
    {
      check_card_allowed();
    }
  }
  else // Distribute to player
  {
    String distribute_player_message = "DISTRIBUTE " + String(current_player_index);

    Serial1.println(distribute_player_message);

    current_player_cards_received++;

    // Move to next instruction || Move to the next player || Distribute one more card
    if (is_current_distribution_done() && is_current_player_the_last()) // All players received cards, move to next instruction
    {
      next_instruction();
    }
    else if (is_current_distribution_done()) // Current player received all cards from instruction, move to next player
    {
      current_player_index++;
      current_player_cards_received = 0;
      check_card_allowed();
    }
    else // Current player should receive one more card
    {
      check_card_allowed();
    }
  }
}

void send_excluded_cards()
{
  Game current_game = game_list.get(game_index);
  String excluded_cards_message = "EXCLUDED_CARDS";

  for (int i = 0; i < current_game.total_excluded_cards; i++)
  {
    excluded_cards_message = excluded_cards_message + " " + String(current_game.excluded_cards[i]);
  }

  Serial.println(excluded_cards_message);
}

void start_game()
{
  Game current_game = game_list.get(game_index);
  String start_game_message = "START " + String(total_players);
  Serial.println(start_game_message);
  setup_is_playing_screen();
}

void add_players_count()
{
  Game current_game = game_list.get(game_index);

  if (total_players == current_game.max_players)
  {
    return;
  }

  total_players++;

  tela.fillRect(130, 50, 100, 40, TFT_BLACK);

  String players_count_message = "Jogadores:" + String(total_players);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);
}

void dec_players_count()
{
  Game current_game = game_list.get(game_index);

  if (total_players == current_game.min_players)
  {
    return;
  }

  total_players--;

  tela.fillRect(130, 50, 100, 40, TFT_BLACK);

  String players_count_message = "Jogadores:" + String(total_players);

  tela.setCursor(10, 60);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(3);
  tela.print(players_count_message);
}

void next_game_screen()
{
  if (game_index == total_items - 1)
  {
    game_index = 0;
  }
  else
  {
    game_index++;
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

  Game current_game = game_list.get(game_index);

  // game click button
  detail_button.init(&tela, &touch, 120, 130, 200, 100, TFT_WHITE, TFT_GREEN, TFT_BLACK, current_game.name, 2);
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

  Game current_game = game_list.get(game_index);
  total_players = current_game.min_players;

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(current_game.name);

  String max_message = "Maximo: " + String(current_game.max_players);
  String min_message = "Minimo: " + String(current_game.min_players);

  tela.setCursor(10, 80);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
  tela.print(min_message);

  tela.setCursor(10, 110);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
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

  Game current_game = game_list.get(game_index);

  String players_count_message = "Jogadores: " + String(total_players);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(current_game.name);

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

  Game current_game = game_list.get(game_index);

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print(current_game.name);

  String turn_message = "Distribuicao: " + String(instruction_index + 1);

  tela.setCursor(10, 80);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
  tela.print(turn_message);

  update_distribution_message();

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

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(4);
  tela.print("Game over");

  // distribute next cards button
  play_button.init(&tela, &touch, 170, 250, 100, 80, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Replay", 2);
  play_button.setPressHandler(setup_is_playing_screen);

  // go foward button
  back_button.init(&tela, &touch, 60, 250, 100, 80, TFT_WHITE, TFT_RED, TFT_WHITE, "Sair", 2);
  back_button.setPressHandler(setup_game_list_screen);
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

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
    if (!is_distributing)
    {
      play_button.process();
    }
  }
  else if (game_over_page)
  { // Game over page
    play_button.process();
    back_button.process();
  }
  else
  {
    Serial.println("[ERROR] Unknown page status");
  }

  if (Serial.available() > 0)
  {
    String message = Serial.readStringUntil('\n');
    message.trim();
    if (message.length() > 0)
    {

      // Handle card recognition event consumer
      if (message.startsWith(CARD_ALLOWED_RESPONSE) && is_distributing)
      {
        String card_allowed_value = extract_after_space(message);
        card_allowed_value.toUpperCase();
        bool should_distribute = card_allowed_value.startsWith(CARD_ALLOWED_RESPONSE_VALUE_TRUE);
        distribute_card(should_distribute);
      }

      // Handle games database sync
      if (message.startsWith("reset"))
      {
        game_list.clear();
        EEPROM.put(0, 0);
        Serial.println("ACK");
      }
      if (message.startsWith("name"))
      {
        String name_value = extract_after_space(message);
        name_value.toCharArray(current_game_to_save.name, 20);
        Serial.println("ACK");
      }
      else if (message.startsWith("minPlayers"))
      {
        current_game_to_save.min_players = (short)extract_after_space(message).toInt();
        Serial.println("ACK");
      }
      else if (message.startsWith("maxPlayers"))
      {
        current_game_to_save.max_players = (short)extract_after_space(message).toInt();
        Serial.println("ACK");
      }
      else if (message.startsWith("players"))
      {
        current_game_to_save.instructions[current_game_to_save_instruction_count].is_table = false;
        current_game_to_save.instructions[current_game_to_save_instruction_count].card_amount = (short)extract_after_space(message).toInt();
        current_game_to_save_instruction_count++;
        Serial.println("ACK");
      }
      else if (message.startsWith("table"))
      {
        current_game_to_save.instructions[current_game_to_save_instruction_count].is_table = true;
        current_game_to_save.instructions[current_game_to_save_instruction_count].card_amount = (short)extract_after_space(message).toInt();
        current_game_to_save_instruction_count++;
        Serial.println("ACK");
      }
      else if (message.startsWith("excludedCards"))
      {
        short excluded_cards[52];
        int total_excluded_cards;
        extract_excluded(message, excluded_cards, total_excluded_cards);
        for (int i = 0; i < total_excluded_cards; i++)
        {
          current_game_to_save.excluded_cards[i] = excluded_cards[i];
        }
        current_game_to_save.total_excluded_cards = total_excluded_cards;
        Serial.println("ACK");
      }
      else if (message.startsWith("endgame"))
      {
        current_game_to_save.total_instructions = current_game_to_save_instruction_count;
        current_game_to_save_instruction_count = 0;
        game_list_to_save.add(current_game_to_save);
        Serial.println("ACK");
      }
      else if (message.startsWith("cambiodesligo"))
      {
        short total_seed_items = game_list_to_save.size();
        EEPROM.put(0, total_seed_items);
        for (int i = 0; i < total_seed_items; i++)
        {
          EEPROM.put(sizeof(total_seed_items) + (sizeof(Game) * i), game_list_to_save.get(i));
        }
        Serial.println("ACK");
      }
    }
  }
}