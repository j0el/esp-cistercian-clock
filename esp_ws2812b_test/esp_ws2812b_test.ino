#include <FastLED.h>

#define LED_PIN   2    // GPIO2 = D4 on NodeMCU
#define NUM_LEDS   64
#define BRIGHTNESS 1   // keep low when powering from USB (64 LEDs at full white = ~3.8A)

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== WS2812B 64-LED Test ===");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

// Chase a single lit pixel across the strip
void chase(CRGB color, int delayMs) {
  for (int i = 0; i < NUM_LEDS; i++) {
    FastLED.clear();
    leds[i] = color;
    FastLED.show();
    Serial.printf("  LED %d\n", i);
    delay(delayMs);
  }
  FastLED.clear();
  FastLED.show();
}

// Fill entire strip with one color
void fill(const char* name, CRGB color, int holdMs) {
  Serial.printf("  FILL %s\n", name);
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
  delay(holdMs);
}

void loop() {
  Serial.println("--- Chase RED ---");
  chase(CRGB::Red, 30);

  Serial.println("--- Chase GREEN ---");
  chase(CRGB::Green, 30);

  Serial.println("--- Chase BLUE ---");
  chase(CRGB::Blue, 30);

  Serial.println("--- Fill cycle ---");
  fill("RED",   CRGB::Red,   800);
  fill("GREEN", CRGB::Green, 800);
  fill("BLUE",  CRGB::Blue,  800);
  fill("WHITE", CRGB::White, 800);
  fill("OFF",   CRGB::Black, 400);
}
