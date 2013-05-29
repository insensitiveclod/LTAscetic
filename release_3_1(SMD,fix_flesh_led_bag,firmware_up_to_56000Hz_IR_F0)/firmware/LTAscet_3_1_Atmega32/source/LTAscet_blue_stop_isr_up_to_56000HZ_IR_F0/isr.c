#include "ltag_ascetic.h"
//#include "milshot_snd.h"
//#include "joystick_driver.h"
//#include "hitraw.h"
/**************************************************************************************
* The Timer Interrupt
***************************************************************************************/

ISR(TIMER2_COMP_vect){
//timer1++;
static volatile uint8_t prt;


prt = TSOP_IN&TSOP_PIN; 
if (prt==0) //If IR-receiver pin is LOW (demodulated by TSOP)
	{
	//	PORTA &= ~(1 << 0); //Turn on auxiliary LED
		low_level_counter++;//Increment the LOW-level counter 
//	//	if (chit_detected_counter < (IR_ZERO*1000)) chit_detected_counter++;
//	//	if (chit_detected_counter >= (IR_ZERO*1000)) chit_detected=true;

	}
else  // if IR-receiver pin is HIGH (demodulated by TSOP)
	{
	//	PORTA |=(1<<0);	//Deactivate the auxiliary LED
// //		chit_detected_counter = 0;
// //		if (chit_detected) chit_detected=false;
		high_level_counter++;///Increment the HIGH-level counter
		if((start_bit_received)&&(high_level_counter > IR_ZERO*8)/*&&(bit_in_rx_buff>=13)*/)
		{//Set a TIMEOUT deadline for the counter
			start_bit_received	= false; 		//Cancel state
			if (bit_in_rx_buff>=13) rx_event = RX_COMPLETE;	//We have received a full packet..
			else rx_event = RX_ERROR;			//..or we have an RX-error
			
			receiver_on = false;//Turn off the reciever
			if (ir_transmitter_on==false) TIMSK &=~_BV(OCIE2); //If the transmission was not - turn off interrupts (FIXME: Not sure what this is supposed to do/mean)
		}
		if((high_level_counter > IR_ZERO*8)&&(ir_transmitter_on==false))
		{
			receiver_on = false;// turn off the receiver
			TIMSK &=~_BV(OCIE2);
		}
	
	}
if (ir_transmitter_on==true)
	{//If we can send... then do

		if (ir_pulse_counter > 0)		//check the IR-LED pulse-counter
		{					//Not reached ? then we ...
			IR_LED_INVERT;  		//invert the LED-state (for modulation)
			ir_pulse_counter--;
		}
		else					//End reached ? then we...
		{
			IR_LED_OFF;			//turn off the LED
			if ( ir_space_counter > 0)	//check to see if the gap is maintained between the pulses.
			{							//Nope, no gap
			
					IR_LED_OFF;		//Turn off LED
					ir_space_counter--;	//Decrease the countdown-pause with one TICK		
			}
			else //data-packet and gap between bits have been sent..
			{	 //..we need to prepare the next packet for transmission
				
				
				if (data_packet.data[cursor_position]!=0) //Check to see if we don't point to an empty cell
				{
					ir_pulse_counter =data_packet.data[cursor_position++] ; //set up the counter
					ir_space_counter = IR_SPACE;      //and we of course add a space
				}
				else // .. If the cell WAS empty, then all data is transmitted..
				{
					ir_transmitter_on=false; //..so we turn off the transmitter
				//	display_bullets_update();
					FIRE_LED_OFF;
					display_bullets_update_now++;
				// if
					if (!receiver_on) // If there is no packet reception..
					{
						TIMSK &=~_BV(OCIE2);          // ...we disable interrupts for the capture/compare unit 
						
					}
					//TIMSK |= _BV(OCIE2);   
				}	
				
				
				
				
						
			}				 
		}




	}
else 	{//if the transfer is prohibited, we do nothing
		}


}



/**************************************************************************************
* Handler for external input via INT0 interrupt
***************************************************************************************/


ISR(INT0_vect){
TIMSK |= _BV(OCIE2);		// Enable interrupt on capture/compare
receiver_on = true;
if(!(MCUCR&_BV(ISC00)))		//if the interrupt is caused by the recession (FIXME: "recession" is likely wrong word, based on context: 'HI->LO flank ?')
	{
		MCUCR |=_BV(ISC00); 	//cause next interrupt to come from LOW->HIGH transition
		if (start_bit_received)	//If the start-bit has been recieved, packet-reception has started
			{
				if((high_level_counter < (IR_SPACE + ERROR_TOLERANCE))&&(high_level_counter > (IR_SPACE - ERROR_TOLERANCE)))//Check the pause between pulses
				{
					//pause between pulses is within specifications
				}
				else 	//the pause between pulses is not correct
				{	//fix a reception error
						start_bit_received	= false;	//cancel reception state
						bit_in_rx_buff = 0;			//clear the buffer
						rx_event = RX_ERROR;			//generate a 'RECEPTION ERROR' event
//						TIMSK &= ~_BV(OCIE2);			//disable interrupts for the capture/compare
				}
			
			
			}		
		low_level_counter = 0;	//clear LOW-level tick-counter
		high_level_counter = 0;	//clear HIGH-level tick-counter
	}
else 					//Interrupt is caused by the front (FIXME: i am guessing 'LOW-HIGH flank' ?)
	{
		MCUCR &=~_BV(ISC00);	//make next interrupt come from HIGH->LOW transition
		
		if (start_bit_received)	//if start-bit has been received, packet-reception has started
			{
				if((low_level_counter < (IR_ZERO + ERROR_TOLERANCE))&&(low_level_counter > (IR_ZERO - ERROR_TOLERANCE)))//Check to see if bit is a 'ZERO' bit
				{
					set_buffer_bit(bit_in_rx_buff++, false);//The bit is a valid zero bit; we add it to the RX-buffer
				
				}
				else	//No, it's not a valid 'ZERO' bit...
				{
					if((low_level_counter < (IR_ONE + ERROR_TOLERANCE))&&(low_level_counter > (IR_ONE - ERROR_TOLERANCE)))//perhaps it's a 'ONE' bit ?
					{
							set_buffer_bit(bit_in_rx_buff++, true);//Yep, it's a 'ONE' bit, we add it to the RX-buffer		
					}
					else //It's not a 'ZERO' and not a 'ONE'. It's a menace...
					{
						start_bit_received	= false;	//cancel reception state
						bit_in_rx_buff = 0;			//clear the buffer
						rx_event = RX_ERROR;			//generate a 'RECEPTION ERROR' event
//						TIMSK &= ~_BV(OCIE2);			// disable interrupts for the capture/compare
					}
				}
			}
		else 	// start bit has not been received yet; ....let's check to see if this is one...
		{
			if ((low_level_counter < (IR_START + ERROR_TOLERANCE))&&(low_level_counter > (IR_START - ERROR_TOLERANCE))) //Is this a start_bit ?	
			{//This IS the start-bit!
				bit_in_rx_buff = 0;			//we clear the buffer
				start_bit_received	= true;		//and we set the reception state 
				
			}
			else	//It's not a starbit either... It's really useless signal
			{
				//... so we ignore it.
			}
		}
		
		
		low_level_counter = 0;		//clear LOW-level tick-counter
		high_level_counter = 0;		//clear HIGH-level tick-counter
	


	}

}


/**************************************************************************************
* routine for processing INT1 interrupt
* Triggered when touching 'touch-memory' to reader
***************************************************************************************/


ISR(INT1_vect){
tm_connect = true;
volatile uint8_t tmp;
tmp = 0;
tmp++;
tm_read_serial_number();
}




uint8_t read_seimpl(){//Read the next segment from buffer ; defined here for use in TIMER1A Interrupt Service Routine (below).
uint8_t result;

//result = usartRxBuf[curr_pos_in_sound_buff];
if (!eeprom_is_open) {
eeprom_is_open = open_eeprom(eeprom_read_byte(&sound_1_adress));//open the EEPROM
}
if (simples_in_queue==1)	//if this is the last byte in the sample
		{
			close_eeprom(&result);			//read the final byte and close the EEPROM
			eeprom_is_open = false;
		}
else 				//if this is NOT the last byte in the sample
		{
			read_eeprom_byte(&result); //read the next byte
			
		}

if (simples_in_queue==cut_off_sound)//(eeprom_read_word(&sound_1_size)/100)*(100-CUT_OFF_SOUNT))
	{

		if (fire_mode()==queues)
		{
			
		if ((get_keyboard_status()==key_pressed)&&(life>0)&&(bullets>0)) //The fire-trigger has been pressed; we cut the sound off!
			{
							bullets--;		//We subtract 1 bullet from the total, first...
							send_ir_package();	//..and then we produce a 'shot' packet
							
//							last_simple=0;		//restart the sound from start
							close_eeprom(&result);// read the final byte and close the EEPROM
							eeprom_is_open = false;
			//eeprom_is_open = open_eeprom(eeprom_read_byte(&sound_1_adress));//Open the EEPROM
							simples_in_queue=eeprom_read_word(&sound_1_size);

				//			display_bullets_update();
			}
			
		
		}

	}


return result;
}






/**************************************************************************************
* Interrupt service routine for Timer1A; 
* used for playing sound, 
* keyboard handling,
* display updating,
* handling 'HIT's
***************************************************************************************/

ISR(TIMER1_COMPA_vect){
	TIMSK &= ~_BV(OCIE1A);  //disable interrupts for Timer1 to avoid recursion
	sei(); 
	//uart_timer++;
	if ((bullets >0))//&&(!play_hit_snd))//if you are not out of ammo
	{

		if (simples_in_queue>0) //if not zero, we keep playing the sample
		{ 
		
		
			OCR0 =	read_seimpl();
			if (simples_in_queue==1) OCR0=0;
			simples_in_queue--;


		}

//if (()(!play_hit_snd)) OCR0=0;

/***************************************************************************
	if (last_simple == 0) 
	{

	}
	else;
	if (last_simple < sizeof(pSnd))//3913
			{
				if (last_simple==(sizeof(pSnd)/100)*CUT_OFF_SOUNT)
				{
					if (fire_mode()==queues)
					{
						if ((get_keyboard_status()==key_pressed)&&(life>0)) //If 'fire' trigger is pulled; we cut sound off
						{
							bullets--;		//we take one bullet
							send_ir_package();	//..and produce a 'shot' packet
							last_simple=0;		//we reset the sample byte-counter
						}
						else 	{};//fire_led_status=ON;						
					}
					else;
				}
				else;
				
				OCR0 = pgm_read_byte(&(pSnd[last_simple++]));
			}
	if (last_simple >= sizeof(pSnd)&&(last_simple)!=0xFFFF)//3913
			{
			//	last_simple = 0;
			//	PORTA &= ~(1 << 2);
				OCR0 = 128; //Duty-cycle = 0,5
//				fire_led_status=OFF;
				//FIRE_LED_OFF;
			};

********************************************************/
}

/*
if (bullets <= 0) //ran out of ammo
	{
		BULLETS_OUT_LED_ON; // Turn on 'Out of Ammo' LED
		if (last_simple < sizeof(pSnd)) {OCR0 = pgm_read_byte(&(pSnd[last_simple++]));}//дадим выстрелу прозвучать до конца
		else {};//fire_led_status = OFF;
	};

*/

	if (bullets <= 0) //Ran out of ammo
	{
		BULLETS_OUT_LED_ON; // Turn on 'Out of AMMO' LED
		if (simples_in_queue>0) //if bytes left in sample queue, we play them...
		{
			OCR0 =	read_seimpl();
			if (simples_in_queue==1) OCR0=0;
			simples_in_queue--;
		}

		//if (last_simple < sizeof(pSnd)) {OCR0 = pgm_read_byte(&(pSnd[last_simple++]));}//....
		//else {};//fire_led_status = OFF;
	};


	static volatile uint16_t tmp_cntr=0;

	if ((tmp_cntr - (tmp_cntr/100)*100)==0)// time to check for keyboard presses
	{
		
		uart_timer++;
		switch(keyboard_event) 
			{
		  	case no_key_pressing: 
		  		{
					keyboard_event=test_keyboard(); 
					break;
				}
		  	default:;         
			}	

		switch(reload_key_event)
			{
		  	case no_key_pressing: 
		  		{
					reload_key_event=test_reload_key(); 
					break;
				}
		  	default:;         
			}	

		switch(joystick_event)
			{
		  	case no_pressing: 
		  		{
					joystick_event=test_joystick(); 
					break;
				}
		  	default:;         
			}	

	
	
	
	
	}



//cli();

	if (++tmp_cntr > 1000) //It's time to update the display!
	{
		  
/*
		volatile uint16_t adc_data;
volatile uint16_t batt_voltage;
adc_data=ReadADC(ADC_CHANNEL);
//adc_data=(adc_data/4)*7.5;
display_voltage_update(adc_data);
*/		
		
		//test_keyboard();
		//LIFE_LED1_INVERT;
		tmp_cntr = 0;
		static volatile uint8_t bit_mask = 0b00000001;
	
		if ((life_leds_status[0]&bit_mask)==0) 
		{
			LIFE_LED1_OFF;
		}
		else 
		{
			LIFE_LED1_ON;
		};
		if ((life_leds_status[1]&bit_mask)==0) 
		{
			LIFE_LED2_OFF;
		}
		else 
		{
			LIFE_LED2_ON;
		};
		if ((life_leds_status[2]&bit_mask)==0) 
		{
			LIFE_LED3_OFF;
		}
		else 
		{
			LIFE_LED3_ON;
		};

		if ((life_leds_status[3]&bit_mask)==0) 
		{
			LIFE_LED4_OFF;
		}
		else 
		{
			LIFE_LED4_ON;
		};

/*
			if ((fire_led_status&bit_mask)==0)
				{
					FIRE_LED_OFF;
				}
			else
				{
					FIRE_LED_ON;
				};


*/
		bit_mask = (bit_mask<<1);
		if (bit_mask == 0)  bit_mask = 0b00000001;
	
	}


/*
if (play_hit_snd) // if we're in the middle of playing the 'HIT' sound
	{
		if (snd_adress.end_adress >= snd_adress.curr_adress) //... we're not done playing it yet
		{
						
			OCR0 = pgm_read_byte(snd_adress.curr_adress++);
		}

		if (snd_adress.end_adress < snd_adress.curr_adress)//we've completed playing it
		{
		
			snd_adress.curr_adress = (uint8_t*)0xFFFF;
			play_hit_snd = false;
		
		}
	}

*/

if (fcurr_simple_prepared) 
	{
		OCR0 = curr_simple;
		fcurr_simple_prepared = false;
	}

if(!(TSOP_IN&TSOP_PIN))//if INT0 is low 
	{
		if (chit_detected_counter < (4000)) chit_detected_counter++;
		if (chit_detected_counter >= (4000)) chit_detected=true;

	}
else { 			// if INT0 is HIGH
		chit_detected_counter = 0;
		if (chit_detected) chit_detected=false;
}

timer2++;

cli();

TIMSK |= _BV(OCIE1A);  //Now re re-anble the TIMER1 interrupt


}





inline  TKEYBOARD_STATUS get_keyboard_status(void) {

volatile	TKEYBOARD_STATUS s_ret;
	
	switch (FIRE_KEY_IN&FIRE_KEY_PIN) //Check to see if 'FIRE' trigger is pulled
		{
			case FIRE_KEY_PIN: s_ret=no_key_pressed  ; break;
			default: s_ret=key_pressed ;	
		}



		return s_ret;
}



inline  TKEYBOARD_STATUS get_reload_key_status(void) {

volatile	TKEYBOARD_STATUS s_ret;
	
	switch (RELOAD_KEY_IN&RELOAD_KEY_PIN) //Check to see if 'RELOAD' button is pressed (FIXME: why is this in here twice ? See below ?)
		{
			case RELOAD_KEY_PIN: s_ret=no_key_pressed  ; break;
			default: s_ret=key_pressed ;	
		}



		return s_ret;
}



inline  TKEYBOARD_EVENT test_keyboard(void){
	TKEYBOARD_STATUS key_status;
	TKEYBOARD_EVENT ret_ev;
	key_status=get_keyboard_status();
	switch(key_status)  //Check to see if any keyboard button is pressed
	{
		case no_key_pressed: 
		{
			if (key_pressing_duration.key_1>= SHORT_DURATION)
			{
				ret_ev=key_pressing;
                key_pressing_duration.key_1    =0;
                key_pressing_duration.key_1_inc=0;
				return ret_ev;
			
			}

			else 
			{
				ret_ev=no_key_pressing;
                key_pressing_duration.key_1    =0;
                key_pressing_duration.key_1_inc=1;
			
			}
		
		} 
		break;
	 	case key_pressed  : //Key '1' pressed
		{ 
			if(key_pressing_duration.key_1>= SHORT_DURATION)
			{
				ret_ev=key_pressing;
             	key_pressing_duration.key_1    =0;
                key_pressing_duration.key_1_inc=0;
			} 
			else 
			{
				key_pressing_duration.key_1 += key_pressing_duration.key_1_inc; 
                ret_ev=keyboard_event;

			}
		}
		break;
		default: ret_ev=keyboard_event;
	
	}

return   ret_ev;
}




inline  TKEYBOARD_EVENT test_reload_key(void){
	TKEYBOARD_STATUS key_status;
	TKEYBOARD_EVENT ret_ev;
	key_status=get_reload_key_status();
	switch(key_status)  //check if 'RELOAD' pressed (FIXME: why is this in here twice ? see above)
	{
		case no_key_pressed: 
		{
			if (key_pressing_duration.key_2>= SHORT_DURATION)
			{
				ret_ev=key_pressing;
                key_pressing_duration.key_2    =0;
                key_pressing_duration.key_2_inc=0;
				return ret_ev;
			
			}

			else 
			{
				ret_ev=no_key_pressing;
                key_pressing_duration.key_2    =0;
                key_pressing_duration.key_2_inc=1;
			
			}
		
		} 
		break;
	 	case key_pressed  : //'RELOAD' has been pressed
		{ 
			if(key_pressing_duration.key_2>= SHORT_DURATION)
			{
				ret_ev=key_pressing;
             	key_pressing_duration.key_2    =0;
                key_pressing_duration.key_2_inc=0;
			} 
			else 
			{
				key_pressing_duration.key_2 += key_pressing_duration.key_2_inc; 
                ret_ev=reload_key_event;

			}
		}
		break;
		default: ret_ev=reload_key_event;
	
	}

return   ret_ev;
}





