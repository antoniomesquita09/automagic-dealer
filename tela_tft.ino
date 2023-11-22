#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <JKSButton.h>

MCUFRIEND_kbv tela;
TouchScreen touch(6, A1, A2, 7, 300);
JKSButton botao1, botao2;

int screen_state = 0;

void setup_screen_one() {
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(10, 10);
  tela.setTextColor(TFT_DARKGREY);
  tela.setTextSize(4);
  tela.print("Escolha o game");
  botao1.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Poker", 2);
  botao1.setPressHandler(change_state);
}

void setup_screen_two() {
  tela.fillScreen(TFT_BLACK);
  botao2.init(&tela, &touch, 120, 170, 200, 100, TFT_WHITE, TFT_PURPLE, TFT_BLACK, "Voltar", 2);
  botao2.setPressHandler(change_state);
}

void setup() {
  tela.begin(tela.readID());
  setup_screen_one();
}

void loop() {
  if (screen_state == 0) {
    botao1.process();
  } else {
    botao2.process();
  }
}

void change_state(JKSButton &botaoPressionado) {
  if (screen_state == 0) {
    screen_state = 1;
    setup_screen_two();
  } else {
    screen_state = 0;
    setup_screen_one();
  }
}

void clean(int x1, int y1, int x2, int y2) {
  tela.fillRect(x1, x2, y1, y2, TFT_BLACK);
}
