# ESP8266 Cistercian Clock

An ESP8266-based clock that displays the current time using [Cistercian numerals](https://en.wikipedia.org/wiki/Cistercian_numerals) on an 8×8 WS2812B LED matrix. Time is fetched via WiFi and NTP — no RTC required.

The Cistercian numeral system encodes an entire time (HHMM, up to 9999) in a single glyph using four quadrants around a central vertical stave.

## Hardware

| Component | Detail |
|---|---|
| Microcontroller | ESP8266MOD (ESP-12E / NodeMCU) |
| LED matrix | 8×8 WS2812B (64 LEDs), standard raster wiring |
| Data pin | D4 (GPIO2) |
| Power | USB (5V) — keep brightness low; use external 5V supply for full brightness |

### Wiring

```
ESP8266 D4 (GPIO2) → WS2812B DIN
ESP8266 GND        → WS2812B GND
5V supply          → WS2812B 5V
```

> **Note:** If powering the LED strip from an external 5V supply, share GND with the ESP8266 but do not connect the external 5V to the ESP8266's VIN while USB is also connected.

## Matrix Layout

The Cistercian glyph occupies the bottom-right **7×7** pixels (rows 1–7, cols 1–7). The top row and left column form an L-shaped border animated with a travelling green dot.

```
col:  0   1  2  3 | 4 | 5  6  7
      ──────────────────────────
row 0  ←── green dot border ──→
row 1  .  TENS  |stave| UNITS  .
row 2  .        |     |        .  upper half
row 3  .        |     |        .
           (gap — stave only)
row 4  .  THOU. |     | HUND.  .
row 5  .        |     |        .  lower half
row 6  .        |     |        .
row 7  .        |     |        .
      (col 0 also part of green dot border)
```

### Cistercian Quadrant Mapping

| Quadrant | Digits | Position |
|---|---|---|
| Upper-right | minute ones (units) | rows 1–3, cols 5–7 |
| Upper-left  | minute tens         | rows 1–3, cols 1–3 |
| Lower-right | hour ones (hundreds)| rows 5–7, cols 5–7 |
| Lower-left  | hour tens (thousands)| rows 5–7, cols 1–3 |

Stroke encoding from [`cistercian-clock`](https://github.com/j0el/cistercian-clock) (Android widget companion project):

```
1: TL→TR   top edge
2: BL→BR   bottom edge
3: TL→BR   diagonal
4: BL→TR   anti-diagonal
5: 1 + 4
6: TR→BR   outer edge
7: 1 + 6
8: 2 + 6
9: 1 + 2 + 6
0: (blank)
```

## Sketches

| Folder | Purpose |
|---|---|
| `esp_cistercian_clock/` | **Main sketch** — WiFi NTP clock on 8×8 WS2812B |
| `esp_time/` | Basic NTP time fetch, prints to Serial |
| `esp_ws2812b_test/` | WS2812B color cycle and chase test |
| `esp_matrix_test/` | Matrix wiring diagnostic (index order, rows, columns, corners) |
| `esp_wifi_debug/` | WiFi connection diagnostic with scan and status codes |

## Configuration

Edit the top of `esp_cistercian_clock/esp_cistercian_clock.ino`:

```cpp
const char* ssid     = "YourNetwork";
const char* password = "YourPassword";

const long gmtOffset = -28800;  // UTC offset in seconds (Pacific = -28800)
const int  dstOffset = 3600;    // DST offset (3600 = 1 hour, 0 = no DST)

#define BRIGHTNESS 10           // 0-255; keep ≤10 on USB power for 64 LEDs
```

## Dependencies

- [ESP8266 Arduino core](https://arduino.esp8266.com/stable/package_esp8266com_index.json) 3.1.2+
- [FastLED](https://github.com/FastLED/FastLED) 3.10+

Install via Arduino IDE: `Tools → Manage Libraries → FastLED`

## Building with arduino-cli

```bash
# Install board support
arduino-cli config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli lib install FastLED

# Compile
arduino-cli compile --fqbn esp8266:esp8266:generic esp_cistercian_clock/

# Upload (adjust port as needed)
arduino-cli upload --fqbn esp8266:esp8266:generic --port /dev/cu.usbserial-1140 esp_cistercian_clock/
```

## Related

- [cistercian-clock](https://github.com/j0el/cistercian-clock) — Android home-screen widget using the same Cistercian stroke table
