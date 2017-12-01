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


String toLog;
/**/


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

void lora_send_gps_data(void);
bool find_fix(uint32_t);
void do_flash_led(int);
void ttn_setup(void);
void ttn_send(void);


void setup() {
  
  /* Init serial */
  delay(3000);
  while (!SerialUSB) {
      // Wait for USB to connect
  }

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

  /* if sending is too fast for TTN uncoment lines */
//  if (millis() - previousMillis > interval){
//    previousMillis = millis();
  
    if (find_fix(0)){
      /* blink led */
      do_flash_led(LED_RED);
      /* Send loara data */
      LoraP2P_Setup();
      lora_send_gps_data(); 

      /* Send data to TTN */
      ttn_setup();
      ttn_send();
//    }
//  }else{
//    /* send no data over Lora */
//    
   }
}


/*
 * Send GPS data over Lora P2P communication
 */
void lora_send_gps_data(void){
//  MySerial.println("Sending Message...");
////  LORA_Write("10");
////  delay(200);

  str_conv_a = String(lat);
  MySerial.println(String("test string: ") + str_conv_a);
  str_conv_a.toCharArray(char_conv_a, 10);
  MySerial.println(char_conv_a);
  //send_char[0] = 'a';
  for (int i = 0; i < 9; i++){
    send_char[i] = char_conv_a[i];
  }

  str_conv_o = String(lon);
  MySerial.println(String("test string: ") + str_conv_o);
  str_conv_o.toCharArray(char_conv_o, 10);
  MySerial.println(char_conv_o);
  //send_char[10] = 'b';
  for (int i = 0; i < 9; i++){
    send_char[i+9] = char_conv_o[i];
  }


  str_conv_t = String(alt);
  MySerial.println(String("test string") + str_conv_t);
  str_conv_t.toCharArray(char_conv_t, 10);
  MySerial.println(char_conv_t);
  //send_char[20] = 'c';
  for (int i = 0; i < 9; i++){
    send_char[i+18] = char_conv_t[i];
  }

  //send_char[30] = '\n';
  MySerial.println(String("send char: ") + send_char);
  
  LORA_Write(send_char);
  MySerial.println("lora sent");
  delay(200);

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
//  SerialUSB.println("RN2xx3 firmware version:");
//  SerialUSB.println(myLora.sysver());
  
  bool join_result = false;

  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("26011FE5", "657FE762E8328542BAF3CBB71A949593", "92DB6FCF4592986E0F26B60CD3117ED2");

//  while(!join_result)
//  {
//    SerialUSB.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
//    delay(60000); //delay a minute before retry
//    digitalWrite(LED_BLUE, LOW);
//    join_result = myLora.init();
//    digitalWrite(LED_BLUE, HIGH);
//  }
//  SerialUSB.println("Successfully joined TTN");
}

void ttn_send(void){
  
  toLog = String(long(sodaq_gps.getLat()*1000000));
  toLog +=" ";
  toLog += String(long(sodaq_gps.getLon()*1000000));
  toLog+=" ";
  toLog+=String(int(sodaq_gps.getAlt()));
  toLog+=" ";
  toLog+=String(int(sodaq_gps.getHDOP()*100));
  
  SerialUSB.println(toLog);
  digitalWrite(LED_BLUE, LOW);
  myLora.tx(toLog);
  digitalWrite(LED_BLUE, HIGH);
  SerialUSB.println("TX done");
}

