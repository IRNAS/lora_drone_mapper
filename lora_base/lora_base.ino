#include <Arduino.h>
#include "LORA_P2P_lib.h"

float lat, lon, alti, speed;                                                                  // latitude, longitude, altitude and speed
int rssi; 
int snr;


bool LED_Flag = 1;
volatile bool send_Flag = 0;

void setup()
{
	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(BUTTON, INPUT);

	LORA_STREAM.begin(57600);
	CONSOLE_STREAM.begin(57600);
	delay(3000);

	CONSOLE_STREAM.println("Started!");

	LoraP2P_Setup();

	CONSOLE_STREAM.println("LORA SETUP DONE!");

	attachInterrupt(BUTTON, BTN_ISR, FALLING);

}

void BTN_ISR() {
	send_Flag = 1;
}


void loop()
{

	if (send_Flag == 1) {
		send_Flag = 0;
		//CONSOLE_STREAM.println("Sending Message...");
		LORA_Write("10");
		delay(1000);
	}
	else {

		char msg[100] = "";
		//CONSOLE_STREAM.println("Listening for Message...");
		int errorCode = LORA_Read(msg); // We have a message if returncode = 1

		if (errorCode == 1) { // Switch LED if message = 10
			LED_Flag = !LED_Flag;
			digitalWrite(LED_GREEN, LED_Flag); // Change LED if there is a message
      /* decode msg */
			//CONSOLE_STREAM.println(msg);
      decode_and_send_msg(msg);

		}
	}

}

void decode_and_send_msg(char *data){
  
  char lat_char[10];
  char lon_char[10];
  char alti_char[10];

  for (int i = 0; i < 9; i++)           {
    lat_char[i]   = data[i];  // write it into the local arrays
  }
  for (int i = 0; i < 9; i++)          {
    lon_char[i]  = data[i+9];  // write it into the local arrays
  }
  for (int i = 0; i < 9; i++)          {
    alti_char[i] = data[i+18];  // write into the local arrays
  }

  lat = (float)atof(lat_char);   
  lon = (float)atof(lon_char);
  alti = (float)atof(alti_char);                             // convert it to float
  lat = lat / 1000000; lon = lon / 1000000; alti = alti / 100000; //speed = speed / 10000;           // divide it

  //snr = getSNR();

 // debug printing with 10 decimal points
  CONSOLE_STREAM.print(lat,  10);
  CONSOLE_STREAM.print(',');
  CONSOLE_STREAM.print(lon,  10);
  CONSOLE_STREAM.print(',');
  CONSOLE_STREAM.print(alti, 10);
  CONSOLE_STREAM.print(',');
  CONSOLE_STREAM.print(0, 10);//speed
  CONSOLE_STREAM.print(',');
  CONSOLE_STREAM.print(0, 10);//rssi
  CONSOLE_STREAM.print(',');
  CONSOLE_STREAM.print(snr, 10);
  CONSOLE_STREAM.println();
  
}

