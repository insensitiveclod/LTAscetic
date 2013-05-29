// TODO: Fix code-cleanup in tm_read_serial_number
//#include <util/delay.h>    	// Supplies delay-routines

#define TM_TX_PULLDOWN()   DDRD|=(1<<3);
#define TM_TX_RELEASE()   DDRD &=~(1<<3); 	

enum typtm_event {no_tm_event, tm_crc_ok, tm_crc_error};
typedef enum typtm_event TTM_EVENT;






//#define lcd_e_delay()   __asm__ __volatile__( "rjmp 1f\n 1:" );   //#define lcd_e_delay() __asm__ __volatile__( "rjmp 1f\n 1: rjmp 2f\n 2:" );



/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )    
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
    	    "1: sbiw %0,1" "\n\t"                  
    	    "brne 1b"                              // 4 cycles/loop
    	    : "=w" (__count)
    	    : "0" (__count)
    	   );
}


/************************************************************************* 
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/
#define delay(us)  _delayFourCycles( ( ( 1*(F_CPU/4000) )*us)/1000 )



volatile  unsigned char ch;
volatile unsigned char crc8;
volatile unsigned char code;
volatile unsigned char in;
volatile unsigned char device_code; 	//Device code 
volatile unsigned char tm_code[6]; 	//read the TouchMemory code
volatile TTM_EVENT tm_event; 		//touch-memory event





/**************************************************************************************
* This function configures pin for triggering external interrupt INT1
***************************************************************************************/
void init_tm(void){
	tm_connect = false;
	DDRD &=~(1<<3); 			//Set the pin as input for INT1
	PORTD&=~(1<<3);				//Turn off internal pull-up resistor
	MCUCR |=_BV(ISC11);			//Enable triggering of interrupts..
	MCUCR &=~_BV(ISC10);			//..on HIGH->LOW transition
	GICR |=_BV(INT1); 			//Enable interrupt on INT1
	tm_event = no_tm_event;
}



/*****************************************************************************
 * tm_send
 *
 * For sending bytes
 */
void tm_send( unsigned char byte )
{
    unsigned char i,b;
    b=byte;
    for( i = 0; i < 8; i++ )
    {
        volatile unsigned char pulldown_time, recovery_time;

        if( b & 1 )
        {
		/* write-one */
		TM_TX_PULLDOWN();
	        TM_TX_RELEASE();
		delay( 80 );	
        }
        if ((b&1)==0)
        {
          /* write-zero */
		TM_TX_PULLDOWN();
		delay( 70 );
		TM_TX_RELEASE();
		delay( 20 );	 		   
        }

	b >>= 1;
    }
}




/*****************************************************************************
 * tm_read
 *
 * For Receiving bytes
 */
unsigned char tm_read()
{
    unsigned char i, ret = 0;

    for( i = 0; i < 8; i++ )
    {
 //       unsigned char in;

 //       dint();
        TM_TX_PULLDOWN();
 ////       delay( 1 );
        TM_TX_RELEASE();
        delay( 5);
        in =  (PIND) & (1<<3);
//        eint();
        ret >>= 1;
	if(in)
		ret |= (1<<7);
        /* release & recovery */
        delay( 200);
    }
    return ret;
}


/*****************************************************************************
 * new_crc8
 *
 * Calculation of CRC8 checksum
 */
unsigned char new_crc8( unsigned char crc8, unsigned char byte )
{
    unsigned char i;

    for( i = 0; i < 8; i++ )
    {
        if( (crc8 ^ byte) & 1 )
            crc8 = ((crc8 ^ 0x18) >> 1) | 0x80;
        else
            crc8 >>= 1;
        byte >>= 1;
    }
    return crc8;
}




void tm_read_serial_number()
{       
//GICR &=~_BV(INT1); 				//Disable triggering INT1

uint8_t read_attempts = 1;
while( read_attempts-- )      // FIXME: there is NO curly brace here
if (tm_event==no_tm_event)  { // FIXME: There is a curly brace here



	{	delay(512);   // FIXME: But why is there one here ?




		/* reset */
		TM_TX_PULLDOWN();
		delay(480);  /* tRSTL > 480us */
		/* Reset pulse is complete; check to see if level is HIGH */
//		cli(); //Do not allow global interrupt
		TM_TX_RELEASE();
//		GIFR &=~_BV(INTF1);

		delay(7);   /* 15us minimum tPDH */
//		GIFR |=_BV(INTF1);
 		if( (PIND&(1<<3)) == 0 )
        {
            /* probably just shorted input */
//            sei();
            continue;
        }
		 	/* Check 'presence pulse' */
        
		delay(80);  /* 60us minimum tPDL */
//		sei(); //Allow shared interrupts

		if( PIND&(1<<3) )
            /* Something is wrong; level should be LOW */
            continue;

		/* Continue to completion of 'presence pulse' */

        delay(240); /* tPDL */
        if( !(PIND&(1<<3)) )
            /* таймаут */
            continue;

		/* FIXME: translation 'withstands high tRSTH' */
        delay( 480 );
		
		  /* send command 'Read ROM' */
        tm_send( 0x33 );

 /* Read device type */
      
		device_code = tm_read();
        if( 0 == device_code)
        /* Type of device cannot be '0' */
        continue;

		

		crc8 = new_crc8( 0, device_code);


 /* Read code */
        for( ch = 0; ch < 6; ch++ )
        {
            code = tm_read();
            tm_code[ ch ] = code;
            crc8 = new_crc8( crc8, code );
        }
 /* Read and check CRC8 */
        ch = tm_read();
		if( ch != crc8 )
           {
           tm_event=tm_crc_error;           
			continue;
	
           }
        else { 
			tm_event=tm_crc_ok;  
		return;}

	}
///	sei(); //Allow shared interrupts
	}
GIFR |=_BV(INTF1);
//GICR |=_BV(INT1); 				//Enable triggering of  INT1

} 	

/******************************************
* Compare retrieved serial key 
* with value stored in EEPROM
*******************************************/

uint8_t tm_verification(void)
{
	if (eeprom_read_byte(&eeprom_tm_serial_num.device_code)!=device_code) return 0;
	for (int i = 0; i<6; i++ )
		{
			if (eeprom_read_byte(&eeprom_tm_serial_num.serial[i])!=tm_code[i])  return 0;

		}
	return 1;
}
