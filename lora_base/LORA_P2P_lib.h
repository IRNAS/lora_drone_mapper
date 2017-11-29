// LORA_P2P_lib.h

#ifndef _LORA_P2P_LIB_h
#define _LORA_P2P_LIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif



#endif

#define LORA_STREAM Serial1
#define CONSOLE_STREAM SerialUSB
void LoraP2P_Setup(void);
void FlushSerialBufferIn(void);
void LORA_Write(char* Data);
void waitTillMessageGone(void);
void StartLoraRead(void);
int LORA_Read(char* Data);
int getSNR(void);
