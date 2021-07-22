#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include "ArduinoLowPower.h"

#define REG_OCP 0x0B
#define REG_PA_CONFIG 0x09
#define REG_LNA 0x0c
#define REG_OP_MODE 0x01
#define REG_MODEM_CONFIG_1 0x1d
#define REG_MODEM_CONFIG_2 0x1e
#define REG_MODEM_CONFIG_3 0x26
#define REG_PA_DAC 0x4D
#define PA_DAC_HIGH 0x87

#define RFM_TCXO (40u)
#define RFM_SWITCH (41u)

uint16_t count = 0;
uint32_t myFreq = 868.125e6;
int mySF = 10;
uint8_t myBW = 7;
uint8_t myCR = 5;
uint8_t TxPower = 20;
double BWs[10] = {
  7.8, 10.4, 15.6, 20.8, 31.25,
  41.7, 62.5, 125.0, 250.0, 500.0
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1); // Turn on Blue LED
  SerialUSB.begin(115200);
  delay(2000);
  pinMode(RFM_TCXO, OUTPUT);
  pinMode(RFM_SWITCH, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.setPins(SS, RFM_RST, RFM_DIO0);
  if (!LoRa.begin(myFreq)) {
    SerialUSB.println("Starting LoRa failed!\nNow that's disappointing...");
    while (1);
  }
  LoRa.setSpreadingFactor(mySF);
  LoRa.setSignalBandwidth(BWs[myBW] * 1e3);
  LoRa.setCodingRate4(myCR);
  LoRa.setPreambleLength(8);
  LoRa.setTxPower(TxPower, PA_OUTPUT_PA_BOOST_PIN);
  digitalWrite(RFM_SWITCH, HIGH);
  LoRa.setTxPower(TxPower, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.writeRegister(REG_PA_CONFIG, 0b11111111); // That's for the transceiver
  LoRa.writeRegister(REG_PA_DAC, PA_DAC_HIGH); // That's for the transceiver
  LoRa.writeRegister(REG_OCP, 0b00011111); // NO OCP
  LoRa.receive();
  LoRa.writeRegister(REG_LNA, 0x23); // TURN ON LNA FOR RECEIVE
  digitalWrite(LED_BUILTIN, 0);
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  LoRa.idle();
  LoRa.writeRegister(REG_LNA, 00); // TURN OFF LNA FOR TRANSMIT
  digitalWrite(RFM_SWITCH, LOW);
  char buf[64];
  sprintf(buf, "This is packet #%d\n", count);
  // digitalWrite(PIN_PA28, LOW);
  digitalWrite(RFM_SWITCH, 0);
  LoRa.beginPacket();
  LoRa.print(buf);
  LoRa.endPacket();
  digitalWrite(RFM_SWITCH, HIGH);
  LoRa.receive();
  LoRa.writeRegister(REG_LNA, 0x23); // TURN ON LNA FOR RECEIVE
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  // Triggers a 30-second sleep (the device will be woken up only by the registered wakeup sources and by internal RTC)
  // The power consumption of the chip will drop consistently
  LowPower.sleep(30000);
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
  count++;
}
