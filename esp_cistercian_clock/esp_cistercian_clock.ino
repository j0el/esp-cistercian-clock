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
int  mode = 0;  // 0 = Cistercian clock, 1 = Game of Life, 2 = Snake

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
//  MODE 1: CONWAY'S GAME OF LIFE
// ════════════════════════════════════════════════════════════════════════════════

#define GOL_GEN_MS   1000
#define GOL_HIST_LEN 16
#define GOL_MAX_GENS 500
#define GOL_FILL_PCT 35

bool golCur[8][8];
bool golNxt[8][8];
uint64_t golHistory[GOL_HIST_LEN];
int golHistIdx   = 0;
int golHistCount = 0;
int golGenCount  = 0;
unsigned long golLastGenMs = 0;

const CRGB GOL_SURVIVE = CRGB(  0,   0, 200);
const CRGB GOL_BORN    = CRGB(  0, 200,   0);
const CRGB GOL_DYING   = CRGB(200,   0,   0);

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

bool golIsStuck(uint64_t mask) {
    if (mask == 0) return true;
    int n = (golHistCount < GOL_HIST_LEN) ? golHistCount : GOL_HIST_LEN;
    for (int i = 0; i < n; i++)
        if (golHistory[i] == mask) return true;
    return false;
}

void golPushHistory(uint64_t mask) {
    golHistory[golHistIdx] = mask;
    golHistIdx = (golHistIdx + 1) % GOL_HIST_LEN;
    golHistCount++;
}

void resetGoL() {
    memset(golHistory, 0, sizeof(golHistory));
    golHistIdx   = 0;
    golHistCount = 0;
    golGenCount  = 0;
    golLastGenMs = 0;
    randomSeed(analogRead(0));
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            golCur[r][c] = random(100) < GOL_FILL_PCT;
    golComputeNext();
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
    uint64_t nxtMask = golMask(golNxt);
    if (golIsStuck(nxtMask) || golGenCount >= GOL_MAX_GENS) {
        delay(800);
        Serial.printf("GoL reset after %d generations\n", golGenCount);
        resetGoL();
    } else {
        golPushHistory(golMask(golCur));
        memcpy(golCur, golNxt, sizeof(golCur));
        golGenCount++;
        golComputeNext();
    }
}

// ════════════════════════════════════════════════════════════════════════════════
//  MODE 2: SNAKE
// ════════════════════════════════════════════════════════════════════════════════
//
//  One-button control: short press = turn right (clockwise).
//  Long press (>= 1500ms) always advances to the next mode.
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
int     snkDir     = 0;   // current direction
int     snkNextDir = 0;   // queued turn (applied at next step)
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

    // Apply queued direction — disallow 180° reversal
    if (snkNextDir != (snkDir + 2) % 4) snkDir = snkNextDir;

    int nr = (snkRow[0] + SNK_DR[snkDir] + 8) % 8;
    int nc = (snkCol[0] + SNK_DC[snkDir] + 8) % 8;

    // Death: hit own body (tail excluded — it moves away this step)
    if (snkOnBody(nr, nc, snkLen - 1)) {
        Serial.printf("Snake died! Score: %d\n", snkScore);
        snkFlash(CRGB(150, 0, 0), 3, 200, 150);
        resetSnake();
        return;
    }

    bool ate = (nr == snkFoodR && nc == snkFoodC);
    if (ate && snkLen < SNAKE_MAX_LEN) snkLen++;   // grow before shift

    // Shift body toward tail, insert new head
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

// ════════════════════════════════════════════════════════════════════════════════
//  BUTTON + SETUP + LOOP
// ════════════════════════════════════════════════════════════════════════════════
//
//  Short press (< 1500ms):
//    Snake mode → turn right (clockwise)
//    Other modes → advance to next mode
//
//  Long press (>= 1500ms held then released):
//    Any mode → advance to next mode

void switchMode() {
    mode = (mode + 1) % 3;
    Serial.printf("Mode -> %d (%s)\n", mode,
        mode == 0 ? "Clock" : mode == 1 ? "Life" : "Snake");
    if (mode == 1) resetGoL();
    if (mode == 2) resetSnake();
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
            if (mode == 2) {
                snkNextDir = (snkDir + 1) % 4;   // turn right
            } else {
                switchMode();
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Cistercian Clock + Life + Snake ===");
    Serial.println("Short press: next mode (or turn snake)");
    Serial.println("Long press:  always next mode");

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
    if      (mode == 0) runClock();
    else if (mode == 1) runGoL();
    else                runSnake();
}
