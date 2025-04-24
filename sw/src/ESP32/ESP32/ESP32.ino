#include <WiFi.h>
#include <esp_now.h>

// Remap Serial1 to use GPIO15 as RX and GPIO2 as TX
#define UART_RX_PIN 15
#define UART_TX_PIN 2

// Pack a 12-bit value into a uint16_t
typedef struct {
  uint16_t data12;  
} data_pkt_t;

// Your peer’s MAC address
uint8_t peerMAC[6] = { 0x78, 0x42, 0x1C, 0x6D, 0xA6, 0xAC };

void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.printf("ESP-NOW send %s\n",
    status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1) delay(100);
  }
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMAC, 6);
  peer.channel = 1;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (1) delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  // Initialize Serial1 on GPIO15 (RX) and GPIO2 (TX)
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  initEspNow();
  Serial.println("ESP32S2 ready — reading UART1 and forwarding via ESP-NOW");
}

void loop() {
  // Read two bytes from Serial1, reconstruct your 12-bit value
  if (Serial1.available() >= 2) { 
    uint8_t lo = Serial1.read();
    uint8_t hi = Serial1.read();
    uint16_t raw = ((uint16_t)hi << 8) | lo;
    raw &= 0x0FFF;  // mask to 12 bits

    // Print out what we received from the TM4C
    Serial.printf("UART1 → Received 12-bit = 0x%03X (%u)\n", raw, raw);

    // Pack and send via ESP-NOW
    data_pkt_t pkt = { raw };
    esp_now_send(peerMAC, (uint8_t*)&pkt, sizeof(pkt));
  }
}
