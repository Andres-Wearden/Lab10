#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>

// UART2 on ESP32: RX=GPIO16, TX=GPIO17
#define UART2_RX_PIN 16
#define UART2_TX_PIN 17

// Now only one 12-bit value per packet:
typedef struct {
  uint16_t v1;
} data_pkt_t;

// Replace with your transmitter’s MAC
uint8_t peerMAC[6] = { 0x78, 0x42, 0x1C, 0x6D, 0xA6, 0xAC };

void onDataRecv(const esp_now_recv_info_t* info,
                const uint8_t* incomingData, int len) {
  // require exactly 2 bytes
  if (len != sizeof(data_pkt_t)) return;

  // unpack
  data_pkt_t pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  // mask to 12 bits
  uint16_t a1 = pkt.v1 & 0x0FFF;

  // print over USB-Serial
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1],
           info->src_addr[2], info->src_addr[3],
           info->src_addr[4], info->src_addr[5]);
  Serial.printf("From %s → ADC1=%u\n", macStr, a1);

  // forward the single 16-bit word (little-endian) over UART2
  Serial2.write((uint8_t*)&a1, sizeof(a1));
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1) delay(100);
  }
  esp_now_register_recv_cb(onDataRecv);

  // (optional) add peer for bi-directional comms
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMAC, 6);
  peer.channel = 1;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void setup() {
  Serial.begin(115200);
  // bring up UART2 on GPIO16=RX, GPIO17=TX
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);

  initEspNow();
  Serial.println("ESP32 ready — forwarding single 12-bit ADC via UART2");
}

void loop() {
  // nothing here; onDataRecv does the work
  delay(100);
}
