#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid     = "Detonator";
const char* password = "Emerson03";

// NTP servers and UTC offset
const char* ntpServer1  = "pool.ntp.org";
const char* ntpServer2  = "time.nist.gov";
const long  gmtOffset   = -18000;  // UTC-5 (EST); adjust as needed
const int   dstOffset   = 3600;    // 1 hour DST

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset, dstOffset, ntpServer1, ntpServer2);
  Serial.println("Waiting for NTP time sync...");

  time_t now = time(nullptr);
  while (now < 1000000000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\nTime synchronized!");
}

void loop() {
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);

  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeInfo);
  Serial.println(buf);

  delay(1000);
}
