#include <ESP8266WiFi.h>

const char* ssid     = "Detonator";
const char* password = "Emerson03";

// ── WiFi reliability settings (from ESP8266 community notes) ─────────────────
// Force STA mode — omitting this causes unstable connections on boot
// Disable power saving — prevents dropped connections during idle
// Enable auto-reconnect — recovers from router drops
// persistent(true) — saves credentials to flash for faster reconnect

void printStatus(int s) {
  switch (s) {
    case WL_IDLE_STATUS:     Serial.println("IDLE");          break;
    case WL_NO_SSID_AVAIL:   Serial.println("NO_SSID_AVAIL"); break;
    case WL_SCAN_COMPLETED:  Serial.println("SCAN_COMPLETED");break;
    case WL_CONNECTED:       Serial.println("CONNECTED");     break;
    case WL_CONNECT_FAILED:  Serial.println("CONNECT_FAILED");break;
    case WL_CONNECTION_LOST: Serial.println("CONNECT_LOST");  break;
    case WL_DISCONNECTED:    Serial.println("DISCONNECTED");  break;
    default:                 Serial.println(s);               break;
  }
}

void scanNetworks() {
  Serial.println("\n--- WiFi Scan ---");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    for (int i = 0; i < n; i++) {
      Serial.printf("  [%d] SSID: \"%s\"  RSSI: %d dBm  Channel: %d  Auth: %d\n",
        i + 1,
        WiFi.SSID(i).c_str(),
        WiFi.RSSI(i),
        WiFi.channel(i),
        (int)WiFi.encryptionType(i));
      if (WiFi.SSID(i) == ssid) {
        Serial.println("       ^^^ TARGET FOUND ^^^");
      }
    }
  }
  Serial.println("-----------------\n");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== ESP8266 WiFi Debug ===");
  Serial.printf("SDK:       %s\n", ESP.getSdkVersion());
  Serial.printf("Chip ID:   %08X\n", ESP.getChipId());
  Serial.printf("MAC:       %s\n", WiFi.macAddress().c_str());
  Serial.printf("Target SSID: \"%s\"\n", ssid);

  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect();
  delay(100);

  scanNetworks();

  Serial.printf("Connecting to \"%s\" ...\n", ssid);
  WiFi.begin(ssid, password);

  for (int i = 0; i < 40; i++) {
    delay(500);
    int st = WiFi.status();
    Serial.printf("  [%2ds] status: ", (i + 1) / 2);
    printStatus(st);
    if (st == WL_CONNECTED) break;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("SUCCESS!");
    Serial.printf("  IP:      %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("  RSSI:    %d dBm\n", WiFi.RSSI());
    Serial.printf("  Channel: %d\n", WiFi.channel());
  } else {
    Serial.println("FAILED after 20 seconds.");
    Serial.printf("  Final status: ");
    printStatus(WiFi.status());
  }
}

void loop() {}
