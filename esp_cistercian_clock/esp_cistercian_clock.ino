#include <ESP8266WiFi.h>
#include <time.h>
#include <FastLED.h>

// ── WiFi ──────────────────────────────────────────────────────────────────────
const char* ssid     = "Detonator";
const char* password = "Emerson03";

// ── NTP ───────────────────────────────────────────────────────────────────────
const long gmtOffset = -28800;  // UTC-8 (US Pacific Standard)
const int  dstOffset = 3600;

// ── LED matrix ────────────────────────────────────────────────────────────────
#define LED_PIN    2
#define NUM_LEDS   64
#define BRIGHTNESS 10

CRGB fgColor = CRGB(0, 0, 40);   // dark blue — Cistercian strokes

CRGB leds[NUM_LEDS];

// ── Matrix layout ─────────────────────────────────────────────────────────────
//
//  Standard raster: LED index = row*8 + col, row 0 = top, col 0 = left.
//
//  col:  0   1  2  3 | 4 | 5  6  7
//        ────────────────────────────
//  row 0  ← green dot border (unused by glyph) →
//  row 1  .  TENS  |stave| UNITS  .
//  row 2  .        |     |        .  ← upper half
//  row 3  .        |     |        .
//        ──────────── gap ─────────
//  row 4  .  THOU  |     | HUND   .
//  row 5  .        |     |        .  ← lower half
//  row 6  .        |     |        .
//  row 7  .        |     |        .
//        (col 0 also part of green dot border)
//
//  Cistercian glyph occupies rows 1-7, cols 1-7 (7×7).
//  Within this 7×7 area, LOCAL coords 0-6 are used.
//
//  Local layout (0-indexed within 7×7):
//    cols 0-2: left quadrant  (TENS / THOUSANDS)
//    col  3:   center stave
//    cols 4-6: right quadrant (UNITS / HUNDREDS)
//    rows 0-2: upper half
//    row  3:   center gap (stave only)
//    rows 4-6: lower half

int ledIndex(int row, int col) { return row * 8 + col; }

// Draw a Cistercian pixel at 7×7 local coords (offset +1 into the 8×8 grid)
void setLED(int lc, int lr) {
    int col = lc + 1, row = lr + 1;
    if (col < 1 || col > 7 || row < 1 || row > 7) return;
    leds[ledIndex(row, col)] = fgColor;
}

// ── Cistercian stroke renderer ────────────────────────────────────────────────
//
//  Each quadrant occupies a 3×3 pixel area (3 cols × 3 rows).
//  Quadrant corners in local coords:
//    UNITS     (upper-right): TL=(4,0) TR=(6,0) BL=(4,2) BR=(6,2)
//    TENS      (upper-left):  TL=(2,0) TR=(0,0) BL=(2,2) BR=(0,2)
//    HUNDREDS  (lower-right, v-flip): TL=(4,6) TR=(6,6) BL=(4,4) BR=(6,4)
//    THOUSANDS (lower-left):  TL=(2,6) TR=(0,6) BL=(2,4) BR=(0,4)
//
//  Strokes (from Cistercian.kt rawStrokes):
//    1: TL→TR   2: BL→BR   3: TL→BR   4: BL→TR
//    5: TL→TR + BL→TR      6: TR→BR
//    7: TL→TR + TR→BR      8: BL→BR + TR→BR
//    9: TL→TR + BL→BR + TR→BR

void drawStave() {
    for (int r = 0; r < 7; r++) setLED(3, r);
}

void drawUnits(int d) {
    // TL=(4,0) TR=(6,0) BL=(4,2) BR=(6,2)
    switch (d) {
        case 1: setLED(4,0); setLED(5,0); setLED(6,0); break;
        case 2: setLED(4,2); setLED(5,2); setLED(6,2); break;
        case 3: setLED(4,0); setLED(5,1); setLED(6,2); break;
        case 4: setLED(4,2); setLED(5,1); setLED(6,0); break;
        case 5: setLED(4,0); setLED(5,0); setLED(6,0);
                setLED(4,2); setLED(5,1); break;
        case 6: setLED(6,0); setLED(6,1); setLED(6,2); break;
        case 7: setLED(4,0); setLED(5,0); setLED(6,0);
                              setLED(6,1); setLED(6,2); break;
        case 8: setLED(4,2); setLED(5,2); setLED(6,2);
                setLED(6,0); setLED(6,1); break;
        case 9: setLED(4,0); setLED(5,0); setLED(6,0);
                setLED(4,2); setLED(5,2); setLED(6,2);
                              setLED(6,1); break;
    }
}

void drawTens(int d) {
    // TL=(2,0) TR=(0,0) BL=(2,2) BR=(0,2)
    switch (d) {
        case 1: setLED(2,0); setLED(1,0); setLED(0,0); break;
        case 2: setLED(2,2); setLED(1,2); setLED(0,2); break;
        case 3: setLED(2,0); setLED(1,1); setLED(0,2); break;
        case 4: setLED(2,2); setLED(1,1); setLED(0,0); break;
        case 5: setLED(2,0); setLED(1,0); setLED(0,0);
                setLED(2,2); setLED(1,1); break;
        case 6: setLED(0,0); setLED(0,1); setLED(0,2); break;
        case 7: setLED(2,0); setLED(1,0); setLED(0,0);
                              setLED(0,1); setLED(0,2); break;
        case 8: setLED(2,2); setLED(1,2); setLED(0,2);
                setLED(0,0); setLED(0,1); break;
        case 9: setLED(2,0); setLED(1,0); setLED(0,0);
                setLED(2,2); setLED(1,2); setLED(0,2);
                              setLED(0,1); break;
    }
}

void drawHundreds(int d) {
    // TL=(4,6) TR=(6,6) BL=(4,4) BR=(6,4)  [v-flipped]
    switch (d) {
        case 1: setLED(4,6); setLED(5,6); setLED(6,6); break;
        case 2: setLED(4,4); setLED(5,4); setLED(6,4); break;
        case 3: setLED(4,6); setLED(5,5); setLED(6,4); break;
        case 4: setLED(4,4); setLED(5,5); setLED(6,6); break;
        case 5: setLED(4,6); setLED(5,6); setLED(6,6);
                setLED(4,4); setLED(5,5); break;
        case 6: setLED(6,6); setLED(6,5); setLED(6,4); break;
        case 7: setLED(4,6); setLED(5,6); setLED(6,6);
                              setLED(6,5); setLED(6,4); break;
        case 8: setLED(4,4); setLED(5,4); setLED(6,4);
                setLED(6,6); setLED(6,5); break;
        case 9: setLED(4,6); setLED(5,6); setLED(6,6);
                setLED(4,4); setLED(5,4); setLED(6,4);
                              setLED(6,5); break;
    }
}

void drawThousands(int d) {
    // TL=(2,6) TR=(0,6) BL=(2,4) BR=(0,4)  [v-flipped + mirrored]
    switch (d) {
        case 1: setLED(2,6); setLED(1,6); setLED(0,6); break;
        case 2: setLED(2,4); setLED(1,4); setLED(0,4); break;
        case 3: setLED(2,6); setLED(1,5); setLED(0,4); break;
        case 4: setLED(2,4); setLED(1,5); setLED(0,6); break;
        case 5: setLED(2,6); setLED(1,6); setLED(0,6);
                setLED(2,4); setLED(1,5); break;
        case 6: setLED(0,6); setLED(0,5); setLED(0,4); break;
        case 7: setLED(2,6); setLED(1,6); setLED(0,6);
                              setLED(0,5); setLED(0,4); break;
        case 8: setLED(2,4); setLED(1,4); setLED(0,4);
                setLED(0,6); setLED(0,5); break;
        case 9: setLED(2,6); setLED(1,6); setLED(0,6);
                setLED(2,4); setLED(1,4); setLED(0,4);
                              setLED(0,5); break;
    }
}

// ── Border animation ──────────────────────────────────────────────────────────
//
//  15-pixel L-border: pos 0-7 = row 0 cols 0-7, pos 8-14 = col 0 rows 1-7.
//  Five schemes rotate randomly every SCHEME_SECS seconds, 1 step/second.
//
//  At BRIGHTNESS=10, FastLED scales values by 10/255 (~4%).
//  Use large raw values so the dim output is still visibly gradated:
//    255→~10, 150→~6, 80→~3, 30→~1

#define BORDER_LEN  15
#define DOT_MS      1000    // advance one step per second
#define SCHEME_SECS 15      // seconds per scheme
#define NUM_SCHEMES 5

static uint8_t borderPixels[BORDER_LEN];

void borderSetPos(int pos, uint8_t g) {
    int p = ((pos % BORDER_LEN) + BORDER_LEN) % BORDER_LEN;
    if (borderPixels[p] < g) borderPixels[p] = g;  // brightest wins on overlap
}

void advanceBorder(int scheme, int step) {
    memset(borderPixels, 0, sizeof(borderPixels));
    switch (scheme) {

        case 0: {  // Comet: looping with 3-pixel fading tail
            static const uint8_t f[] = {255, 150, 70, 30};
            for (int i = 0; i < 4; i++)
                borderSetPos(step - i, f[i]);
            break;
        }

        case 1: {  // Ping-pong: dot bounces end-to-end with tail
            int cycle = (BORDER_LEN - 1) * 2;  // 28
            int s = step % cycle;
            int pos = s < BORDER_LEN ? s : cycle - s;
            int dir = s < BORDER_LEN ? 1 : -1;
            static const uint8_t f[] = {255, 130, 50};
            for (int i = 0; i < 3; i++)
                borderSetPos(pos - dir * i, f[i]);
            break;
        }

        case 2: {  // Double: two comets chasing, offset by half the border
            static const uint8_t f[] = {255, 120, 45};
            for (int j = 0; j < 2; j++) {
                int head = step + j * 7;
                for (int i = 0; i < 3; i++)
                    borderSetPos(head - i, f[i]);
            }
            break;
        }

        case 3: {  // Fill-drain: pixels accumulate then shrink
            int s = step % (BORDER_LEN * 2);
            if (s < BORDER_LEN) {
                for (int i = 0; i <= s; i++)
                    borderPixels[i] = (i == s) ? 255 : 150;
            } else {
                int n = BORDER_LEN * 2 - s;
                for (int i = 0; i < n; i++)
                    borderPixels[i] = 150;
            }
            break;
        }

        case 4: {  // Wipe: dark notch sweeps across fully-lit border
            for (int i = 0; i < BORDER_LEN; i++) borderPixels[i] = 150;
            int notch = step % BORDER_LEN;
            borderPixels[notch] = 0;
            if (notch > 0)              borderPixels[notch - 1] = 40;
            if (notch < BORDER_LEN - 1) borderPixels[notch + 1] = 40;
            break;
        }
    }
}

void drawBorder() {
    for (int i = 0; i < BORDER_LEN; i++) {
        int row = (i < 8) ? 0 : i - 7;
        int col = (i < 8) ? i : 0;
        leds[ledIndex(row, col)] = CRGB(0, borderPixels[i], 0);
    }
}

// ── Display ───────────────────────────────────────────────────────────────────

int lastHour = -1, lastMinute = -1;

void drawGlyph(int hour, int minute) {
    int n         = hour * 100 + minute;
    int units     = n % 10;
    int tens      = (n / 10) % 10;
    int hundreds  = (n / 100) % 10;
    int thousands = (n / 1000) % 10;

    if (hour != lastHour || minute != lastMinute) {
        lastHour = hour; lastMinute = minute;
        Serial.printf("%02d:%02d  [T=%d H=%d t=%d U=%d]\n",
            hour, minute, thousands, hundreds, tens, units);
    }

    drawStave();
    drawThousands(thousands);
    drawHundreds(hundreds);
    drawTens(tens);
    drawUnits(units);
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Cistercian Clock ===");

    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.printf("\nConnected: %s\n", WiFi.localIP().toString().c_str());

    configTime(gmtOffset, dstOffset, "pool.ntp.org", "time.nist.gov");
    Serial.print("Syncing NTP");
    time_t now = time(nullptr);
    while (now < 1000000000) {
        delay(500); Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("\nTime synchronized!");

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    advanceBorder(0, 0);
    FastLED.show();
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
    static unsigned long lastStepMs   = 0;
    static unsigned long schemeStartMs = 0;
    static int step   = 0;
    static int scheme = 0;

    unsigned long now_ms = millis();

    // Switch scheme randomly every SCHEME_SECS, no immediate repeat
    if (now_ms - schemeStartMs >= (unsigned long)SCHEME_SECS * 1000) {
        int next;
        do { next = random(0, NUM_SCHEMES); } while (next == scheme);
        scheme = next;
        schemeStartMs = now_ms;
        step = 0;
        advanceBorder(scheme, step);
        Serial.printf("Border scheme -> %d\n", scheme);
    }

    // Advance animation one step per second
    if (now_ms - lastStepMs >= DOT_MS) {
        lastStepMs = now_ms;
        step++;
        advanceBorder(scheme, step);
    }

    // Redraw full frame
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);

    FastLED.clear();
    drawGlyph(t->tm_hour, t->tm_min);
    drawBorder();
    FastLED.show();

    delay(20);
}
