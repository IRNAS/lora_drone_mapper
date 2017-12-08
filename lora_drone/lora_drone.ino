#include <Arduino.h>
#include <Wire.h>
//#include <Sodaq_UBlox_GPS.h>
#include "LORA_P2P_lib.h"

/**/
#include <rn2xx3.h>
/**/


//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(LORA_STREAM);

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
  //delay(3000);
  // make sure usb serial connection is available,
  // or after 10s go on anyway for 'headless' use of the node.
  while ((!CONSOLE_STREAM) && (millis() < 10000));

  CONSOLE_STREAM.begin(57600);
  //test_function(); while(1);

  /* Init GPS */
  CONSOLE_STREAM.println("LoRaONE test is starting ...");


  /* Init Lora */
  LORA_STREAM.begin(57600);
  //delay(3000);
  //LoraP2P_Setup();
  //CONSOLE_STREAM.println("Lora setup done.");
  
}

void loop() {

  
   
//    LoraP2P_Setup();
//    LORA_Write((char*)txBuffer);
//    CONSOLE_STREAM.println("LORA TX done");

    /* Send data to TTN */
    ttn_setup();
    myLora.txBytes(txBuffer, sizeof(txBuffer));
    CONSOLE_STREAM.println("TTN TX done");

    delay(2000);

}



void ttn_setup(void){
  //print out the HWEUI so that we can register it via ttnctl
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    CONSOLE_STREAM.println("Communication with RN2xx3 unsuccessful. Power cycle the Sodaq One board.");
    delay(10000);
    hweui = myLora.hweui();
  }
  bool join_result = false;
  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("26011AA1", "4B4D878F6DCE8E592169D2AA0D111EC6", "0172714C95D54A931C9C778DF5362DBD");

}


