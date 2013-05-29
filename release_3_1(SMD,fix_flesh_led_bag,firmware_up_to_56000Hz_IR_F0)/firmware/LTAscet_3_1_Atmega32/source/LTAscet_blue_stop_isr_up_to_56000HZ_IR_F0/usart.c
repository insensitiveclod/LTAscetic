//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru   
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: Driver for USART/UART with circular buffer
//
//  Data........: 11.01.10 
//
//  Translation to english and cleanup : 2013 05 29
//***************************************************************************
#include "usart.h"
#include "ltag_ascetic.h"


//Transmit buffer
volatile unsigned char usartTxBuf[SIZE_BUF];
volatile unsigned char txBufTail = 0;
volatile unsigned char txBufHead = 0;
volatile unsigned char txCount = 0;

//Receive buffer
volatile unsigned char usartRxBuf[SIZE_BUF];
volatile unsigned char rxBufTail = 0;
volatile unsigned char rxBufHead = 0;
volatile unsigned char rxCount = 0;

//Initialise USART
void USART_Init(void)
{
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  #if USE_2X
   UCSRA |= (1 << U2X);
  #else
   UCSRA &= (~(1 << U2X));
  #endif
  UCSRB = (1<<RXCIE)/*|(1<<TXCIE)*/|(1<<RXEN)|(1<<TXEN);	//configure both RX and TX and enable interrupts (only for RX here)
  UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);			//8-bit word-size
}

//______________________________________________________________________________
//Return a surprising number of characters from the TX-buffer
unsigned char USART_GetTxCount(void)
{
  return txCount;  
}

//"Clears" the transmit-buffer
void USART_FlushTxBuf(void)
{
  txBufTail = 0;
  txBufHead = 0;
  txCount = 0;
}

//Place a symbol in the buffer, initiate transfer
void USART_PutChar(unsigned char sym)
{
 
 while ((UCSRA & (1<<UDRE)) == 0){};
 UDR = sym;
 /*
  //	if(sym>=192) sym = pgm_read_byte(&(Decode2Rus[sym-192]));
  //if the USART module is available and symbol is first character
  //we write it directly into the UDR register
  if(((UCSRA & (1<<UDRE)) != 0) && (txCount == 0)) 
  {
  	UDR = sym;
  }

 else {
	//if (!(((UCSRA & (1<<UDRE)) != 0) && (txCount == 0))){ 
    if (txCount < SIZE_BUF){    	//if there is still room in the buffer
      usartTxBuf[txBufTail] = sym;	//we add the character
      txCount++;			//We increment the count of characters in TX-buffer
      txBufTail++;			//and the index of the buffer-tail
      if (txBufTail == SIZE_BUF) txBufTail = 0;
    }
  }
*/
}

//function sends a string via USART
void USART_SendStr(char * data)
{
  unsigned char sym;
//  while(*data)
  while((sym = *data++)){
    USART_PutChar(sym);
  }
}

//Function to send a string in PROGMEM via USART

void USART_SendStrP(char * data)
{
  unsigned char sym;
//  while(*data)
  while((sym = pgm_read_byte(data++))){
    USART_PutChar(sym);
  }
}
//Interrupt handler to complete the transfer
ISR(USART_TXC_vect)
{
  if (txCount > 0){              	//If the buffer is not empty yet
    UDR = usartTxBuf[txBufHead];	//we write a character from the buffer into UDR
    txCount--;				//we decrement the count of characters left in TX-buffer
    txBufHead++;			//and increment the buffer-head index
    if (txBufHead == SIZE_BUF) txBufHead = 0;
  } 
} 

//______________________________________________________________________________
//Returns a surprising number of characters in the INPUT buffer
unsigned char USART_GetRxCount(void)
{
  return rxCount;  
}

//"Clears" the receive buffer
void USART_FlushRxBuf(void)
{
  DisableRxInt(); 	//disables the interrupt on the reception
  rxBufTail = 0;
  rxBufHead = 0;
  rxCount = 0;
  EnableRxInt();
}

//Read buffer
unsigned char USART_GetChar(void)
{
  unsigned char sym;
  if (rxCount > 0){                     //If the receive buffer is not empty
    sym = usartRxBuf[rxBufHead];        //We read a symbol from buffer
    rxCount--;                          //decrement the count of characters still in buffer
    rxBufHead++;                        //and increment the index of the buffer-head
    if (rxBufHead == SIZE_BUF) rxBufHead = 0;
    return sym;                         //return the character retrieved from buffer
  }
  return 0;
}


//Interrupt handler to handle completion of reception
ISR(USART_RXC_vect) 
{
  
  if (rxCount < SIZE_BUF){			//If there is still room in the buffer
      usartRxBuf[rxBufTail] = UDR;		//take a symbol from the UDR into the buffer
      if (usartRxBuf[rxBufTail]=='\r')
	  { 
  		cr_received=true;
	  }
	  rxBufTail++;				//Increment the index of the buffer-tail
      if (rxBufTail == SIZE_BUF) rxBufTail = 0;  
      rxCount++;				//increment the count of received symbols
    }
} 

