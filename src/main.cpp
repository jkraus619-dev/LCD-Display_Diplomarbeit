#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <esp_system.h>

// Passe die Pins an deine Verdrahtung an.
constexpr int TFT_CS = 5;
constexpr int TFT_DC = 16;
constexpr int TFT_RST = 17;
constexpr int TFT_SCLK = 18; //SCL-Pin
constexpr int TFT_MOSI = 23; //SDA-Pin
constexpr int BUTTON_PIN = 4;  // Taste zwischen Pin und GND, interner Pull-up aktiv

constexpr int DICE_SIZE = 160;
constexpr int DICE_CORNER_RADIUS = 24;
constexpr int DICE_DOT_RADIUS = 14;
constexpr uint16_t SCREEN_BG = GC9A01A_RED;
constexpr uint16_t DICE_FILL_COLOR = GC9A01A_WHITE;
constexpr uint16_t DICE_BORDER_COLOR = GC9A01A_WHITE;
constexpr uint16_t DICE_DOT_COLOR = GC9A01A_BLACK;
constexpr uint8_t ROLL_STEPS = 18;
constexpr uint16_t FRAME_DELAY_START_MS = 70;
constexpr uint16_t FRAME_DELAY_MID_MS = 280;
constexpr uint16_t FINAL_STEP_DELAYS_MS[3] = {460, 720, 1000};

struct DiceDotOffset {
  int8_t dx;
  int8_t dy;
};

constexpr DiceDotOffset kDiceDotOffsets[] = {
    {0, 0},   // center
    {-1, -1}, // top-left
    {1, -1},  // top-right
    {-1, 0},  // middle-left
    {1, 0},   // middle-right
    {-1, 1},  // bottom-left
    {1, 1}    // bottom-right
};

constexpr uint8_t kFaceDotCounts[] = {1, 2, 3, 4, 5, 6};

constexpr uint8_t kFaceDotIndices[6][6] = {
    {0, 0, 0, 0, 0, 0},  // 1 //0, weil 0. Eintrag im Array
    {1, 6, 0, 0, 0, 0},  // 2 //1 und 6, weil 1. und 6. Eintrag im Array
    {0, 1, 6, 0, 0, 0},  // 3 // usw.
    {1, 2, 5, 6, 0, 0},  // 4
    {0, 1, 2, 5, 6, 0},  // 5  
    {1, 2, 3, 4, 5, 6}   // 6
};

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

void drawDiceFace(uint8_t face) {
  face = constrain(face, 1, 6);

  int16_t originX = (tft.width() - DICE_SIZE) / 2;
  int16_t originY = (tft.height() - DICE_SIZE) / 2;
  int16_t centerX = originX + DICE_SIZE / 2;
  int16_t centerY = originY + DICE_SIZE / 2;
  int16_t dotSpacing = DICE_SIZE / 4;

  tft.fillRoundRect(originX, originY, DICE_SIZE, DICE_SIZE, DICE_CORNER_RADIUS, DICE_FILL_COLOR);
  tft.drawRoundRect(originX, originY, DICE_SIZE, DICE_SIZE, DICE_CORNER_RADIUS, DICE_BORDER_COLOR);

  uint8_t dotCount = kFaceDotCounts[face - 1];
  for (uint8_t i = 0; i < dotCount; ++i) {
    uint8_t dotIndex = kFaceDotIndices[face - 1][i];
    const DiceDotOffset &offset = kDiceDotOffsets[dotIndex];
    int16_t px = centerX + offset.dx * dotSpacing;
    int16_t py = centerY + offset.dy * dotSpacing;
    tft.fillCircle(px, py, DICE_DOT_RADIUS, DICE_DOT_COLOR);
  }
}

uint8_t rollDice() {
  uint8_t face = 1;

  for (uint8_t step = 0; step < ROLL_STEPS; ++step) {
    face = random(6) + 1;
    drawDiceFace(face);

    uint16_t delayMs;
    if (step >= ROLL_STEPS - 3) {
      uint8_t slowIndex = step - (ROLL_STEPS - 3);
      delayMs = FINAL_STEP_DELAYS_MS[slowIndex];
    } else {
      delayMs = FRAME_DELAY_START_MS;
      uint8_t rampSteps = (ROLL_STEPS > 3) ? (ROLL_STEPS - 3) : 1;
      if (rampSteps > 1) {
        delayMs += ((FRAME_DELAY_MID_MS - FRAME_DELAY_START_MS) * step) / (rampSteps - 1);
      }
    }
    delay(delayMs);
  }

  face = random(6) + 1;
  drawDiceFace(face);
  return face;
}

void setup() {
  delay(100);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI);
  tft.begin(40000000);
  tft.setRotation(0);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  randomSeed(esp_random());

  tft.fillScreen(SCREEN_BG);
  drawDiceFace(1);
}

void loop() {
  static bool lastButtonState = true;
  bool currentState = digitalRead(BUTTON_PIN);

  if (!currentState && lastButtonState) {
    rollDice();
  }

  lastButtonState = currentState;
  delay(10);
}
