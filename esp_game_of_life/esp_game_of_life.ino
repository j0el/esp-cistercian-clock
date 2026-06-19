#include <FastLED.h>

#define LED_PIN     2
#define NUM_LEDS    64
#define BRIGHTNESS  10
#define GEN_MS      1000    // ms between generations
#define HISTORY_LEN 16      // detect oscillators up to period 16
#define MAX_GENS    500     // safety reset after N generations
#define FILL_PCT    35      // % of cells alive on random start

CRGB leds[NUM_LEDS];

bool cur[8][8];   // current generation
bool nxt[8][8];   // next generation (pre-computed each step)

// Three-color display: see current state + fate of each cell simultaneously.
// Values near 200 remain visually distinct after BRIGHTNESS=10 scaling.
const CRGB COL_SURVIVE = CRGB(  0,   0, 200);  // alive, will survive  → blue
const CRGB COL_BORN    = CRGB(  0, 200,   0);  // dead,  will be born  → green
const CRGB COL_DYING   = CRGB(200,   0,   0);  // alive, will die      → red

// ── Game of Life ──────────────────────────────────────────────────────────────

int countNeighbors(int r, int c) {
    int n = 0;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            if (cur[(r + dr + 8) % 8][(c + dc + 8) % 8]) n++;
        }
    return n;
}

void computeNext() {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            int n = countNeighbors(r, c);
            nxt[r][c] = cur[r][c] ? (n == 2 || n == 3) : (n == 3);
        }
}

// Pack the board into a 64-bit mask for fast equality comparison.
uint64_t boardMask(bool b[8][8]) {
    uint64_t m = 0;
    for (int i = 0; i < 64; i++)
        if (b[i / 8][i % 8]) m |= (1ULL << i);
    return m;
}

void randomize() {
    randomSeed(analogRead(0));
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            cur[r][c] = random(100) < FILL_PCT;
}

// ── Oscillation detection ─────────────────────────────────────────────────────
//
//  Stores the last HISTORY_LEN board masks in a ring buffer.
//  Before advancing, check whether the *next* state matches any history entry.
//  A match means we've entered a loop (period ≤ HISTORY_LEN).

uint64_t history[HISTORY_LEN];
int histIdx   = 0;
int histCount = 0;
int genCount  = 0;

bool isStuck(uint64_t mask) {
    if (mask == 0) return true;  // all cells dead
    int n = (histCount < HISTORY_LEN) ? histCount : HISTORY_LEN;
    for (int i = 0; i < n; i++)
        if (history[i] == mask) return true;
    return false;
}

void pushHistory(uint64_t mask) {
    history[histIdx] = mask;
    histIdx = (histIdx + 1) % HISTORY_LEN;
    histCount++;
}

void resetBoard() {
    Serial.printf("Reset after %d generations\n", genCount);
    memset(history, 0, sizeof(history));
    histIdx   = 0;
    histCount = 0;
    genCount  = 0;
    randomize();
    computeNext();
}

// ── Display ───────────────────────────────────────────────────────────────────

void display() {
    FastLED.clear();
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            CRGB col = CRGB::Black;
            if      (cur[r][c] && nxt[r][c])  col = COL_SURVIVE;
            else if (cur[r][c])               col = COL_DYING;
            else if (nxt[r][c])               col = COL_BORN;
            leds[r * 8 + c] = col;
        }
    }
    FastLED.show();
}

// ── Setup / Loop ──────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Conway's Game of Life ===");
    Serial.println("Blue=survive  Green=born  Red=dying");

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    randomize();
    computeNext();
}

void loop() {
    static unsigned long lastGenMs = 0;
    if (millis() - lastGenMs < GEN_MS) { delay(10); return; }
    lastGenMs = millis();

    display();  // show current state colored by each cell's fate

    uint64_t nxtMask = boardMask(nxt);
    if (isStuck(nxtMask) || genCount >= MAX_GENS) {
        delay(800);       // pause on final frame so the reset is visible
        resetBoard();
    } else {
        pushHistory(boardMask(cur));
        memcpy(cur, nxt, sizeof(cur));
        genCount++;
        computeNext();    // pre-compute so next display() has both cur and nxt
    }
}
