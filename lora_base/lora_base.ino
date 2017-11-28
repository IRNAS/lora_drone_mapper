#include <Arduino.h>
#include "LORA_P2P_lib.h"




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
		CONSOLE_STREAM.println("Sending Message...");
		LORA_Write("10");
		delay(1000);
	}
	else {

		char msg[100] = "";
		CONSOLE_STREAM.println("Listening for Message...");
		int errorCode = LORA_Read(msg); // We have a message if returncode = 1

		if (errorCode == 1) { // Switch LED if message = 10
			LED_Flag = !LED_Flag;
			digitalWrite(LED_GREEN, LED_Flag); // Change LED if there is a message
			CONSOLE_STREAM.println(msg);
		}
		else
			CONSOLE_STREAM.print("Error: ");
			CONSOLE_STREAM.println(errorCode);
	}

}
