#include <FastLED.h>

// ── Config ────────────────────────────────────────────────────────────────────

#define LED_PIN    2
#define NUM_LEDS   64
#define BRIGHTNESS 10
#define BUTTON_PIN 0   // GPIO0 — the FLASH button

CRGB leds[NUM_LEDS];

// ── Mandelbrot ────────────────────────────────────────────────────────────────
//
//  Precomputed on 8×8.  Four views cycle every MAND_VIEW_MS seconds.
//  Boundary pixels animate with a slowly rotating rainbow hue so the
//  iteration-count contours flow with color.  Inside-set pixels stay black.
//
//  Note: ESP8266 has no FPU so floats are software-emulated, but 64 pixels
//  × 48 iterations takes only a few milliseconds — recompute is imperceptible.
//
//  Short press: advance to next view immediately.
//  Long press (>= 1500ms): same.

#define MAND_MAX_ITER 48
#define MAND_VIEW_MS  15000   // ms per view before auto-cycling
#define MAND_HUE_MS   60      // ms between color steps

struct MandView { float reMin, reMax, imMin, imMax; const char* name; };

static const MandView MAND_VIEWS[] = {
    {-2.5f,  1.0f, -1.25f,  1.25f, "Full"},
    {-2.2f, -0.4f, -1.0f,   1.0f,  "West"},
    {-1.0f,  0.4f, -0.75f,  0.75f, "Cardioids"},
    {-0.72f, 0.28f,-0.55f,  0.55f, "Core"},
};
#define MAND_NUM_VIEWS 4

uint8_t mandIter[64];
static unsigned long lastHueMs   = 0;
static unsigned long viewStartMs = 0;
static uint8_t  hue     = 0;
static int      viewIdx = 0;

void mandCompute(int vi) {
    const MandView& v = MAND_VIEWS[vi];
    for (int r = 0; r < 8; r++) {
        float im = v.imMax - (r / 7.0f) * (v.imMax - v.imMin);
        for (int c = 0; c < 8; c++) {
            float re = v.reMin + (c / 7.0f) * (v.reMax - v.reMin);
            float zr = 0, zi = 0;
            uint8_t it = MAND_MAX_ITER;
            for (int i = 0; i < MAND_MAX_ITER; i++) {
                float zr2 = zr*zr - zi*zi + re;
                zi = 2.0f*zr*zi + im;
                zr = zr2;
                if (zr*zr + zi*zi > 4.0f) { it = i; break; }
            }
            mandIter[r*8+c] = it;
        }
    }
    Serial.printf("Mandelbrot: %s\n", MAND_VIEWS[vi].name);
}

void nextView() {
    viewIdx = (viewIdx + 1) % MAND_NUM_VIEWS;
    viewStartMs = millis();
    mandCompute(viewIdx);
}

void runMandelbrot() {
    unsigned long now_ms = millis();

    if (now_ms - viewStartMs >= MAND_VIEW_MS) {
        nextView();
    }

    if (now_ms - lastHueMs < MAND_HUE_MS) { delay(10); return; }
    lastHueMs = now_ms;
    hue += 2;

    FastLED.clear();
    for (int i = 0; i < 64; i++) {
        if (mandIter[i] >= MAND_MAX_ITER) {
            leds[i] = CRGB::Black;
        } else {
            uint8_t h = hue + (uint8_t)(mandIter[i] * 5);
            leds[i] = CHSV(h, 255, 200);
        }
    }
    FastLED.show();
}

// ── Button ────────────────────────────────────────────────────────────────────

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
        nextView();
    }

    if (!down && wasDown) {
        unsigned long held = millis() - pressedAt;
        wasDown = false;
        if (!actionTaken && held >= 50) {
            nextView();
        }
    }
}

// ── Setup / Loop ──────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Mandelbrot ===");
    Serial.println("Short/Long press: next view");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    viewStartMs = millis();
    mandCompute(0);
}

void loop() {
    checkButton();
    runMandelbrot();
}
