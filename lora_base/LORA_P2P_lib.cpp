// 
// 
// 

#include "LORA_P2P_lib.h"


int8_t trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf9";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 2000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable)

const char CR = '\r';
const char LF = '\n';

#define DEBUG


// Configuring the RN2483 for P2P
void LoraP2P_Setup()
{
	LORA_STREAM.print("sys reset\r\n");
	delay(200);
	LORA_STREAM.print("radio set pwr ");
	LORA_STREAM.print(trPower);
	LORA_STREAM.print("\r\n");
	delay(100);
	LORA_STREAM.print("radio set sf ");
	LORA_STREAM.print(SprFactor);
	LORA_STREAM.print("\r\n");
	delay(100);
	LORA_STREAM.print("radio set wdt ");
	LORA_STREAM.print(readDelay);
	LORA_STREAM.print("\r\n");
	delay(100);
	LORA_STREAM.print("mac pause\r\n");
	delay(100);

	FlushSerialBufferIn();
}

// Flushes any message available
void FlushSerialBufferIn()
{
	while (LORA_STREAM.available() > 0)
	{
#ifdef DEBUG
		CONSOLE_STREAM.write(LORA_STREAM.read());
#else
		LORA_STREAM.read();
#endif
	}
}

// Send Data array (in HEX)
void LORA_Write(char* Data)
{
	LORA_STREAM.print("radio tx ");
	LORA_STREAM.print(Data);
	LORA_STREAM.print("\r\n");
	LORA_STREAM.flush();

	waitTillMessageGone();

}

// Waits until the data transmit is done
void waitTillMessageGone()
{
	while (!LORA_STREAM.available());
	delay(10);
	while (LORA_STREAM.available() > 0)
		LORA_STREAM.read();

	while (!LORA_STREAM.available());
	delay(10);
	while (LORA_STREAM.available() > 0)
	{
#ifdef DEBUG
		CONSOLE_STREAM.write(LORA_STREAM.read());
#else
		LORA_STREAM.read();
#endif
	}
}

// Setting up the receiver to read for incomming messages
void StartLoraRead()
{
	LORA_STREAM.print("radio rx 0\r\n");
	delay(100);

	FlushSerialBufferIn();
}

//////////////////////////////////////
// Read message from P2P TX module  //
// Returns 1 if there is a message  //
// Returns 2 if there is no message //
//////////////////////////////////////
int LORA_Read(char* Data)
{
	int messageFlag = 0;
	String dataStr = "radio_rx  ";
	String errorStr = "radio_err";
	String Buffer = "";

	StartLoraRead();

	while (messageFlag == 0) // As long as there is no message
	{
		while (!LORA_STREAM.available());

		delay(50);  // Some time for the buffer to fill

					// Read message from RN2483 LORA chip
		while (LORA_STREAM.available() > 0 && LORA_STREAM.peek() != LF)
		{
			Buffer += (char)LORA_STREAM.read();
		}

		// If there is an incoming message
		if (Buffer.startsWith(dataStr, 0)) // if there is a message in the buffer
		{
			int i = 10;  // Incoming data starts at the 11th character

						 // Seperate message from string till end of datastring
			while (Buffer[i] != CR && i - 10 < max_dataSize)
			{
				Data[i - 10] = Buffer[i];
				i++;
			}
			messageFlag = 1; // Message received
		}
		else if (Buffer.startsWith(errorStr, 0))
		{
			messageFlag = 2; // Read error or Watchdogtimer timeout
		}
	}

#ifdef DEBUG
	CONSOLE_STREAM.println(Buffer);
#endif

	return (messageFlag);
}
