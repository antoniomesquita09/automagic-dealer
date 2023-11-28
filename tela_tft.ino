#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <JKSButton.h>

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);

// button_[page]_[index]
JKSButton button_1_1, button_1_2, button_1_3; // buttons from first page
JKSButton button_2_1, button_2_2, button_2_3; // buttons from second page
JKSButton button_3_1, button_3_2, button_3_3; // buttons from second page

int screen_state = 1;

void clean() {
  tela.fillScreen(TFT_BLACK);
}

void setup_screen_one() {
  clean();

  tela.setCursor(10, 10);
  tela.setTextColor(TFT_DARKGREY);
  tela.setTextSize(4);
  tela.print("Escolha o game");
  
  // go back button
  button_1_1.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Poker", 2);
  button_1_1.setPressHandler(backward_page);
  
  // go foward button
  button_1_2.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Poker", 2);
  button_1_2.setPressHandler(forward_page);
}

void setup_screen_two() {
  clean();
  
  // go back button
  button_2_1.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Voltar", 2);
  button_2_1.setPressHandler(backward_page);

  // go foward button
  button_2_2.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Próxima", 2);
  button_2_2.setPressHandler(forward_page);
}

void setup_screen_three() {
  clean();

  // go back button
  button_3_1.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Voltar", 2);
  button_3_1.setPressHandler(backward_page);
}

void setup() {
  tela.begin(tela.readID());
  setup_screen_one();
}

void loop() {
  if (screen_state == 1) { // first screen
    button_1_1.process();
    button_1_2.process();
    button_1_3.process();
  } else if (screen_state == 2) { // second screen
    button_2_1.process();
    button_2_2.process();
    button_2_3.process();
  } else if (screen_state == 3) { // Third screen
    button_3_1.process();
    button_3_2.process();
    button_3_3.process();
  } else {
    // throw an error
  }
}

void forward_page(JKSButton &botaoPressionado) {
  if (screen_state == 1) {
    screen_state = 2;
    setup_screen_two();
  } else {
    screen_state = 3;
    setup_screen_three();
  }
}

void backward_page(JKSButton &botaoPressionado) {
  if (screen_state == 2) {
    screen_state = 1;
    setup_screen_one();
  } else {
    screen_state = 2;
    setup_screen_two();
  }
}

void clean_pos(int x1, int y1, int x2, int y2) {
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
}
