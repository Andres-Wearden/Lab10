#include <WiFi.h>
#include <esp_now.h>

// 1) Change the packet to a 16‑bit container for your 12‑bit data
typedef struct {
  uint16_t data12;
} data_pkt_t;

void onDataRecv(
  const esp_now_recv_info_t *info,
  const uint8_t *incomingData,
  int len
) {
  // 2) Check against the new size
  if (len != sizeof(data_pkt_t)) return;

  data_pkt_t pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  // 3) Mask off the top 4 bits so you only get your original 12‑bit value
  uint16_t val12 = pkt.data12 & 0x0FFF;

  // Print the sender’s MAC
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1],
           info->src_addr[2], info->src_addr[3],
           info->src_addr[4], info->src_addr[5]);

  // And finally print your 12‑bit value (in hex and decimal)
  Serial.printf("Got 12‑bit=0x%03X (%u) from %s\n",
                val12, val12, macStr);
}

void initEspNow(){
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP‑NOW init failed");
    while (true) { delay(100); }
  }
}

void setup() {
  Serial.begin(115200);
  initEspNow();

  // Register the receive callback
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Receiver ready—waiting for 12‑bit packets");
}

void loop() {
  // Nothing here; work happens in onDataRecv()
  delay(100);
}
