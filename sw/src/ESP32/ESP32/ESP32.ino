#include <WiFi.h>
#include <esp_now.h>

// Remap Serial2 to use GPIO15 as RX and GPIO2 as TX
#define UART2_RX_PIN 15
#define UART2_TX_PIN 2

// how many samples to average
#define AVG_SAMPLES 16

// Pack three 12-bit values into a struct
typedef struct {
  uint16_t v1;
  uint16_t v2;
  uint16_t v3;
} data_pkt_t;

// Your peer’s MAC address
uint8_t peerMAC[6] = { 0x78,0x42,0x1C,0x6D,0xA6,0xAC };

void onSent(const uint8_t *mac, esp_now_send_status_t status){
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

void setup(){
  Serial.begin(115200);
  // Initialize Serial2 on GPIO15 (RX) and GPIO2 (TX)
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);
  initEspNow();
  Serial.println("ESP32S2 ready — averaging 3×12-bit values over UART2");
}

void loop(){
  static uint32_t sum1 = 0, sum2 = 0, sum3 = 0;
  static uint8_t count = 0;

  // accumulate up to AVG_SAMPLES packets
  while (Serial2.available() >= 6 && count < AVG_SAMPLES) {
    // read in order: hi1, lo1, hi2, lo2, hi3, lo3
    uint8_t hi1 = Serial2.read();
    uint8_t lo1 = Serial2.read();
    uint8_t hi2 = Serial2.read();
    uint8_t lo2 = Serial2.read();
    uint8_t hi3 = Serial2.read();
    uint8_t lo3 = Serial2.read();

    // reconstruct and mask to 12 bits
    uint16_t v1 = (((uint16_t)hi1 << 8) | lo1) & 0x0FFF;
    uint16_t v2 = (((uint16_t)hi2 << 8) | lo2) & 0x0FFF;
    uint16_t v3 = (((uint16_t)hi3 << 8) | lo3) & 0x0FFF;

    sum1 += v1;
    sum2 += v2;
    sum3 += v3;
    count++;
  }

  if (count >= AVG_SAMPLES) {
    // compute the averages
    uint16_t avg1 = sum1 / AVG_SAMPLES;
    uint16_t avg2 = sum2 / AVG_SAMPLES;
    uint16_t avg3 = sum3 / AVG_SAMPLES;

    // print the averaged values
    Serial.printf("Avg → ADC1=%u  ADC2=%u  ADC3=%u\n", avg1, avg2, avg3);

    // pack and send them via ESP-NOW
    data_pkt_t pkt = { avg1, avg2, avg3 };
    esp_now_send(peerMAC, (uint8_t*)&pkt, sizeof(pkt));

    // reset for next block of samples
    sum1 = sum2 = sum3 = 0;
    count = 0;
  }

  // slight delay to yield to WiFi/ESP-NOW tasks
  delay(10);
}
