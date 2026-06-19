#include <FastLED.h>

// ── Config ────────────────────────────────────────────────────────────────────

#define LED_PIN    2
#define NUM_LEDS   64
#define BRIGHTNESS 10
#define BUTTON_PIN 0   // GPIO0 — the FLASH button

CRGB leds[NUM_LEDS];

// ── Snake ─────────────────────────────────────────────────────────────────────
//
//  One-button control: short press = turn right (clockwise).
//  Snake wraps around edges. Eating food grows the snake by 1.
//  On collision with own body: flash red, restart.
//  On filling the grid: flash green, restart.
//
//  Direction encoding: 0=RIGHT, 1=DOWN, 2=LEFT, 3=UP

#define SNAKE_MAX_LEN 64
#define SNAKE_STEP_MS 600

static const int8_t SNK_DR[] = { 0,  1,  0, -1};
static const int8_t SNK_DC[] = { 1,  0, -1,  0};

int8_t  snkRow[SNAKE_MAX_LEN];
int8_t  snkCol[SNAKE_MAX_LEN];
int     snkLen     = 0;
int     snkDir     = 0;
int     snkNextDir = 0;
int8_t  snkFoodR   = 0;
int8_t  snkFoodC   = 0;
int     snkScore   = 0;
unsigned long snkLastStepMs = 0;

bool snkOnBody(int r, int c, int checkLen) {
    for (int i = 0; i < checkLen; i++)
        if (snkRow[i] == r && snkCol[i] == c) return true;
    return false;
}

void snkPlaceFood() {
    if (snkLen >= SNAKE_MAX_LEN) return;
    do {
        snkFoodR = random(8);
        snkFoodC = random(8);
    } while (snkOnBody(snkFoodR, snkFoodC, snkLen));
}

void resetSnake() {
    snkLen     = 3;
    snkRow[0]  = 3; snkCol[0] = 5;   // head
    snkRow[1]  = 3; snkCol[1] = 4;
    snkRow[2]  = 3; snkCol[2] = 3;   // tail
    snkDir     = 0;   // RIGHT
    snkNextDir = 0;
    snkScore   = 0;
    snkLastStepMs = 0;
    randomSeed(analogRead(0));
    snkPlaceFood();
}

void snkDisplay() {
    FastLED.clear();
    leds[snkFoodR * 8 + snkFoodC] = CRGB(200, 0, 0);
    for (int i = snkLen - 1; i >= 0; i--) {
        uint8_t g = (i == 0) ? 255
                  : (snkLen > 1) ? map(i, 1, snkLen - 1, 180, 40) : 180;
        leds[snkRow[i] * 8 + snkCol[i]] = CRGB(0, g, 0);
    }
    FastLED.show();
}

void snkFlash(CRGB col, int times, int onMs, int offMs) {
    for (int f = 0; f < times; f++) {
        fill_solid(leds, NUM_LEDS, col);
        FastLED.show(); delay(onMs);
        FastLED.clear();
        FastLED.show(); delay(offMs);
    }
}

void runSnake() {
    if (millis() - snkLastStepMs < SNAKE_STEP_MS) { delay(10); return; }
    snkLastStepMs = millis();

    if (snkNextDir != (snkDir + 2) % 4) snkDir = snkNextDir;

    int nr = (snkRow[0] + SNK_DR[snkDir] + 8) % 8;
    int nc = (snkCol[0] + SNK_DC[snkDir] + 8) % 8;

    if (snkOnBody(nr, nc, snkLen - 1)) {
        Serial.printf("Snake died! Score: %d\n", snkScore);
        snkFlash(CRGB(150, 0, 0), 3, 200, 150);
        resetSnake();
        return;
    }

    bool ate = (nr == snkFoodR && nc == snkFoodC);
    if (ate && snkLen < SNAKE_MAX_LEN) snkLen++;

    for (int i = snkLen - 1; i > 0; i--) {
        snkRow[i] = snkRow[i - 1];
        snkCol[i] = snkCol[i - 1];
    }
    snkRow[0] = nr;
    snkCol[0] = nc;

    if (ate) {
        snkScore++;
        Serial.printf("Score: %d\n", snkScore);
        if (snkLen >= SNAKE_MAX_LEN) {
            Serial.println("Snake wins!");
            snkFlash(CRGB(0, 150, 0), 5, 300, 200);
            resetSnake();
            return;
        }
        snkPlaceFood();
    }

    snkDisplay();
}

// ── Button ────────────────────────────────────────────────────────────────────
//
//  Short press (< 1500ms): turn right (clockwise)
//  Long press (>= 1500ms): restart game

void checkButton() {
    static unsigned long pressedAt = 0;
    static bool wasDown            = false;
    static bool actionTaken        = false;

    bool down = (digitalRead(BUTTON_PIN) == LOW);

    if (down && !wasDown) {
        pressedAt   = millis();
        wasDown     = true;
        actionTaken = false;
    }

    if (down && !actionTaken && millis() - pressedAt >= 1500) {
        actionTaken = true;
        Serial.println("Long press: restarting");
        resetSnake();
    }

    if (!down && wasDown) {
        unsigned long held = millis() - pressedAt;
        wasDown = false;
        if (!actionTaken && held >= 50) {
            snkNextDir = (snkDir + 1) % 4;
        }
    }
}

// ── Setup / Loop ──────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Snake ===");
    Serial.println("Short press: turn right");
    Serial.println("Long press:  restart");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    resetSnake();
}

void loop() {
    checkButton();
    runSnake();
}
