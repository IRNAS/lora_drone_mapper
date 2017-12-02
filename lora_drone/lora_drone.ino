#include <Arduino.h>
#include <Wire.h>
#include <Sodaq_UBlox_GPS.h>
#include "LORA_P2P_lib.h"

/**/
#include <rn2xx3.h>
/**/
#define MySerial        SERIAL_PORT_MONITOR
/**/

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(Serial1);

long lat = 0;
long lon = 0;
long alt = 0;

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 200;           // interval at which to blink (milliseconds)

char send_char[31];

char char_conv_a[10];
char char_conv_o[10];
char char_conv_t[9];
String str_conv_a;
String str_conv_o;
String str_conv_t;

String toLog;
uint8_t txBuffer[9];
uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;
int dr = 0;

void lora_send_gps_data(void);
bool find_fix(uint32_t);
void do_flash_led(int);
void ttn_setup(void);
void ttn_send(void);




void setup() {
  
  /* Init serial */
  delay(3000);
  // make sure usb serial connection is available,
  // or after 10s go on anyway for 'headless' use of the node.
  while ((!SerialUSB) && (millis() < 10000));

  MySerial.begin(57600);
  //test_function(); while(1);

  /* Init LED */
  digitalWrite(LED_RED, HIGH);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  pinMode(LED_BLUE, OUTPUT);

  do_flash_led(LED_RED);
  do_flash_led(LED_GREEN);
  do_flash_led(LED_BLUE);

  /* Init GPS */
  MySerial.println("SODAQ LoRaONE test_gps is starting ...");

  sodaq_gps.init(GPS_ENABLE);

  // First time finding a fix
  find_fix(0);

  /* Init Lora */
  LORA_STREAM.begin(57600);
  delay(3000);
  LoraP2P_Setup();
  MySerial.println("Lora setup done.");
  
}

void loop() {
  do_flash_led(LED_RED);

  /* if sending is too fast for TTN uncoment lines */
//  if (millis() - previousMillis > interval){
//    previousMillis = millis();
  
    if (find_fix(0)){
      /* blink led */
      do_flash_led(LED_RED);
      /* Send loara data */

      LatitudeBinary = ((sodaq_gps.getLat() + 90) / 180) * 16777215;
      LongitudeBinary = ((sodaq_gps.getLon() + 180) / 360) * 16777215;
    
      txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
      txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
      txBuffer[2] = LatitudeBinary & 0xFF;
    
      txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
      txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
      txBuffer[5] = LongitudeBinary & 0xFF;
    
      altitudeGps = sodaq_gps.getAlt();
      txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
      txBuffer[7] = altitudeGps & 0xFF;
    
      hdopGps = sodaq_gps.getHDOP()*10;
      txBuffer[8] = hdopGps & 0xFF;
    
      toLog = "";
      for(size_t i = 0; i<sizeof(txBuffer); i++)
      {
        char buffer[3];
        sprintf(buffer, "%02x", txBuffer[i]);
        toLog = toLog + String(buffer);
      }
    
      SerialUSB.print("Transmit on DR");
      SerialUSB.print(dr);
      SerialUSB.print(" coordinates ");
      SerialUSB.print(sodaq_gps.getLat(), 13);
      SerialUSB.print(" ");
      SerialUSB.print(sodaq_gps.getLon(), 13);
      SerialUSB.print(" altitude ");
      SerialUSB.print(sodaq_gps.getAlt(), 1);
      SerialUSB.print(" and HDOP ");
      SerialUSB.print(sodaq_gps.getHDOP(), 2);
      SerialUSB.print(" hex ");
      SerialUSB.println(toLog);
    
      digitalWrite(LED_BLUE, LOW);
      myLora.txBytes(txBuffer, sizeof(txBuffer));
      digitalWrite(LED_BLUE, HIGH);
    
      SerialUSB.println("TX done");

      LoraP2P_Setup();
      LORA_Write((char*)txBuffer);
      SerialUSB.println("LORA TX done");

      /* Send data to TTN */
      ttn_setup();
      myLora.txBytes(txBuffer, sizeof(txBuffer));
      SerialUSB.println("TTN TX done");
   }
}

/*!
 * Find a GPS fix, but first wait a while
 */
bool find_fix(uint32_t delay_until){
    //MySerial.println(String("delay ... ") + delay_until + String("ms"));
    //delay(delay_until);

    uint32_t start = millis();
    //uint32_t timeout = 50000L * 1;
    //MySerial.println(String("waiting for fix ..., timeout=") + timeout + String("ms"));
    if (sodaq_gps.scan(true)) {
        MySerial.println(String(" time to find fix: ") + (millis() - start) + String("ms"));
        MySerial.println(String(" datetime = ") + sodaq_gps.getDateTimeString());
        MySerial.println(String(" lat = ") + String(sodaq_gps.getLat(), 7));
        lat = long(sodaq_gps.getLat() * 1000000);
        MySerial.println(String(" lon = ") + String(sodaq_gps.getLon(), 7));
        lon = long(sodaq_gps.getLon() * 1000000);
        MySerial.println(String(" alt = ") + String(sodaq_gps.getAlt(), 7));
        alt = long(sodaq_gps.getAlt() * 1000000);
        MySerial.println(String(" num sats = ") + String(sodaq_gps.getNumberOfSatellites()));
        return true;
    } else {
        MySerial.println("No Fix");
        return false;
    }
}


void do_flash_led(int pin){
  for (size_t i = 0; i < 2; ++i) {
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
    digitalWrite(pin, HIGH);
  }
}

void ttn_setup(void){
  //print out the HWEUI so that we can register it via ttnctl
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    SerialUSB.println("Communication with RN2xx3 unsuccessful. Power cycle the Sodaq One board.");
    delay(10000);
    hweui = myLora.hweui();
  }
  bool join_result = false;
  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("26011AA1", "4B4D878F6DCE8E592169D2AA0D111EC6", "0172714C95D54A931C9C778DF5362DBD");

}


