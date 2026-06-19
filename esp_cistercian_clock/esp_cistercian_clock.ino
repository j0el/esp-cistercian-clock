#include <ESP8266WiFi.h>
#include <time.h>
#include <FastLED.h>

// ── Config ────────────────────────────────────────────────────────────────────

const char* ssid     = "Detonator";
const char* password = "Emerson03";
const long  gmtOffset = -28800;  // UTC-8 Pacific Standard
const int   dstOffset = 3600;

#define LED_PIN    2
#define NUM_LEDS   64
#define BRIGHTNESS 10
#define BUTTON_PIN 0   // GPIO0 — the FLASH button

CRGB leds[NUM_LEDS];
int  mode = 0;  // 0=Clock  1=Life

int ledIndex(int row, int col) { return row * 8 + col; }

// ════════════════════════════════════════════════════════════════════════════════
//  MODE 0: CISTERCIAN CLOCK
// ════════════════════════════════════════════════════════════════════════════════

CRGB fgColor = CRGB(0, 0, 40);

void setLED(int lc, int lr) {
    int col = lc + 1, row = lr + 1;
    if (col < 1 || col > 7 || row < 1 || row > 7) return;
    leds[ledIndex(row, col)] = fgColor;
}

void drawStave() { for (int r = 0; r < 7; r++) setLED(3, r); }

void drawUnits(int d) {
    switch (d) {
        case 1: setLED(4,0); setLED(5,0); setLED(6,0); break;
        case 2: setLED(4,2); setLED(5,2); setLED(6,2); break;
        case 3: setLED(4,0); setLED(5,1); setLED(6,2); break;
        case 4: setLED(4,2); setLED(5,1); setLED(6,0); break;
        case 5: setLED(4,0); setLED(5,0); setLED(6,0); setLED(4,2); setLED(5,1); break;
        case 6: setLED(6,0); setLED(6,1); setLED(6,2); break;
        case 7: setLED(4,0); setLED(5,0); setLED(6,0); setLED(6,1); setLED(6,2); break;
        case 8: setLED(4,2); setLED(5,2); setLED(6,2); setLED(6,0); setLED(6,1); break;
        case 9: setLED(4,0); setLED(5,0); setLED(6,0); setLED(4,2); setLED(5,2);
                setLED(6,2); setLED(6,1); break;
    }
}

void drawTens(int d) {
    switch (d) {
        case 1: setLED(2,0); setLED(1,0); setLED(0,0); break;
        case 2: setLED(2,2); setLED(1,2); setLED(0,2); break;
        case 3: setLED(2,0); setLED(1,1); setLED(0,2); break;
        case 4: setLED(2,2); setLED(1,1); setLED(0,0); break;
        case 5: setLED(2,0); setLED(1,0); setLED(0,0); setLED(2,2); setLED(1,1); break;
        case 6: setLED(0,0); setLED(0,1); setLED(0,2); break;
        case 7: setLED(2,0); setLED(1,0); setLED(0,0); setLED(0,1); setLED(0,2); break;
        case 8: setLED(2,2); setLED(1,2); setLED(0,2); setLED(0,0); setLED(0,1); break;
        case 9: setLED(2,0); setLED(1,0); setLED(0,0); setLED(2,2); setLED(1,2);
                setLED(0,2); setLED(0,1); break;
    }
}

void drawHundreds(int d) {
    switch (d) {
        case 1: setLED(4,6); setLED(5,6); setLED(6,6); break;
        case 2: setLED(4,4); setLED(5,4); setLED(6,4); break;
        case 3: setLED(4,6); setLED(5,5); setLED(6,4); break;
        case 4: setLED(4,4); setLED(5,5); setLED(6,6); break;
        case 5: setLED(4,6); setLED(5,6); setLED(6,6); setLED(4,4); setLED(5,5); break;
        case 6: setLED(6,6); setLED(6,5); setLED(6,4); break;
        case 7: setLED(4,6); setLED(5,6); setLED(6,6); setLED(6,5); setLED(6,4); break;
        case 8: setLED(4,4); setLED(5,4); setLED(6,4); setLED(6,6); setLED(6,5); break;
        case 9: setLED(4,6); setLED(5,6); setLED(6,6); setLED(4,4); setLED(5,4);
                setLED(6,4); setLED(6,5); break;
    }
}

void drawThousands(int d) {
    switch (d) {
        case 1: setLED(2,6); setLED(1,6); setLED(0,6); break;
        case 2: setLED(2,4); setLED(1,4); setLED(0,4); break;
        case 3: setLED(2,6); setLED(1,5); setLED(0,4); break;
        case 4: setLED(2,4); setLED(1,5); setLED(0,6); break;
        case 5: setLED(2,6); setLED(1,6); setLED(0,6); setLED(2,4); setLED(1,5); break;
        case 6: setLED(0,6); setLED(0,5); setLED(0,4); break;
        case 7: setLED(2,6); setLED(1,6); setLED(0,6); setLED(0,5); setLED(0,4); break;
        case 8: setLED(2,4); setLED(1,4); setLED(0,4); setLED(0,6); setLED(0,5); break;
        case 9: setLED(2,6); setLED(1,6); setLED(0,6); setLED(2,4); setLED(1,4);
                setLED(0,4); setLED(0,5); break;
    }
}

// ── Border animation ──────────────────────────────────────────────────────────

#define BORDER_LEN  15
#define DOT_MS      1000
#define SCHEME_SECS 15
#define NUM_SCHEMES 5

static uint8_t borderPixels[BORDER_LEN];

void borderSetPos(int pos, uint8_t g) {
    int p = ((pos % BORDER_LEN) + BORDER_LEN) % BORDER_LEN;
    if (borderPixels[p] < g) borderPixels[p] = g;
}

void advanceBorder(int scheme, int step) {
    memset(borderPixels, 0, sizeof(borderPixels));
    switch (scheme) {
        case 0: {
            static const uint8_t f[] = {255, 150, 70, 30};
            for (int i = 0; i < 4; i++) borderSetPos(step - i, f[i]);
            break;
        }
        case 1: {
            int cycle = (BORDER_LEN - 1) * 2;
            int s = step % cycle;
            int pos = s < BORDER_LEN ? s : cycle - s;
            int dir = s < BORDER_LEN ? 1 : -1;
            static const uint8_t f[] = {255, 130, 50};
            for (int i = 0; i < 3; i++) borderSetPos(pos - dir * i, f[i]);
            break;
        }
        case 2: {
            static const uint8_t f[] = {255, 120, 45};
            for (int j = 0; j < 2; j++) {
                int head = step + j * 7;
                for (int i = 0; i < 3; i++) borderSetPos(head - i, f[i]);
            }
            break;
        }
        case 3: {
            int s = step % (BORDER_LEN * 2);
            if (s < BORDER_LEN) {
                for (int i = 0; i <= s; i++) borderPixels[i] = (i == s) ? 255 : 150;
            } else {
                int n = BORDER_LEN * 2 - s;
                for (int i = 0; i < n; i++) borderPixels[i] = 150;
            }
            break;
        }
        case 4: {
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

void runClock() {
    static unsigned long lastStepMs    = 0;
    static unsigned long schemeStartMs = 0;
    static int step   = 0;
    static int scheme = 0;
    unsigned long now_ms = millis();
    if (now_ms - schemeStartMs >= (unsigned long)SCHEME_SECS * 1000) {
        int next;
        do { next = random(0, NUM_SCHEMES); } while (next == scheme);
        scheme = next;
        schemeStartMs = now_ms;
        step = 0;
        advanceBorder(scheme, step);
        Serial.printf("Border scheme -> %d\n", scheme);
    }
    if (now_ms - lastStepMs >= DOT_MS) {
        lastStepMs = now_ms;
        step++;
        advanceBorder(scheme, step);
    }
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    FastLED.clear();
    drawGlyph(t->tm_hour, t->tm_min);
    drawBorder();
    FastLED.show();
    delay(20);
}

// ════════════════════════════════════════════════════════════════════════════════
//  MODE 1: CONWAY'S GAME OF LIFE  — preset patterns
// ════════════════════════════════════════════════════════════════════════════════
//
//  Patterns rotate randomly every GOL_SWITCH_MS (20 seconds) across four
//  categories.  Grid is 8×8 toroidal so any placement wraps correctly.
//
//  Spaceships  → 4 glider orientations
//  Oscillators → Blinker (p2), Toad (p2), Beacon (p2)
//  Guns        → R-pentomino, Acorn, Die Hard  (Gosper Gun is 36×9; too big)
//  Breeders    → 3 randomly-placed gliders     (true breeders also too big)
//
//  Three-color display: blue=survive  green=born  red=dying

#define GOL_GEN_MS    1000    // ms per generation
#define GOL_SWITCH_MS 20000   // ms before switching to a new pattern

bool golCur[8][8];
bool golNxt[8][8];
unsigned long golLastGenMs      = 0;
unsigned long golPatternStartMs = 0;
int golGenCount = 0;

const CRGB GOL_SURVIVE = CRGB(  0,   0, 200);
const CRGB GOL_BORN    = CRGB(  0, 200,   0);
const CRGB GOL_DYING   = CRGB(200,   0,   0);

// ─── GoL engine ──────────────────────────────────────────────────────────────

int golCountN(int r, int c) {
    int n = 0;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            if (golCur[(r+dr+8)%8][(c+dc+8)%8]) n++;
        }
    return n;
}

void golComputeNext() {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            int n = golCountN(r, c);
            golNxt[r][c] = golCur[r][c] ? (n == 2 || n == 3) : (n == 3);
        }
}

uint64_t golMask(bool b[8][8]) {
    uint64_t m = 0;
    for (int i = 0; i < 64; i++)
        if (b[i/8][i%8]) m |= (1ULL << i);
    return m;
}

// ─── Pattern library ─────────────────────────────────────────────────────────
// Each entry is a flat array of (row,col) pairs, relative to a placement anchor.

// Spaceships — four glider orientations
static const int8_t P_GL_SE[] = {0,1, 1,2, 2,0,2,1,2,2};       // .#. / ..# / ###
static const int8_t P_GL_SW[] = {0,1, 1,0, 2,0,2,1,2,2};       // .#. / #.. / ###
static const int8_t P_GL_NE[] = {0,0,0,1,0,2, 1,2, 2,1};       // ### / ..# / .#.
static const int8_t P_GL_NW[] = {0,0,0,1,0,2, 1,0, 2,1};       // ### / #.. / .#.

// Oscillators
static const int8_t P_BLINKER[] = {0,0, 0,1, 0,2};              // ###  period 2
static const int8_t P_TOAD[]    = {0,1,0,2,0,3, 1,0,1,1,1,2};  // .### / ###.  period 2
static const int8_t P_BEACON[]  = {0,0,0,1, 1,0, 2,3, 3,2,3,3};// ##.. / #... / ...# / ..##  period 2

// "Guns" — long-lived chaotic seeds (true guns don't fit on 8×8)
// R-pentomino: 5 cells, active for 1103 gens on infinite grid
static const int8_t P_RPENTO[]  = {0,1,0,2, 1,0,1,1, 2,1};
// Acorn: 7 cells, ~5000 gens on infinite grid
static const int8_t P_ACORN[]   = {0,1, 1,3, 2,0,2,1,2,4,2,5,2,6};
// Die Hard: 7 cells, vanishes at gen 130 on infinite grid
static const int8_t P_DIEHARD[] = {0,6, 1,0,1,1, 2,1,2,5,2,6,2,7};

struct PatDef { const char* name; const int8_t* cells; uint8_t n; };

static const PatDef SPACESHIPS[]  = {
    {"Glider SE", P_GL_SE, 5}, {"Glider SW", P_GL_SW, 5},
    {"Glider NE", P_GL_NE, 5}, {"Glider NW", P_GL_NW, 5},
};
static const PatDef OSCILLATORS[] = {
    {"Blinker", P_BLINKER, 3}, {"Toad", P_TOAD, 6}, {"Beacon", P_BEACON, 6},
};
static const PatDef GUNS[] = {
    {"R-pentomino", P_RPENTO, 5}, {"Acorn", P_ACORN, 7}, {"Die Hard", P_DIEHARD, 7},
};

// Stamp a pattern onto golCur at (baseR, baseC), wrapping toroidally
void golPlace(const int8_t* cells, int n, int baseR, int baseC) {
    for (int i = 0; i < n; i++) {
        int r = (baseR + cells[i*2]   + 8) % 8;
        int c = (baseC + cells[i*2+1] + 8) % 8;
        golCur[r][c] = true;
    }
}

// Pick and stamp a new starting pattern
void golStartPattern() {
    memset(golCur, 0, sizeof(golCur));
    golGenCount = 0;
    randomSeed(analogRead(0) ^ millis());

    const int8_t* gliders[] = {P_GL_SE, P_GL_SW, P_GL_NE, P_GL_NW};

    int cat = random(4);
    int r = random(8), c = random(8);

    switch (cat) {
        case 0: {  // Spaceship
            int p = random(4);
            golPlace(SPACESHIPS[p].cells, SPACESHIPS[p].n, r, c);
            Serial.printf("Life: Spaceship / %s at (%d,%d)\n", SPACESHIPS[p].name, r, c);
            break;
        }
        case 1: {  // Oscillator
            int p = random(3);
            golPlace(OSCILLATORS[p].cells, OSCILLATORS[p].n, r, c);
            Serial.printf("Life: Oscillator / %s at (%d,%d)\n", OSCILLATORS[p].name, r, c);
            break;
        }
        case 2: {  // "Gun" — long-lived chaotic seed
            int p = random(3);
            golPlace(GUNS[p].cells, GUNS[p].n, r, c);
            Serial.printf("Life: Gun / %s at (%d,%d)\n", GUNS[p].name, r, c);
            break;
        }
        case 3: {  // "Breeder" — 3 gliders that interact on the torus
            for (int g = 0; g < 3; g++)
                golPlace(gliders[random(4)], 5, random(8), random(8));
            Serial.println("Life: Breeder / 3-glider cluster");
            break;
        }
    }

    golComputeNext();
}

void resetGoL() {
    golLastGenMs      = 0;
    golPatternStartMs = millis();
    golStartPattern();
}

void runGoL() {
    if (millis() - golLastGenMs < GOL_GEN_MS) { delay(10); return; }
    golLastGenMs = millis();

    FastLED.clear();
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            CRGB col = CRGB::Black;
            if      (golCur[r][c] && golNxt[r][c]) col = GOL_SURVIVE;
            else if (golCur[r][c])                 col = GOL_DYING;
            else if (golNxt[r][c])                 col = GOL_BORN;
            leds[r * 8 + c] = col;
        }
    FastLED.show();

    bool dead    = (golMask(golNxt) == 0);
    bool timeout = (millis() - golPatternStartMs >= GOL_SWITCH_MS);
    if (dead || timeout) {
        if (dead) { delay(500); Serial.println("Life: extinction"); }
        resetGoL();
        return;
    }

    memcpy(golCur, golNxt, sizeof(golCur));
    golGenCount++;
    golComputeNext();
}

// ════════════════════════════════════════════════════════════════════════════════
//  BUTTON + SETUP + LOOP
// ════════════════════════════════════════════════════════════════════════════════
//
//  Short press (< 1500ms): advance to next mode
//  Long press  (>= 1500ms): advance to next mode

void switchMode() {
    mode = (mode + 1) % 2;
    const char* names[] = {"Clock", "Life"};
    Serial.printf("Mode -> %d (%s)\n", mode, names[mode]);
    if (mode == 1) resetGoL();
    FastLED.clear();
    FastLED.show();
}

void checkButton() {
    static unsigned long pressedAt  = 0;
    static bool wasDown             = false;
    static bool actionTaken         = false;

    bool down = (digitalRead(BUTTON_PIN) == LOW);

    if (down && !wasDown) {
        pressedAt   = millis();
        wasDown     = true;
        actionTaken = false;
    }

    // Trigger long press while still held
    if (down && !actionTaken && millis() - pressedAt >= 1500) {
        actionTaken = true;
        switchMode();
    }

    if (!down && wasDown) {
        unsigned long held = millis() - pressedAt;
        wasDown = false;
        if (!actionTaken && held >= 50) {
            switchMode();
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Cistercian Clock + Life ===");
    Serial.println("Short press: next mode");
    Serial.println("Long press:  next mode");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

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

void loop() {
    checkButton();
    if (mode == 0) runClock();
    else           runGoL();
}
