//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru   
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: Driver for  USART/UART with circular buffer
//
//  Data........: 11.01.10 
//
//  Translation to english and code cleanup: 2013 05 29
//
//***************************************************************************
#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifndef F_CPU
	#define F_CPU 16000000     //Set CPU-frequency to 16Mhz if not set already
#endif
#define BAUD 9600         //desired baud-rate
#include <util/setbaud.h> //Provides macros for calculation of baud-rate

//Macros to enable and disable interrupts related to USART
#define EnableRxInt()   UCSRB |= (1<<RXCIE);
#define DisableRxInt()  UCSRB &= (~(1<<RXCIE));
#define EnableTxInt()   UCSRB |= (1<<TXCIE);
#define DisableTxInt()  UCSRB &= (~(1<<TXCIE));


#define SIZE_BUF 256       //define ring-buffer size <255




void USART_Init(void); 			//Initialize USART
unsigned char USART_GetTxCount(void); 	//Get the number of characters in TX-buffer
void USART_FlushTxBuf(void);		//Clear the TX-Buffer
void USART_PutChar(unsigned char sym);	//Put a character into TX-buffer
void USART_SendStr(char * data);	//Send a string via USART
void USART_SendStrP(char * data);	//Send a string in PROGMEM via USART
unsigned char USART_GetRxCount(void);	//Get the number of characters in the RX-buffer
void USART_FlushRxBuf(void);		//Clear the RX-buffer
unsigned char USART_GetChar(void);	//Retrieve next character in RX-buffer
#endif //USART_H
