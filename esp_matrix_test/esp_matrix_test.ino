#include <FastLED.h>

#define LED_PIN    2
#define NUM_LEDS   64
#define BRIGHTNESS 8

CRGB leds[NUM_LEDS];

void allOff() { FastLED.clear(); FastLED.show(); delay(300); }

// Test 1: light each LED in index order (0→63), 150ms each
// Watch which physical direction the dot travels — tells us the wiring path
void test_indexOrder() {
    Serial.println("TEST 1: Index order 0-63 (watch the dot travel)");
    for (int i = 0; i < NUM_LEDS; i++) {
        FastLED.clear();
        leds[i] = CRGB::White;
        FastLED.show();
        Serial.printf("  LED %d\n", i);
        delay(150);
    }
    allOff();
}

// Test 2: light one full row at a time, top to bottom by index group
// Row 0 = LEDs 0-7, Row 1 = LEDs 8-15, etc.
void test_rawRows() {
    Serial.println("TEST 2: Raw rows (LEDs 0-7, 8-15, ...) one at a time");
    for (int row = 0; row < 8; row++) {
        FastLED.clear();
        for (int i = 0; i < 8; i++) leds[row * 8 + i] = CRGB::Red;
        FastLED.show();
        Serial.printf("  Raw row %d (LEDs %d-%d)\n", row, row*8, row*8+7);
        delay(800);
    }
    allOff();
}

// Test 3: light one column at a time using serpentine mapping
// Column goes top-to-bottom; even rows L→R, odd rows R→L
void test_serpentineCol(int col) {
    FastLED.clear();
    for (int row = 0; row < 8; row++) {
        int idx = (row & 1) ? row * 8 + (7 - col) : row * 8 + col;
        leds[idx] = CRGB::Green;
    }
    FastLED.show();
    Serial.printf("  Serpentine col %d\n", col);
    delay(600);
}

// Test 4: light corners and edges to identify orientation
void test_corners() {
    Serial.println("TEST 4: Corners — note which physical corner lights");

    FastLED.clear(); leds[0] = CRGB::Red; FastLED.show();
    Serial.println("  LED 0 (should be one corner)"); delay(1200);

    FastLED.clear(); leds[7] = CRGB::Green; FastLED.show();
    Serial.println("  LED 7 (same row, other end)"); delay(1200);

    FastLED.clear(); leds[56] = CRGB::Blue; FastLED.show();
    Serial.println("  LED 56 (opposite corner row)"); delay(1200);

    FastLED.clear(); leds[63] = CRGB::White; FastLED.show();
    Serial.println("  LED 63 (last LED)"); delay(1200);

    allOff();
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Matrix Wiring Test ===");
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    delay(500);
}

void loop() {
    test_corners();
    delay(500);

    test_indexOrder();
    delay(500);

    test_rawRows();
    delay(500);

    Serial.println("TEST 3: Serpentine columns 0-7");
    for (int c = 0; c < 8; c++) test_serpentineCol(c);
    allOff();
    delay(1000);
}
