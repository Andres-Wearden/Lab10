#include <WiFi.h>
#include <esp_now.h>

#define UART2_RX_PIN 16
#define UART2_TX_PIN 17

// pack a 12‑bit value into a uint16_t
typedef struct {
  uint16_t data12;  
} data_pkt_t;

// your peer MAC as before…
uint8_t peerMAC[6] = { 0x78,0x42,0x1C,0x6D,0xA6,0xAC };

void onSent(const uint8_t *mac, esp_now_send_status_t status){
  Serial.printf("ESP‑NOW send %s\n",
    status==ESP_NOW_SEND_SUCCESS?"OK":"FAIL");
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init()!=ESP_OK) while(1){ Serial.println("init fail"); delay(100); }
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMAC, 6);
  peer.channel=1; peer.encrypt=false;
  if (esp_now_add_peer(&peer)!=ESP_OK) while(1){ Serial.println("peer fail"); delay(100); }
}

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);
  initEspNow();
}

void loop(){
  // let's say you read a 12‑bit ADC from your TM4C over UART2…
  if (Serial2.available()>=2) { 
    // read two bytes
    uint8_t lo = Serial2.read();
    uint8_t hi = Serial2.read();
    uint16_t raw = (hi<<8) | lo;      // reconstruct
    raw &= 0x0FFF;                    // mask to lower 12 bits

    data_pkt_t pkt = { raw };         // pack into 16‑bit struct
    esp_now_send(peerMAC,(uint8_t*)&pkt,sizeof(pkt));
  }
}
