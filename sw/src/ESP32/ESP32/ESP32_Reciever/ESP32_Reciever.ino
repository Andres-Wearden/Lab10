#include <WiFi.h>
#include <esp_now.h>

// UART2 on ESP32: RX=16, TX=17
#define UART2_RX_PIN 16
#define UART2_TX_PIN 17

// 1) Packet struct for 12‑bit data
typedef struct {
  uint16_t data12;
} data_pkt_t;

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(data_pkt_t)) return;

  // unpack
  data_pkt_t pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));
  uint16_t val12 = pkt.data12 & 0x0FFF;

  // print to USB‑Serial
  char macStr[18];
  Serial.printf("ESP‑NOW → Got 12‑bit=0x%03X (%u) from %s\n", val12, val12, macStr);

  // forward as two raw bytes over Serial2 (PD7)
  Serial2.write((uint8_t)(val12 >> 8));   // high byte
  Serial2.write((uint8_t)(val12 & 0xFF)); // low  byte
}

void initEspNow(){
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP‑NOW init failed");
    while (1) delay(100);
  }
  esp_now_register_recv_cb(onDataRecv);
}

void setup() {
  Serial.begin(115200);
  // bring up Serial2 for forwarding & reception
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);

  initEspNow();
  Serial.println("Receiver ready—waiting for 12‑bit packets");
}

void loop() {
  // 1) Handle any UART2 data coming back on PD7/16
  while (Serial2.available() >= 2) {
    uint8_t hi = Serial2.read();
    uint8_t lo = Serial2.read();
    uint16_t val = ((uint16_t)hi << 8) | lo;
    val &= 0x0FFF;
    Serial.printf("UART2 → Got raw 12‑bit = 0x%03X (%u)\n", val, val);
  }

  // 2) Let ESP‑NOW callbacks run
  delay(10);
}
