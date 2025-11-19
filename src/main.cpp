#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <esp_system.h>

const int PIN_TFT_CS = 5;
const int PIN_TFT_DC = 16;
const int PIN_TFT_RST = 17;
const int PIN_TFT_SCLK = 18;
const int PIN_TFT_MOSI = 23;
const int PIN_BUTTON = 4;

const int DICE_SIZE = 160;
const int DICE_CORNER_RADIUS = 24;
const int DICE_DOT_RADIUS = 14;
const uint16_t COLOR_BG = GC9A01A_RED;
const uint16_t COLOR_DICE = GC9A01A_WHITE;
const uint16_t COLOR_DICE_BORDER = GC9A01A_WHITE;
const uint16_t COLOR_DICE_DOT = GC9A01A_BLACK;

const int DICE_STEPS = 20;
const int FAST_DELAY_MS = 60;
const int MEDIUM_DELAY_MS = 200;
const int SLOW_DELAY_1_MS = 400;
const int SLOW_DELAY_2_MS = 650;
const int SLOW_DELAY_3_MS = 950;

Adafruit_GC9A01A tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
bool showPromptScreen = true;
int gridCenterX = 0;
int gridCenterY = 0;
int gridStep = 0;

void drawPrompt() {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(GC9A01A_BLACK);
  tft.setTextSize(3);

  const char *line1 = "Spieler Rot";
  const char *line2 = "ist dran";

  int16_t x1, y1;
  uint16_t w1, h1;
  tft.getTextBounds(line1, 0, 0, &x1, &y1, &w1, &h1);
  int16_t x2, y2;
  uint16_t w2, h2;
  tft.getTextBounds(line2, 0, 0, &x2, &y2, &w2, &h2);

  int line1Y = (tft.height() / 2) - h1;
  int line2Y = line1Y + h1 + 10;

  tft.setCursor((tft.width() - w1) / 2, line1Y);
  tft.print(line1);
  tft.setCursor((tft.width() - w2) / 2, line2Y);
  tft.print(line2);

  showPromptScreen = true;
}

void drawDiceBackground() {
  int originX = (tft.width() - DICE_SIZE) / 2;
  int originY = (tft.height() - DICE_SIZE) / 2;
  gridCenterX = originX + DICE_SIZE / 2;
  gridCenterY = originY + DICE_SIZE / 2;
  gridStep = DICE_SIZE / 4;

  tft.fillRoundRect(originX, originY, DICE_SIZE, DICE_SIZE, DICE_CORNER_RADIUS, COLOR_DICE);
  tft.drawRoundRect(originX, originY, DICE_SIZE, DICE_SIZE, DICE_CORNER_RADIUS, COLOR_DICE_BORDER);
}

void drawDot(int dx, int dy) {
  int x = gridCenterX + dx * gridStep;
  int y = gridCenterY + dy * gridStep;
  tft.fillCircle(x, y, DICE_DOT_RADIUS, COLOR_DICE_DOT);
}

void drawDiceFace(int face) {
  drawDiceBackground();

  if (face == 1) {
    drawDot(0, 0);
  } else if (face == 2) {
    drawDot(-1, -1);
    drawDot(1, 1);
  } else if (face == 3) {
    drawDot(-1, -1);
    drawDot(0, 0);
    drawDot(1, 1);
  } else if (face == 4) {
    drawDot(-1, -1);
    drawDot(1, -1);
    drawDot(-1, 1);
    drawDot(1, 1);
  } else if (face == 5) {
    drawDot(-1, -1);
    drawDot(1, -1);
    drawDot(0, 0);
    drawDot(-1, 1);
    drawDot(1, 1);
  } else if (face == 6) {
    drawDot(-1, -1);
    drawDot(1, -1);
    drawDot(-1, 0);
    drawDot(1, 0);
    drawDot(-1, 1);
    drawDot(1, 1);
  }

  showPromptScreen = false;
}

void waitMs(int ms) {
  delay(ms);
}

void rollDiceAnimation() {
  tft.fillScreen(COLOR_BG);

  for (int i = 0; i < DICE_STEPS; i++) {
    int value = random(6) + 1;
    drawDiceFace(value);

    if (i < DICE_STEPS - 3) {
      int stepDelay = map(i, 0, DICE_STEPS - 4, FAST_DELAY_MS, MEDIUM_DELAY_MS);
      waitMs(stepDelay);
    } else if (i == DICE_STEPS - 3) {
      waitMs(SLOW_DELAY_1_MS);
    } else if (i == DICE_STEPS - 2) {
      waitMs(SLOW_DELAY_2_MS);
    } else {
      waitMs(SLOW_DELAY_3_MS);
    }
  }

  int finalValue = random(6) + 1;
  drawDiceFace(finalValue);
  Serial.printf("Spieler Rot: %d gewuerfelt\n", finalValue);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  SPI.begin(PIN_TFT_SCLK, -1, PIN_TFT_MOSI);
  tft.begin(40000000);
  tft.setRotation(0);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  randomSeed(esp_random());

  drawPrompt();
}

void loop() {
  static bool lastButtonState = true;
  bool buttonNow = digitalRead(PIN_BUTTON);

  if (!buttonNow && lastButtonState) {
    if (showPromptScreen) {
      rollDiceAnimation();
    } else {
      drawPrompt();
    }
  }

  lastButtonState = buttonNow;
  delay(10);
}