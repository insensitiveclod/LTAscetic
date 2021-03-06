#include "ltag_ascetic.h"
#include "lcd_driver.h"
#include "tm_driver.h"
#include "i2c_eeprom.h"
#include "commands.h"




int main (void) {
configuring_ports();

init_timer2();
init_int0();
init_tm();		//Initialize touch-memory reader
init_timer1();
init_timer0();

init_var();		//set variables
init_lcd(); 
lcd_clrscr();
init_joystick();
init_adc();

USART_Init();
EnableRxInt();
EnableTxInt();


eeInit();


_delay_ms(15); 
lcd_puts("LTAscetic \n");
//lcd_gotoxy(0, 1);
lcd_puts("www.open-tag.ru");
//lcd_puts(int_to_str(4,3));
//lcd_clrscr();
//lcd_generate_batt_symbols();


/*
lcd_putc(0);

lcd_putc(1);
lcd_putc(2);
lcd_putc(3);
lcd_putc(4);
lcd_putc(5);
*/

sei(); 

life_leds_status[0] = ON;
timer2 = 0;
while (timer2 < 1000);

life_leds_status[1] = ON;
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED3_ON;
life_leds_status[2] = ON;
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED4_ON;
life_leds_status[3] = ON;
timer2 = 0;
while (timer2 < 1000);

//FIRE_LED_ON;
fire_led_status = ON;
timer2 = 0;
while (timer2 < 1000);

BULLETS_OUT_LED_ON;
timer2 = 0;
while (timer2 < 1000);

WOUND_LED_ON;
timer2 = 0;
while (timer2 < 1000);

FIRE_LED_ON;
timer2 = 0;
while (timer2 < 1000);



lcd_bl_on();
timer2 = 0;
while (timer2 < 1000);


lcd_bl_off();
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED1_OFF;
life_leds_status[0] = OFF;
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED2_OFF;
life_leds_status[1] = OFF;
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED3_OFF;
life_leds_status[2] = OFF;
timer2 = 0;
while (timer2 < 1000);

//LIFE_LED4_OFF;
life_leds_status[3] = OFF;
timer2 = 0;
while (timer2 < 1000);

FIRE_LED_OFF;
timer2 = 0;
while (timer2 < 1000);

BULLETS_OUT_LED_OFF;
timer2 = 0;
while (timer2 < 1000);

WOUND_LED_OFF;
timer2 = 0;
while (timer2 < 1000);

timer2 = 0;
while (timer2 < 1000);
//life_led1_status = FQR_1HZ;
//life_led2_status = FQR_2HZ;
//life_led3_status = FQR_4HZ;
//life_led4_status = ON;

display_life(life);
beep(1000, 3, 128);

USART_SendStr("Hello!\n\r"); // For english
USART_SendStr("Привет!\n\r"); // and russians
USART_SendStrP(command_0);
USART_SendStrP(command_1);

//play_sound_from_eeprom(0,27264);
/*
play_shot_sound();
while(1){
if(update_suond_buffer_now)  // FIXME: typo here ;)
	{
		sound_buffer_update();
	}
};
*/
/*
volatile bool ok;

volatile uint8_t data_tmp;
data_tmp = 0;
ok = open_eeprom(0);
ok = read_eeprom_byte(&data_tmp);
ok = read_eeprom_byte(&data_tmp);
ok = read_eeprom_byte(&data_tmp);
ok = close_eeprom(&data_tmp);
simples_in_queue = eeprom_read_word(&sound_1_size);
timer1 = 0;
while (timer1 < 10000);
while(eeprom_is_open);
simples_in_queue = eeprom_read_word(&sound_1_size);
*/


invite();
cut_off_sound = (eeprom_read_word(&sound_1_size)/100)*(100-CUT_OFF_SOUNT);
joystick_event=no_pressing;
display_status();

timer2 = 0;
while (timer2 < 1000);


while(1){

////volatile uint16_t adc_data;
////volatile uint16_t batt_voltage;
////adc_data=ReadADC(ADC_CHANNEL);
//adc_data=(adc_data/4)*7.5;
//USART_SendStr("Hello!\n\r");
if (simples_in_queue==0) display_voltage_update();
//if(update_suond_buffer_now)
//	{
//		sound_buffer_update();
//	}
//batt_voltage = (adc_data/4)*7.5;
//batt_voltage = batt_voltage/10;

if (chit_detected)
	{
		lcd_clrscr();
		lcd_puts("Ошибка датчиков,"); // "error sensors"
		lcd_gotoxy(0, 1);
		lcd_puts("Проверь повязку!"); //check the bandana
		while (chit_detected)
		{
			beep(1000, 3, 128);
			beep(500, 3, 128); //play sound (frequency, duration, loudness)
		};
		keyboard_event=no_key_pressing;
		reload_key_event=no_key_pressing;
		joystick_event = no_pressing;
		display_status();
	}

switch(keyboard_event)
	{
	 	case no_key_pressing: break;
		case key_pressing:
		{
			if (bullets > 0)
			{
				bullets--;		//take one bullet	
				//last_simple = 0;	//play 'SHOT' sound
				
				if (simples_in_queue>1)		//If there are bytes left in sample queue
				{
					simples_in_queue=1;	//close EEPROM
					while (eeprom_is_open);	//and wait for EEPROM to really be closed before we continue
				}
				 
				simples_in_queue = eeprom_read_word(&sound_1_size);
				
				
				send_ir_package();		//produce a 'SHOT' packet
			//	display_bullets_update();
				
	 		}
			else
			{
				if (simples_in_queue==0) 	//if there are no bytes left in the sample queue
				{
					play_sound_5();
				}
			}
			keyboard_event=no_key_pressing; 
		} 
        break;
  

		default:keyboard_event=no_key_pressing;	
	}


switch(reload_key_event)
	{
	 	case no_key_pressing: break;
		case key_pressing:
		{
			if ((clips > 0)&&(simples_in_queue==0))//If we still have clips left, and not playing SHOT sound...
			{
				playclipinsound();
				clips--;		//we take a clip
				bullets = eeprom_read_byte(&eeprom_bullets_in_clip);
				display_clips_update();
				display_bullets_update();
				for (int i=0; i< eeprom_read_byte(&eeprom_reload_duration); i++){
				timer2 = 0;
				while (timer2 < 8000);
				}
				//while (timer1 < 10000);
				playclipoutsound();
				BULLETS_OUT_LED_OFF;
		//		last_simple = 0;		//play a 'SHOT' sound
		//		send_ir_package();		//send a 'SHOT' packet
				
	 		}
			reload_key_event=no_key_pressing; 
		} 
        break;
  

		default:reload_key_event=no_key_pressing;	
	}


	{
		switch(rx_event)	//Find out what event we received from the IR-receiver
					{
						case RX_COMPLETE: 	//Packet received completely
						{
						//	cli();
						/*********************************************************
							WOUND_LED_ON; //turn on the auxiliary LED
							timer1=0;
							while(timer1 < 35000);
							WOUND_LED_OFF;	//turn off auxiliary LED
						************************************************************/
							
							rx_event = NOT_EVENT;	
							if(!get_buffer_bit(0)) //if bit is set to '0', this is packet of data.
							{
						
								
	//							uint8_t player_id;
								rx_packet = get_packet_value();
//								volatile int gg;
//								gg++;

								if ((rx_packet.team_id != team_id())||(eeprom_read_byte(&friendly_fire_enable)&&(rx_packet.player_id != eeprom_read_byte(&eeprom_player_id))))//'BULLET' received from player not in our own team
								{
									WOUND_LED_ON; // Turn on axiliary LED
									lcd_bl_on();
									display_hit_data();
									playhitsound();

									WOUND_LED_OFF;





								if (life_in_percent > rx_packet.damage) 
									{ // We got wounded, but still live..
										life_in_percent = life_in_percent-rx_packet.damage;
										life = (life_in_percent*10)/125;
										if ((life==0)&&(life_in_percent>0)) life=1;

									}
									else 
									{ //.. or we received too much damaga and are dead.
										life = 0;
										life_in_percent=0;
										WOUND_LED_ON;
										display_life(life);	// display HEALTH state with LEDs
										display_life_update();	// Display HEALTH state with LCD
										volatile uint8_t keypress_cntr; // loop-counter for checking trigger-press duration
										keypress_cntr = 0;
										
										
										
										if ((eeprom_read_byte(&eeprom_tm_serial_num.device_code)!=0)&&(eeprom_read_byte(&eeprom_tm_serial_num.device_code)!=0xFF))

										/*If the Touch-Memory info is already in the EEPROM memory*/
										{

											joystick_event=no_pressing;
											keyboard_event=no_key_pressing;
											tm_event=no_tm_event;
											uint8_t tm_valide;
											tm_valide=0;
											lcd_bl_off();
											
											
											while (!tm_valide)
											{//[while]
												lcd_clrscr();
												lcd_home();
												lcd_puts("Для активации"); // "to activate"
												lcd_gotoxy(0, 1);
												lcd_puts("приложите ключ"); // "attach key"
												while (tm_event == no_tm_event)
												{
													WOUND_LED_INVERT;
													timer2 = 0;
													while (timer2 < 1000);	
												
												};
												switch(tm_event)
												{//[switch]
													case no_tm_event: 
													{
															
													}
													break;
					
													case tm_crc_error: 
													{
														lcd_clrscr();
														lcd_home();
														lcd_puts("Ошибка CRC"); // "CRC error"
														timer2 = 0;
														while (timer2 < 6000){};
														tm_event=no_tm_event;
													}
													break;

													case tm_crc_ok: 
													{
					
														if (tm_verification()) 	
														{ 	tm_valide=1;
															lcd_clrscr();
															lcd_home();
															lcd_puts("Удачи!"); // "Good luck!"
															timer2 = 0;
															while (timer2 < 6000){};
				
															tm_event=no_tm_event;
															break;
														}	
														lcd_clrscr();
														lcd_home();
														lcd_puts("Не тот ключ"); // "Not the key" (FIXME?)
														timer2 = 0;
														while (timer2 < 6000){};
													/*
														lcd_clrscr();
														lcd_home();
														lcd_puts("Для активации"); // "to activate"
														lcd_gotoxy(0, 1);
														lcd_puts("приложи ключ"); // "apply your key"
													*/
														tm_event=no_tm_event;
														
																		
													}

														break;
				
												}//[/switch]
											}//[while]
										
										}//end if
									
									else
									{
										
										lcd_bl_off();
										display_status();
										while(keypress_cntr < 20)
										{
											WOUND_LED_INVERT;
											timer2 = 0;
											while (timer2 < 1000);
											switch (FIRE_KEY_IN&FIRE_KEY_PIN) //Check to see if 'FIRE' trigger is being pulled
											{
												case 0:  keypress_cntr++ ; break;
												case FIRE_KEY_PIN: keypress_cntr = 0; break;
												default: keypress_cntr = 0;	
											}
						
										}

									}	
										
										
										WOUND_LED_OFF;
										init_var();	//"spawn" - start a new game
										joystick_event=no_pressing;
										keyboard_event=no_key_pressing;
										tm_event=no_tm_event;
									//	display_status();
									}
								
								display_life(life);		//display HEALTH using LEDs
//								display_life_update();		//display HEALTH on LCD (FIXME: is this commented out for a reason ?)
								lcd_bl_off();
								display_status();
								}

									
								



	//							rx_player_id = rx_buffer[0];
	//							rx_damage = get_damage_from_rx_paket();

							

							}
							
						
						
						//	sei();
							break;
						}
						
						case RX_ERROR:		//reception error
						{
						//	cli();
							BULLETS_OUT_LED_ON;
							timer2=0;
							while(timer2 < 4000);
							BULLETS_OUT_LED_OFF;
							rx_event = NOT_EVENT;	
						//	sei();
							break;
						}
						
						case NOT_EVENT:		//reception error
						{
						//	cli();
						//	rx_event = NOT_EVENT;	
						//	sei();
							break;
						}



					}



	}

//	timer1=0;				//pause
//	while(timer1 < 65000);			//wait until 65000 timer interrupts has passed (a little less than a second)

	switch(joystick_event)
	{
		case key_up_pressing: 
			{
		//	lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	
	//		if ((result+10)<=max_value) result=result+10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
		/*
			uint16_t adc_data;
			uint16_t batt_voltage;
			adc_data=ReadADC(ADC_CHANNEL);
			adc_data=(adc_data/4)*7.5;
			display_voltage_update(adc_data);
		*/
			switch(display_batt_mode)
			{
				case icon: display_batt_mode=digit;
				break;
				case digit: display_batt_mode=icon;
			
			}
			
			
			
			joystick_event = no_pressing;
			}
	
		break;
		case key_right_pressing: 
			{
			//lcd_clrscr();
			//lcd_home();
//			if ((result)<max_value) result++;
//			lcd_gotoxy(0, 1);
//			lcd_puts(int_to_str(pgm_read_byte(arr_adress+result),3));
			//lcd_puts("Нажата кнопка \n");
			//lcd_puts("Вправо");
			joystick_event = no_pressing;
			}
		break;
		case key_down_pressing: 
			{
	//		lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	//		if((result-9)>0) result=result-10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
			}
		break;
		case key_left_pressing: 
			{
			//lcd_clrscr();
			//lcd_gotoxy(0, 0);
//			if ((result)>0) result--;
			//lcd_puts("Нажата кнопка \n");
			//lcd_puts("Влево");
//			lcd_gotoxy(0, 1);
//			lcd_puts(int_to_str(pgm_read_byte(arr_adress+result),3));
			joystick_event = no_pressing;
			}
		break;
		case key_central_pressing: 
			{
			joystick_event = no_pressing;
			}
		break;
		default: joystick_event = no_pressing;
	
	
		}
	
	if (display_bullets_update_now>0) 
		{
			display_bullets_update();
			display_bullets_update_now--;
		}

	};
    return 0;

}




void configuring_ports(){
//IR_LED_DDR |= IR_LED_PIN; 	//pin connected to IR-LED 'exit' (FIXME: anode/cathode ? )
FIRE_LED_DDR |= FIRE_LED_PIN; 	//pin connected to FIRE-LED 'exit' (FIXME: anode/cathode ? )

IR_LED_HIGH_POWER_DDR|=IR_LED_HIGH_POWER_PIN;	//pin connected to IR-LED 'exit' (FIXME: 'what is 'exit' in this context ?)
IR_LED_HIGH_POWER_OFF;
IR_LED_LOW_POWER_DDR|=IR_LED_LOW_POWER_PIN;	//pin connected to IR-LED 'exit' (FIXME: same as above...)
IR_LED_LOW_POWER_OFF;
/*
LIFE_LEDS_DDR |= LIFE_LED1_PIN  		// Here we set up
				| LIFE_LED2_PIN // the outputs used
				| LIFE_LED3_PIN // for driving the
				| LIFE_LED4_PIN;// 'HEALTH' (life left) LEDS

*/


LIFE_LED1_DDR |= LIFE_LED1_PIN;
LIFE_LED2_DDR |= LIFE_LED2_PIN;
LIFE_LED3_DDR |= LIFE_LED3_PIN;
LIFE_LED4_DDR |= LIFE_LED4_PIN;


SOUND_DDR |= SOUND_PIN; 			//Set up PWM-output (ADC)... (FIXME: shouldnt this be 'DAC ?)
SOUND_CONTROL_DDR|= SOUND_CONTROL_PIN;		//Set up pin that controls amplifier enable.
SOUND_ON;					//turn on amplifier

BULLETS_OUT_LED_DDR|=BULLETS_OUT_LED_PIN;
//DDRA |= (1 << 4)|(1<<5)|(1<<6)|(1<<7); 	// Prepare PORTA.1 as output выход                                                                                                                                                                    DDRB = 1<<DDB3;					//PB3 (OC0) set as output
//DDRD |= (1 << 7);				//LED-lights on the dressing (FIXME: bandage ? Bandana ?)
DDRB |= (1 << 2);				//disable the amplifier
PORTB &=~(1 << 2);
//PORTD &= ~(1 << 7);				//Turn off LED on the dressing (FIXME: bandage ? bandana ?)
WOUND_LED_DDR |= WOUND_LED_PIN;
TSOP_DDR &=~TSOP_PIN; 				//pin connected to IR-RECEIVER is configured as input
RELOAD_KEY_DDR &=~RELOAD_KEY_PIN; 		//pin connected to 'RELOAD KEY' is configured as input
RELOAD_KEY_PORT |= RELOAD_KEY_PIN;		//enable PULL-UP resistor on 'RELOAD-KEY' pin

FIRE_KEY_DDR&=~FIRE_KEY_PIN;
FIRE_KEY_PORT|=FIRE_KEY_PIN;

FIRE_MODE_KEY_DDR&=~FIRE_MODE_KEY_PIN;
FIRE_MODE_KEY_PORT|=FIRE_MODE_KEY_PIN;

ADC_DDR&=~ADC_PIN;
ADC_PORT&=~ADC_PIN;
}


/**************************************************************************************
* Enabling and configuring TIMER2
* Set Timer-mode to CTC (Clear Timer on Compare)
* Clock - No divider of quartz frequency
* detection of comparision is important to interrupt
* generated at twice the IR-receiver frequency (FIXME: not at all sure about this translation)
***************************************************************************************/


void init_timer2(void){
OCR2 = F_CPU/IR_F0/2-1;		// Interrupts are generated at twice the frequency of IR-carrier
TCCR2 = _BV(CS20)|_BV(WGM21);	// Set timer-mode to CTC (Clear Timer on Compare)
                              	// Clock frequency of resonator (7.372.800 hz)
//TIMSK |= _BV(OCIE2);          // enable capture/compare interrupt
TIMSK &=~_BV(OCIE2);  
}




/**************************************************************************************
* This function sets up the INT0 handler for processing external inputs
***************************************************************************************/
void init_int0(void){
DDRD &=~(1<<2); 				// Set pin to trigger INT0
MCUCR |=_BV(ISC01);				// enable triggering of INT0
MCUCR &=~_BV(ISC00);				// only on HIGH->LOW transition of signal
GICR |=_BV(INT0); 				// enable external triggering of INT0

}



void set_buffer_bit(uint8_t index, bool value){	//set value of bit in IR-receive buffer
uint8_t byte_index;
uint8_t bit_index;
byte_index = index/8; 				//Determine which byte contains the bit being set
bit_index = index - (byte_index*8);		//calculate which bit within that byte
if(value) 
		{
			rx_buffer[byte_index] |= (1<<(7-bit_index));
		}
else	{
			rx_buffer[byte_index] &= ~(1<<(7-bit_index));
		}
}






/*

inline trx_packet get_packet_value(){ 		//Read data from received packet
trx_packet result;
uint8_t byte_tmp;

result.player_id = rx_buffer[0];
byte_tmp = rx_buffer[1];
byte_tmp = byte_tmp << 2; 			//get rid of bit of color commands (FIXME: refers to Team-ID ?)
byte_tmp = byte_tmp >> 4;
result.damage = pgm_read_byte(&(damage_value[byte_tmp]));
result.team_id = rx_buffer[1]>>6;

return result;
}
*/




/**************************************************************************************
* This function produces a 'SHOT' packet
* We reset the cursor-position to be beginning of the data (data_packet.data[0])
* And then we enable the transmitter and send out the data
* Function only returns when all data has been sent(!) (FIXME: not true anymore ? Looks to be INT-based now?)
***************************************************************************************/


void send_ir_package(void){ 	// Send a 'SHOT' packet
//ir_pulse_counter=IR_START;
cursor_position = 0; 		// Set pointer to beginning of datablock
ir_transmitter_on = true;	// Enable IR-transmission
TIMSK |= _BV(OCIE2);		// Set up interrupted on capture/compare
FIRE_LED_ON;


//while (ir_transmitter_on);	//Wait for the packet to complete sending
}

/**************************************************************************************
* Set player identity
* Takes ID as argument for player_id (1-127)
* Manipulates global variable data_packet.player_id
* Constructs properly initialized data_packet.player_id(bit_0...bit_7)
***************************************************************************************/
void set_player_id(uint8_t ID){

uint16_t *p_id_bit; 							//pointer to the bits of the structure player_id
p_id_bit = &data_packet.packet.player_id.bit_6; 			//points to bit 6 of the structure
for (int i=0; i < 7; i++) { 						//Extract the values of the lower 6 bits of the ID parameter
ID = ID << 1; 								//shift left by one bit
if (ID&(1<<7)) 								//if MSB=1
	{
		*p_id_bit++ = IR_ONE; 					//assign 'IR_ONE' value to data_packet.player_id.bit_x
	}
else 
	{
		*p_id_bit++ = IR_ZERO; 					//assign 'IR_ZERO' value to data_packet.player_id.bit_x
	}

}

data_packet.packet.player_id.bit_7 = IR_ZERO; 				//According to protocol, this bit is always 'IR_ZERO'

}



/**************************************************************************************
* Set the team identifier (color)
* Takes argument 'color' (0-3)
* Manipulates global variable data_packet.team_id
* Constructs properly initialized data_packet.team_id (bit_0 and bit_1)
***************************************************************************************/



void set_team_color(tteam_color  color){
	switch(color){

		case Red : {		//According to protocol "00 = red"
				data_packet.packet.team_id.bit_0 = IR_ZERO;
				data_packet.packet.team_id.bit_1 = IR_ZERO;
				break;	
		}
		case Blue: {		//According to protocol "01 = blue"
				data_packet.packet.team_id.bit_0 = IR_ONE;
				data_packet.packet.team_id.bit_1 = IR_ZERO;
				break;	
		}
		case Yellow: {		//According to protocol "10 = yellow"
				data_packet.packet.team_id.bit_0 = IR_ZERO;
				data_packet.packet.team_id.bit_1 = IR_ONE;
				break;	
		}
		case Green: { //According to protocol "11 = green"
				data_packet.packet.team_id.bit_0 = IR_ONE;
				data_packet.packet.team_id.bit_1 = IR_ONE;
				break;	
		}
	}
}




/**************************************************************************************
* Sets the damage of our bullets (the damage)
* Takes argument 'damage' to specify the damage
* Manipulates global variable data_packet.damage
* Constructs properly initialized data_packet.damage (bit_0...bit_3)
***************************************************************************************/


void set_gun_damage(tgun_damage damage){

	switch(damage){
		case Damage_1:{  //According to protocol '0000 = 1'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_2:{  //According to protocol '0001 = 2'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_4:{  //According to protocol '0010 = 4'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_5:{  //According to protocol '0011 = 5'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_7:{  //According to protocol '0100' = 7'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}	
		case Damage_10:{  //According to protocol '0101 = 10'	
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ZERO;	
			break;
		}
		case Damage_15:{  //According to protocol '0110 = 15'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_17:{  //According to protocol '0111 = 17'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ZERO;
			break;
		}
		case Damage_20:{  //According to protocol '1000 = 20'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_25:{  //According to protocol '1001 = 25'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_30:{  //According to protocol '1010 = 30'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_35:{  //According to protocol '1011 = 35'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ZERO;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_40:{  //According to protocol '1100 = 40'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_50:{  //According to protocol '1101 = 50'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ZERO;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_75:{  //According to protocol '1110 = 75'
			data_packet.packet.damage.bit_0 = IR_ZERO;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
		case Damage_100:{  //According to protocol '1111 = 100'
			data_packet.packet.damage.bit_0 = IR_ONE;
			data_packet.packet.damage.bit_1 = IR_ONE;
			data_packet.packet.damage.bit_2 = IR_ONE;
			data_packet.packet.damage.bit_3 = IR_ONE;
			break;
		}
	}
}




void init_var(void){            			//Initializing variables
	check_eeprom();
	last_simple = 0xFFFF; 				//We set the sample to the end of the buffer or else it starts playing at reset
	snd_adress.curr_adress = (uint8_t*)0xFFFF; 	//'budbet otherwise attempt to reproduce the sound (FIXME: This mistranslates)
	ir_transmitter_on=false; 			//Disable transmission of data (since the data isnt constructed yet)
	set_player_id(eeprom_read_byte(&eeprom_player_id));	//set the player ID from EEPROM
	set_team_color(team_id());			//Set the TEAM ID (color)
	set_gun_damage(gun_damage());			//Initialize gun-damage

	clips=eeprom_read_byte(&eeprom_clips);		//initialize clip-count from EEPROM
	bullets=0; //eeprom_read_byte(&eeprom_bullets_in_clip);//Initialize amount of bullets
	data_packet.packet.header = IR_START;		//Set the data_packet header to START_BIT
	data_packet.packet.end_of_data = 0;		//Set this to 0 to indicate end of data
	cursor_position = 0; 				//an index-pointer,0 is beginning of block
	start_bit_received = false;			//We havent received a start_bit yet
	bit_in_rx_buff = 0;				//the receiver buffer is empty
	rx_event = NOT_EVENT;				//reset the receiver event
	//reset_clock(); 				//reset the clock
	life = 8;					//health -100% to drive the HEALTH LED's (FIXME: not sure about translation)
	life_in_percent = 100;				//Value for on LC-Display
	key_pressing_duration.key_1    =0;		//duration-counter is reset
							//Duration (FIXME: Leftover comments ?)
							//Continuous pressing (FIXME: leftover comments ?)
	key_pressing_duration.key_1_inc=1;		//Allow duration_counter for key_1 to increment
	key_pressing_duration.key_2    =0;		//duration counter is reset
							//Duration (FIXME: see above)
							//Continuous pressing (FIXME: see above)
	key_pressing_duration.key_2_inc=1;		//allow duration to increment
	chit_detected_counter=0;
	chit_detected = false;
	display_bullets_update_now = 0;
	display_batt_mode = icon;
	curr_ir_pin = eeprom_read_byte(&eeprom_curr_ir_pin);//Initialize IR-LED transmit power
	cr_received = false; 				//Character '\r' has not been received yet
	simples_in_queue =0;
	//update_suond_buffer_now = 0;
	eeprom_is_open = false;
	receiver_on = false;
}




bool get_buffer_bit(uint8_t index){			//Return value of the bit in the IR receiver buffer
	uint8_t byte_index;
	uint8_t bit_index;
	byte_index = index/8; 				//determine in which byte the index'ed bit is
	bit_index = index - (byte_index*8);		//calculate bit position in found byte
	if(rx_buffer[byte_index]&(1<<(7-bit_index))) return true;
	else return false;
}


inline trx_packet get_packet_value(){ 			//return value of recieved IR packet
	trx_packet result;
	uint8_t byte_tmp;

	result.player_id = rx_buffer[0];
	byte_tmp = rx_buffer[1];
	byte_tmp = byte_tmp << 2;			//get rid of the color-bits (FIXME: is this what it means ?)
	byte_tmp = byte_tmp >> 4;
	result.damage = pgm_read_byte(&(damage_value[byte_tmp]));
	result.team_id = rx_buffer[1]>>6;

	return result;
}



tteam_color team_id()		//returns color of our team as ID (00..11)
{


	tteam_color result;
/*
	switch (SW_TEAM_IN&SW_TEAM_MASK) //проверим состояние переключателя "DAMAGE"
	{
		case SW_TEAM_KEY1_PIN: //1-й ключ в состоянии OFF (разомкнут), а второй замкнут (ON)
		{
			result = Blue;
			//return result;
			break;
		}
		case SW_TEAM_KEY2_PIN://2-й ключ в состоянии OFF (разомкнут), а первый замкнут (ON)
		{
			result = Yellow;
			//return result;
			break;
		}
		
		case SW_TEAM_KEY1_PIN|SW_TEAM_KEY2_PIN: //оба ключа в состоянии OFF
		{
			result = Red;
			//return result;
			break;
		}

		case 0: //оба ключа в состоянии ON
		{
			result = Green;
			//return result;
			break;
		}
		default: result = Red;

	}
*/
	result = eeprom_read_byte(&eeprom_team_id);
	return result;
}



void write_team_id_to_eeprom(tteam_color color){
	eeprom_write_byte(&eeprom_team_id, color);
}


/**************************************************************************************
* Function to prepare TIMER0
* Set it to mode CTC (Clear Timer on Compare)
* Set timer prescaler to 1024
* Make timer trigger interrupt
* Results in 100 interrupts per second
***************************************************************************************/

void init_timer0(void){

	//OCR0 = F_CPU/1024/100-1;		// Interrupts to be generated at a frequency of 100Hz

	OCR0 = 128; 				//Duty-cycle = 0.5
	TCCR0 = _BV(WGM01)| _BV(WGM00);		// timer mode - Fast PWM 
	TCCR0 |=  _BV(CS00);            	// clocking at 8MHZ frequency of resonator
	TCCR0 |=  _BV(COM01);    		// non-inverted PWM-mode

	//TIMSK |= _BV(OCIE0);          // enable interrupt on capture/compare
					// enable global interrupts
}


tgun_damage gun_damage()		//Determine the current damage from our tagger
{
/*	tgun_damage result;
	switch (SW_DAMAGE_IN&SW_DAMAGE_MASK) //Check the state of the switch 'DAMAGE'
	{
		case SW_DAMAGE_KEY1_PIN: //1st switch in OFF (open) state and the other is closed (ON)
		{
			result = Damage_50;
			//return result;
			break;
		}
		case SW_DAMAGE_KEY2_PIN://2nd switch in OFF state (open) and the first is closed (ON)
		{
			result = Damage_25;
			//return result;
			break;
		}
		
		case SW_DAMAGE_KEY1_PIN|SW_DAMAGE_KEY2_PIN: 	//Both switches to OFF (open)
		{
			result = Damage_10;
			//return result;
			break;
		}

		case 0: 					//Both switches to ON (closed)
		{
			result = Damage_100;
			//return result;
			break;
		}
		default: result = Damage_25;

	}

	return result;
*/
	return eeprom_read_byte(&eeprom_damage);
}



void init_timer1(void){ 	//Set up timer1 to match sound-sample frequency -8khz
	TCCR1A &=~_BV(WGM10);	//Timer mode - CTC (Clear Timer on Compare)
	TCCR1A &=~_BV(WGM11);
	TCCR1B |=_BV(WGM12); 
	TCCR1B &=~_BV(WGM13); 
	TCCR1A &=~_BV(COM1A0);	//disable timer output on OC1A
	TCCR1A &=~_BV(COM1A1);
	TCCR1B &=~_BV(COM1B0);	//disable timer output on OC1B
	TCCR1B &=~_BV(COM1B1);

	TCCR1B &=~_BV(CS10);	//divider = 8
	TCCR1B |=_BV(CS11);
	TCCR1B &=~_BV(CS12); 
	//OCR1AL=60;
	//OCR1AL=124;
	OCR1AL=(F_CPU/8000)/8-1; 	// set up the sampling frequency - 8Khz
	//OCR1AL=(F_CPU/16000)/8-1; 	// set up the sampling frequency - 8Khz
	//OCR1AL=248;
	//OCR1AH=0x27;
	//OCR1AL=0x0F;

	TIMSK |= _BV(OCIE1A);
	//TIMSK |= _BV(OCIE1B);
}



void display_life(uint8_t life_value) 	//Display HEALTH indication with the HEALTH-LEDS
{

	uint8_t integer_part;
	for (int i=0; i<4; i++)
	{
		life_leds_status[i] = OFF;
	}
	integer_part = life_value/2;
	for (int i=0; i<integer_part; i++)
	{
		life_leds_status[i] = ON;
	}


	if ((life_value-integer_part*2)>0) 
 		life_leds_status[integer_part] = FQR_2HZ;
}

/*
uint8_t bullets_limit(void)		//Determine the amount of rounds set with DIPSWITCH (FIXME: deprecated)
{

uint16_t result;
	switch (SW_BULLETS_LIMIT_IN&SW_BULLETS_LIMIT_MASK) //check the state of the  "BULLETS_LIMIT" switches
	{
		case SW_BULLETS_LIMIT_KEY1_PIN: // OFF+ON
		{
			result = 64;
			//return result;
			break;
		}
		case SW_BULLETS_LIMIT_KEY2_PIN:// ON+OFF
		{
			result = 32;
			//return result;
			break;
		}
		
		case SW_BULLETS_LIMIT_KEY1_PIN|SW_BULLETS_LIMIT_KEY2_PIN: //OFF+OFF
		{
			result = 16;
			//return result;
			break;
		}

		case 0: //ON+ON
		{
			result = 128;
			//return result;
			break;
		}
		default: result = 16;

	}

return result;


}
*/





TFIRE_MODE_STATUS fire_mode()		//Determine fire-mode (Single-shot/Bursts)
{
	TFIRE_MODE_STATUS result;
	if (FIRE_MODE_KEY_IN&FIRE_MODE_KEY_PIN) 
	{
		result = single;
	}
	else  
	{
		result = queues;
	}
	
	return result;
}




void beep(uint16_t fqr, uint16_t count, uint8_t value) 		//Plays a beep (frequency 'fqr' , duration 'count', volume 'value')
{
uint16_t last_simple_tmp;
uint8_t devider; 						//will contain value we need to divide 8Khz with to get to desired frequency 'fqr'
uint16_t beep_counter; 						//Sound duration in cycles (one cycle is one oscillator period)

if (fqr > 4000) return; 					//if requested sound is higher than 4Khz, we cannot play it

last_simple_tmp = last_simple; 					//save the position of the last sample-sound (for later playback)
last_simple = 0xFFFF; 						//to prevent sound from playing


devider = 4000/fqr; 
if (count > 160) count = 160; 					//we don't allow beeps longer than 16 seconds
beep_counter = (fqr/10)*count; 					//number of cycles (periods of oscillation)

for (uint16_t i=0; i < beep_counter; i++)
	{
		OCR0 = value;
		timer2=0;
		while (timer2 < devider);
		OCR0 = 0;
		timer2=0;
		while (timer2 < devider);

	}


	last_simple = last_simple_tmp;				// restore last position of sound played
}

inline void damage_beep(void) 					// play 'HIT' sound (beep)
{
	WOUND_LED_ON; 						//Turn on 'WOUND' LED

	beep(1000, 3, 128);
	beep(500, 3, 128);
	WOUND_LED_OFF;

}


void playhitsound(void) 					// Plays 'HIT' sound (sample)
{
	if (simples_in_queue>1) 				// sample is playing already
	{
		simples_in_queue=1;				//close the EEPROM
		while (eeprom_is_open);				//Wait until EEPROM is closed
	}
	play_sound_2();
/*
	snd_adress.start_adress = &pSnd_hit[0];
	snd_adress.end_adress = &pSnd_hit[sizeof(pSnd_hit)-1];
	snd_adress.curr_adress = &pSnd_hit[0];
	play_hit_snd = true; 	//allow 'HIT' sound
	while (play_hit_snd);	//Wait for audio to end
*/
}


void playclipinsound(void) 		//Plays while 'entering clip' (FIXME: check this)
{
	//play_hit_snd = true; 		//Allow 'HIT' sound
	play_sound_3();
	//play_hit_snd = false; 
/*
	snd_adress.start_adress = &clipinSnd[0];
	snd_adress.end_adress = &clipinSnd[sizeof(clipinSnd)-1];
	snd_adress.curr_adress = &clipinSnd[0];
	play_hit_snd = true; 	// allow HIT sound
	while (play_hit_snd);	// wait for end of audio
*/
}


void playclipoutsound(void) 	//Plays while 'removing clip' (FIXME: Check this)
{
	//play_hit_snd = true; //allow 'HIT' sound
	play_sound_4();
	//play_hit_snd = false; 

/*
	snd_adress.start_adress = &clipoutSnd[0];
	snd_adress.end_adress = &clipoutSnd[sizeof(clipinSnd)-1];
	snd_adress.curr_adress = &clipoutSnd[0];
	play_hit_snd = true; 	//Allow 'HIT' sound
	while (play_hit_snd);	//wait for audio to finish
*/
}




void invite(){ 			//present menu settings

	volatile uint8_t countdown = 5; //countdown

	lcd_clrscr();
	lcd_home();
	if ((eeprom_read_byte(&eeprom_tm_serial_num.device_code)==0)||(eeprom_read_byte(&eeprom_tm_serial_num.device_code)==0xFF))
	/* If the key is not registered*/
	{//[if]
		joystick_event = no_pressing;
		lcd_puts("Запись ключа ТМ"); // "Record Key TM"
		lcd_gotoxy(0, 1);
		lcd_puts("Центр.кн.-отмена"); // "Tsentr.kn-cancel" (Center key - cancel)
		//timer1 = 0;
		while ((joystick_event!=key_central_pressing)&&(eeprom_read_byte(&eeprom_tm_serial_num.device_code)==0)||(eeprom_read_byte(&eeprom_tm_serial_num.device_code)==0xFF)) //While center-button is pressed OR key is not registered
		{//[while]
				
				while ((cr_received==false)&&(joystick_event==no_pressing)&&(tm_event == no_tm_event)){};
				if (cr_received)
				{	
					parsing_command();
					

				}	
				switch(tm_event)
				{//[switch]
					case no_tm_event: 
					{
					
					}
					break;
					
					case tm_crc_error: 
					{
						lcd_clrscr();
						lcd_home();
						lcd_puts("Ошибка CRC"); // Trans: "CRC error"
						timer2 = 0;
						while (timer2 < 6000){};
						lcd_clrscr();
						lcd_home();
						lcd_puts("Запись ключа ТМ"); // Trans: "Record Key TM"
						lcd_gotoxy(0, 1);
						lcd_puts("Центр.кн.-отмена"); // Trans: Tsentr.kn-cancel" (Center key - cancel ?)
						tm_event=no_tm_event;
					}
					break;

					case tm_crc_ok: 
					{
					
						
						eeprom_write_byte(&eeprom_tm_serial_num.device_code,device_code);
						for (int i = 0; i<6; i++ )
						{
							eeprom_write_byte(&eeprom_tm_serial_num.serial[i],tm_code[i]);
						}
						lcd_clrscr();
						lcd_home();
						lcd_puts("Ключ ТМ записан!"); // Trans: "Key TM is written"
						timer2 = 0;
						while (timer2 < 6000){};
						tm_event=no_tm_event;
					
					}

					break;
				
				}//[/switch]
				if (joystick_event==key_central_pressing) 
				break;	

				if (joystick_event!=key_central_pressing) joystick_event = no_pressing;
		
		};//[/while]
		
		if (joystick_event==key_central_pressing) /*if the left by pressing the center button (FIXME: mistranslation)*/
		{
			joystick_event = no_pressing;
			lcd_clrscr();
			lcd_home();

			lcd_puts("Для настроек жми\nцентр. кнопку 5"); // Trans: "To configure PUSH\ncenter button"
			//lcd_puts("Вправо");
			while ((countdown > 0)&&(joystick_event==no_pressing))// Keep pressed till end of countdown or joystick release
			{
				timer2 = 0;
				while ((timer2 < 6000)&&(joystick_event==no_pressing)){};
				if (joystick_event!=no_pressing) break; 	//If the button is pressed, exit the loop
				lcd_gotoxy(14, 1);
				countdown--;
				lcd_puts(int_to_str(countdown,0));
			}

			if (joystick_event==key_central_pressing) 
			{
		
				get_all_setings();
				/*
				get_int_settings("Идент. игрока:", &eeprom_player_id, 127); 	//Pressed the center button
				set_player_id(eeprom_read_byte(&eeprom_player_id));		//Set ID of player from EEPROM
				get_int_settings("Идент. команды:", &eeprom_team_id, 3); 	//pressed the center button
				set_team_color(team_id());					//Set team (color) id
				get_enum_settings("Наносимый урон:", &eeprom_damage, &damage_value, Damage_100);
				set_gun_damage(gun_damage());		//Устанавливаем мощьность оружия (урон)
				get_int_settings("Емкость магазина:", &eeprom_bullets_in_clip, 90); //Pressed the center button
				get_int_settings("Магазинов:", &eeprom_clips, 90);
				get_int_settings("Время перезаряда:", &eeprom_reload_duration, 8);
				*/
		//	return;
			}
		}


//		bullets = eeprom_read_byte(&eeprom_bullets_in_clip);
//		BULLETS_OUT_LED_OFF;
		bullets = 0;
		BULLETS_OUT_LED_ON;
	
		clips = eeprom_read_byte(&eeprom_clips);
		joystick_event=no_pressing;
		keyboard_event=no_key_pressing;
		tm_event=no_tm_event;


	}//[/if]

	if ((eeprom_read_byte(&eeprom_tm_serial_num.device_code)!=0)&&(eeprom_read_byte(&eeprom_tm_serial_num.device_code)!=0xFF))
	/*If TouchMemory is already in EEPROM*/ 
	{//[if]
		
		volatile uint8_t tm_valide=0;
		while (!tm_valide)
		{//[while]
			lcd_clrscr();
			lcd_home();
			lcd_puts("Для активации\nприложи ключ"); // Trans: 'To activate \n attach key'
			//lcd_gotoxy(0, 1);
			//lcd_puts("приложите ключ");		// Trans: 'attach key'
			while ((cr_received==false)&&(tm_event == no_tm_event)){};
			if (cr_received)
			{	
				parsing_command();
			}	
			switch(tm_event)
			{//[switch]
				case no_tm_event: 
				{

				}
				break;
					
				case tm_crc_error: 
				{
					lcd_clrscr();
					lcd_home();
					lcd_puts("Ошибка CRC"); // Trans: 'CRC Error'
					timer2 = 0;
					while (timer2 < 6000){};
					tm_event=no_tm_event;
				}
				break;

				case tm_crc_ok: 
				{
					
					if (tm_verification()) 	
					{
						tm_valide=1;				
						lcd_clrscr();
						lcd_home();
						lcd_puts("Удачи!");	// Trans: 'Good Luck!'
						timer2 = 0;
						while (timer2 < 6000){};
						tm_event=no_tm_event;
						break;
					}
					lcd_clrscr();
					lcd_home();
					lcd_puts("Не тот ключ");	// Trans: 'Not the key' (FIXME: not 'valid' key ?)
					timer2 = 0;
					while (timer2 < 6000){};
					tm_event=no_tm_event;

				}

				break;
				
			}//[/switch]
		}//[/while]
		lcd_clrscr();
		lcd_home();
		joystick_event=no_pressing;
		keyboard_event=no_key_pressing;
		tm_event=no_tm_event;
		lcd_puts("Для настроек\nприложи ключ  5");	// Trans: 'For settings, press key 5' (use keyboard .. or center key?)
		//lcd_puts("Вправо");				// Trans: 'Right'
		while ((countdown > 0)&&(tm_event == no_tm_event)&&(joystick_event==no_pressing))//Until the end of the countdown or if joystick moved
		{
			timer2 = 0;
			while ((timer2 < 6000)&&(joystick_event==no_pressing)){};
			if (joystick_event!=no_pressing) break; //if joystick moved, exit the loop
			lcd_gotoxy(14, 1);
			countdown--;
			lcd_puts(int_to_str(countdown,0));
		}


		switch(tm_event)
		{//[switch]
			case no_tm_event: 
			{

			}
			break;
					
			case tm_crc_error: 
			{
				lcd_clrscr();
				lcd_home();
				lcd_puts("Ошибка CRC");		// Trans: 'CRC Error'
				timer2 = 0;
				while (timer2 < 6000){};
				lcd_clrscr();
				lcd_home();
				lcd_puts("Для настроек");	// Trans: 'In the settings'
				lcd_gotoxy(0, 1);
				lcd_puts("приложи ключ");	// Trans: 'Apply your key'
				tm_event=no_tm_event;
			}
			break;

			case tm_crc_ok: 
			{
					
				if (tm_verification()) 	// Key is correct
				{
					get_all_setings();
					tm_event=no_tm_event;
					break;
				}
				//Foreign key (FIXME: 'strange' key ?)
				lcd_clrscr();
				lcd_home();
				lcd_puts("Не тот ключ"); 	// Trans: 'Not the key' (FIXME: not 'valid' key ?)
				timer2 = 0;
				while (timer2 < 6000){};
				lcd_clrscr();
				lcd_home();
				lcd_puts("Для настроек");	// Trans: 'In the settings'
				lcd_gotoxy(0, 1);
				lcd_puts("приложи ключ");	// Trans: 'Apply your key'
				tm_event=no_tm_event;

					
			}

			break;
				
		}//[/switch]

/*
		if (joystick_event==key_central_pressing) 
		{
		
				get_int_settings("Идент. игрока:", &eeprom_player_id, 127); // Pressed center button
				get_int_settings("Идент. команды:", &eeprom_team_id, 3); // pressed center button
				get_enum_settings("Наносимый урон:", &eeprom_damage, &damage_value, Damage_100);
				get_int_settings("Емкость магазина:", &eeprom_bullets_in_clip, 90); //Pressed center button
				get_int_settings("Магазинов:", &eeprom_clips, 90);
				get_int_settings("Время перезаряда:", &eeprom_reload_duration, 8);
			//	return;
		}

*/


//		bullets = eeprom_read_byte(&eeprom_bullets_in_clip);
//		BULLETS_OUT_LED_OFF;

		bullets = 0;
		BULLETS_OUT_LED_ON;


		clips = eeprom_read_byte(&eeprom_clips);
		joystick_event=no_pressing;
		keyboard_event=no_key_pressing;
	}//[/if]

}


char numbers[] PROGMEM={"0123456789"};
char* int_to_str(uint8_t x, uint8_t digits){
//const char numbers[10]="0123456789";

static volatile char str[6];


volatile uint8_t celoe, ostatok;
celoe=x;
//tmp = celoe/100;
//str[0]=pgm_read_byte(&numbers[tmp]);
//tmp = celoe - t100;
int digits_tmp;
digits_tmp=digits;
if (digits == 0) digits_tmp=3;
      for (int i=(digits_tmp-1); i>=0; i--)
      {   
      //volatile long int tmp;
	 // tmp = celoe;
      ostatok= celoe%10;
	  celoe  = celoe/10;
	  //ostatok= tmp - celoe*10;   
      str[i]= pgm_read_byte(&numbers[ostatok]);
      }
      str[digits_tmp]='\0';
	  
	  
	  
if (digits == 0)	
{
        while ((str[0]=='0')&str[1] !='\0') for (int i=0; i < 6; i++) str[i]=str[i+1];
}

//str[0]=numbers[0];
//str[0]=pgm_read_byte(&numbers[0]);
//str[1]=numbers[1];//pgm_read_byte(&(numbers[1]));
//str[2]=numbers[2];//pgm_read_byte(&(numbers[2]));
//str[3]='\0';

      return &str;      

}

char* long_int_to_str(uint16_t x, uint8_t digits){
//const char numbers[10]="0123456789";

static volatile char str[6];



volatile uint16_t celoe, ostatok;
celoe=x;
//tmp = celoe/100;
//str[0]=pgm_read_byte(&numbers[tmp]);
//tmp = celoe - t100;
int digits_tmp;
digits_tmp=digits;
if (digits == 0) digits_tmp=5;
      for (int i=(digits_tmp-1); i>=0; i--)
      {   
      //volatile long int tmp;
	 // tmp = celoe;
      ostatok= celoe%10;
	  celoe  = celoe/10;
	  //ostatok= tmp - celoe*10;   
      str[i]= pgm_read_byte(&numbers[ostatok]);
      }
      str[digits_tmp]='\0';
	  
	  
	  
if (digits == 0)	
{
        while ((str[0]=='0')&str[1] !='\0') for (int i=0; i < 6; i++) str[i]=str[i+1];
}

//str[0]=numbers[0];
//str[0]=pgm_read_byte(&numbers[0]);
//str[1]=numbers[1];//pgm_read_byte(&(numbers[1]));
//str[2]=numbers[2];//pgm_read_byte(&(numbers[2]));
//str[3]='\0';

      return &str;      

}





volatile void get_int_settings(char* text, uint8_t* var_adress, uint8_t max_value){//Get the value of options using the joystick and LCD
uint8_t result;
joystick_event=no_pressing;
result = eeprom_read_byte(var_adress);
if (result>max_value) result = max_value;

lcd_clrscr();
lcd_puts(text);
lcd_gotoxy(0, 1);
lcd_puts(int_to_str(result,3));
lcd_puts(" центр.кн.-OK");

while  (joystick_event!=key_central_pressing)
{
	while  (joystick_event==no_pressing){};
	switch(joystick_event){
	case key_up_pressing: 
		{
		//	lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	
			if ((result+10)<=max_value) result=result+10;
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
	
		break;
		case key_right_pressing: 
		{
			//lcd_clrscr();
			//lcd_home();
			if ((result)<max_value) result++;
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(result,3));
			//lcd_puts("Нажата кнопка \n");
			//lcd_puts("Вправо");
			joystick_event = no_pressing;
		}
		break;
		case key_down_pressing: 
		{
	//		lcd_clrscr();
	//		lcd_gotoxy(0, 0);
			if((result-9)>0) result=result-10;
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
		break;
		case key_left_pressing: 
		{
			//lcd_clrscr();
			//lcd_gotoxy(0, 0);
			if ((result)>0) result--;
			//lcd_puts("Нажата кнопка \n");
			//lcd_puts("Влево");
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
		break;
		case key_central_pressing: 
		{
			
		}
		break;
		default: joystick_event = no_pressing;
	
	
	}

}

if (result != eeprom_read_byte(var_adress))
	{
	
	lcd_clrscr();
	lcd_puts("Сохранение..."); // Trans: 'Saving...'
	eeprom_write_byte(var_adress, result);	
	}
}





volatile void get_enum_settings(char* text, uint8_t* var_adress, uint8_t* arr_adress, uint8_t max_value)
{
uint8_t result;
uint8_t value;
joystick_event=no_pressing;
result = eeprom_read_byte(var_adress);
value = pgm_read_byte(arr_adress+result);

lcd_clrscr();
lcd_puts(text);
lcd_gotoxy(0, 1);
lcd_puts(int_to_str(value,3));
lcd_puts(" центр.кн.-OK"); // Trans: 'tsentr.kn-OK' (Center key - OK)

while  (joystick_event!=key_central_pressing)
{
	while  (joystick_event==no_pressing){};
	switch(joystick_event){
	case key_up_pressing: 
		{
		//	lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	
	//		if ((result+10)<=max_value) result=result+10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
	
		break;
		case key_right_pressing: 
		{
			//lcd_clrscr();
			//lcd_home();
			if ((result)<max_value) result++;
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(pgm_read_byte(arr_adress+result),3));
			//lcd_puts("Нажата кнопка \n");  	// Trans: 'Key pressed \n';
			//lcd_puts("Вправо");				// Trans: 'Right';
			joystick_event = no_pressing;
		}
		break;
		case key_down_pressing: 
		{
	//		lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	//		if((result-9)>0) result=result-10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
		break;
		case key_left_pressing: 
		{
			//lcd_clrscr();
			//lcd_gotoxy(0, 0);
			if ((result)>0) result--;
			//lcd_puts("Нажата кнопка \n");		// Trans: 'Key pressed \n';
			//lcd_puts("Влево");				// Trans: 'Left'
			lcd_gotoxy(0, 1);
			lcd_puts(int_to_str(pgm_read_byte(arr_adress+result),3));
			joystick_event = no_pressing;
		}
		break;
		case key_central_pressing: 
		{
			
		}
		break;
		default: joystick_event = no_pressing;
	
	
	}
}

if (result != eeprom_read_byte(var_adress))
	{
	
	lcd_clrscr();
	lcd_puts("Сохранение...");					// Trans: 'Saving...'
	eeprom_write_byte(var_adress, result);	
	}

}



void check_eeprom(){

if (eeprom_read_byte(&eeprom_player_id)>127) eeprom_write_byte(&eeprom_player_id,0); 
if (eeprom_read_byte(&eeprom_team_id)>3) eeprom_write_byte(&eeprom_team_id,0); 
if (eeprom_read_byte(&eeprom_damage)>Damage_100) eeprom_write_byte(&eeprom_damage,Damage_10); 
if (eeprom_read_byte(&eeprom_bullets_in_clip)>90) eeprom_write_byte(&eeprom_bullets_in_clip,30); 
if (eeprom_read_byte(&eeprom_clips)>90) eeprom_write_byte(&eeprom_clips,15); 
if (eeprom_read_byte(&eeprom_reload_duration)>8) eeprom_write_byte(&eeprom_reload_duration,1); 
if (eeprom_read_word(&eeprom_batt_full_voltage)==0xFFFF) eeprom_write_word(&eeprom_batt_full_voltage,(DEFAULT_BATT_FULL_VOLTAGE)); 
if (eeprom_read_word(&eeprom_batt_low_voltage)==0xFFFF) eeprom_write_word(&eeprom_batt_low_voltage,(DEFAULT_BATT_LOW_VOLTAGE)); 
if ((eeprom_read_byte(&eeprom_curr_ir_pin)!=IR_LED_HIGH_POWER_PIN)&&(eeprom_read_byte(&eeprom_curr_ir_pin)!=IR_LED_LOW_POWER_PIN)) eeprom_write_byte(&eeprom_curr_ir_pin,IR_LED_HIGH_POWER_PIN); 
if (eeprom_read_byte(&friendly_fire_enable)>1) eeprom_write_byte(&friendly_fire_enable,1); 
}





void display_status()//Display the current value of life, ammunition and clips
{
	lcd_clrscr();
	lcd_puts("жизнь: ");						// Trans: 'Life' (health)
	lcd_puts(int_to_str(life_in_percent,0));
	lcd_puts("% ");
	lcd_gotoxy(0, 1);
	lcd_puts("патр: ");							// Trans: 'Patriarch' (seems to refer to bullets)
	lcd_puts(int_to_str(bullets,0));
	lcd_gotoxy(10, 1);
	lcd_puts("м: ");							// Trans: 'm' (clips ?)
	lcd_puts(int_to_str(clips,0));
	lcd_puts(" ");



}


void display_life_update(){//update the value of life on display
lcd_gotoxy(7, 0);
lcd_puts(int_to_str(life_in_percent,0));
lcd_puts("%   ");
}



void display_bullets_update(){//update the amount of bullets left on display
lcd_gotoxy(6, 1);
lcd_puts(int_to_str(bullets,0));
lcd_puts(" ");
}

void display_clips_update(){//update the amount of clips left on display
lcd_gotoxy(13, 1);
lcd_puts(int_to_str(clips,0));
lcd_puts(" ");
}


void display_voltage_update(){//Update battery-level on display

volatile uint16_t adc_data;
//volatile uint16_t batt_voltage;
adc_data=ReadADC(ADC_CHANNEL);


uint16_t delta; //The difference, expressed in ADC-values, between fully charged and fully discharged batteries
uint8_t curr_batt_level; //	tekuzy urove battery status (1 out of 6 possible) FIXME: error in translation
uint16_t full_level, low_level;//ADC value of fully charged and fully discharged battery-voltages
full_level = (eeprom_read_word(&eeprom_batt_full_voltage)*4)/75;
low_level = (eeprom_read_word(&eeprom_batt_low_voltage)*4)/75;


//delta=((eeprom_read_word(&eeprom_batt_full_voltage)-eeprom_read_word(&eeprom_batt_low_voltage))*4)/75;
delta = full_level - low_level;

lcd_gotoxy(12, 0);

switch(display_batt_mode)
{
	case icon:
	{
		lcd_puts("   ");
	//	lcd_gotoxy(15, 0);
		if (adc_data < low_level) 
		{
//		curr_batt_level=0;
			lcd_putc(0);
			return;
		}
		if (adc_data >full_level)
		{
			lcd_putc(5);
			return;
		}
		curr_batt_level = (6*(adc_data - low_level))/delta;
		lcd_putc(curr_batt_level);
		return;


	}
	break;
	case digit:
	{
		adc_data=(adc_data*15)/8;
		lcd_puts(int_to_str(adc_data/100,0));
		lcd_puts(",");
		lcd_puts(int_to_str((adc_data%100)/10,0));
		lcd_puts(" ");
		return;
	
	}
	break;

}




/*
lcd_gotoxy(12, 0);
lcd_puts(int_to_str(viltage/100,0));
lcd_puts(",");
lcd_puts(int_to_str(viltage%100,0));
lcd_puts(" ");
*/




}



void init_adc(void)
{

ADMUX=((1<<REFS0)|(1<<REFS1));//Select the internal voltage reference
ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Prescaler div factor =128


}


uint16_t ReadADC(uint8_t ch)
{
   //Select ADC Channel ch must be 0-7
   ch=ch&0b00000111;
   ADMUX|=ch;

   //Start Single conversion
   ADCSRA|=(1<<ADSC);

   //Wait for conversion to complete
   while(!(ADCSRA & (1<<ADIF)));

   //Clear ADIF by writing one to it
   //Note you may be wondering why we have write one to clear it
   //This is standard way of clearing bits in io as said in datasheets.
   //The code writes '1' but it result in setting bit to '0' !!!

   ADCSRA|=(1<<ADIF);

   return(ADC);
}


/********************************************
*Show who hit us and what damage was inflicted
********************************************/

void display_hit_data(void)
{
lcd_clrscr();
lcd_home();
lcd_puts("Урон ");							// Trans: Damage
lcd_puts(int_to_str(rx_packet.damage,0));
lcd_puts("% нанес");						// Trans: '% paid' (done ?)
lcd_gotoxy(0, 1);
lcd_puts("иг. ");							// Trans: u. (player ID ?)
lcd_puts(int_to_str(rx_packet.player_id,0));
lcd_puts(" ком. ");							// Trans: com (team id ?)
lcd_puts(int_to_str(rx_packet.team_id,0));
}





void get_ir_power_settings(void)
{
uint8_t result;
uint8_t value;
joystick_event=no_pressing;
result = eeprom_read_byte(&eeprom_curr_ir_pin);
lcd_clrscr();
lcd_puts("Мощность ИК для");				//Trans: Power to the IR (led)
lcd_gotoxy(0, 1);
if (result==IR_LED_HIGH_POWER_PIN) lcd_puts("улицы");	// Trans: 'Street' (Outdoor)
if (result==IR_LED_LOW_POWER_PIN) lcd_puts("помещ");	// Trans: 'ind'		(indoor)

lcd_puts(" цен.кн.-OK");								// Trans: tsen.kn-ok

while  (joystick_event!=key_central_pressing)
{
	while  (joystick_event==no_pressing){};
	switch(joystick_event){
	case key_up_pressing: 
		{
		//	lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	
	//		if ((result+10)<=max_value) result=result+10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
	
		break;
		case key_right_pressing: 
		{
			//lcd_clrscr();
			//lcd_home();
		
		//	if ((result)<max_value) result++;
			if (result==IR_LED_LOW_POWER_PIN)
			{
				result=IR_LED_HIGH_POWER_PIN;
				lcd_gotoxy(0, 1);
				lcd_puts("улицы");				// Trans: 'street'
			}
			
			joystick_event = no_pressing;
		}
		break;
		case key_down_pressing: 
		{
	//		lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	//		if((result-9)>0) result=result-10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
		break;
		case key_left_pressing: 
		{
			//lcd_clrscr();
			//lcd_gotoxy(0, 0);
			
			if (result==IR_LED_HIGH_POWER_PIN) 
			{
				result=IR_LED_LOW_POWER_PIN;
				lcd_gotoxy(0, 1);
				lcd_puts("помещ");			// Trans: 'ind'
			}
			
			joystick_event = no_pressing;
		}
		break;
		case key_central_pressing: 
		{
			
		}
		break;
		default: joystick_event = no_pressing;
	
	
	}
}

if (result != eeprom_read_byte(&eeprom_curr_ir_pin))
	{
	
	lcd_clrscr();
	lcd_puts("Сохранение...");			//Trans: 'Saving...'
	eeprom_write_byte(&eeprom_curr_ir_pin, result);	
	}

}





void get_friendly_fire_settings(void)//Enable/disable detection of friendly-fire
{
uint8_t result;
uint8_t value;
joystick_event=no_pressing;
result = eeprom_read_byte(&friendly_fire_enable);
lcd_clrscr();
lcd_puts("Дружеств. огонь:");		//Trans: 'Commonwealth. fire' 
lcd_gotoxy(0, 1);
if (result) lcd_puts("Да ");		// Trans: 'Yes'
else lcd_puts("Нет");				// Trans: 'No'

lcd_puts(" цен.кн.-OK");

while  (joystick_event!=key_central_pressing)
{
	while  (joystick_event==no_pressing){};
	switch(joystick_event){
	case key_up_pressing: 
		{
		//	lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	
	//		if ((result+10)<=max_value) result=result+10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
	
		break;
		case key_right_pressing: 
		{
			//lcd_clrscr();
			//lcd_home();
		
		//	if ((result)<max_value) result++;
			if (result==false)
			{
				result=true;
				lcd_gotoxy(0, 1);
				lcd_puts("Да ");			//Trans 'Yes'
			}
			
			joystick_event = no_pressing;
		}
		break;
		case key_down_pressing: 
		{
	//		lcd_clrscr();
	//		lcd_gotoxy(0, 0);
	//		if((result-9)>0) result=result-10;
	//		lcd_gotoxy(0, 1);
	//		lcd_puts(int_to_str(result,3));
			joystick_event = no_pressing;
		}
		break;
		case key_left_pressing: 
		{
			//lcd_clrscr();
			//lcd_gotoxy(0, 0);
			
			if (result) 
			{
				result=false;
				lcd_gotoxy(0, 1);
				lcd_puts("Нет");		// Trans: 'No'
			}
			
			joystick_event = no_pressing;
		}
		break;
		case key_central_pressing: 
		{
			
		}
		break;
		default: joystick_event = no_pressing;
	
	
	}
}

if (result != eeprom_read_byte(&friendly_fire_enable))
	{
	
	lcd_clrscr();
	lcd_puts("Сохранение...");					// Trans: 'Saving...'
	eeprom_write_byte(&friendly_fire_enable, result);	
	}

}






void get_all_setings(void)
{
	get_int_settings("Идент. игрока:", &eeprom_player_id, 127); //Pressed the center button
	set_player_id(eeprom_read_byte(&eeprom_player_id));	//Set the ID of the player
	get_int_settings("Идент. команды:", &eeprom_team_id, 3); //Pressed the center button
	set_team_color(team_id());	//Set team id (color)
	get_enum_settings("Наносимый урон:", &eeprom_damage, &damage_value, Damage_100);
	set_gun_damage(gun_damage());		//Set bullet damage-level (Done)
	get_int_settings("Емкость магазина:", &eeprom_bullets_in_clip, 90); //Pressed the center button
	get_int_settings("Магазинов:", &eeprom_clips, 90);
	get_int_settings("Время перезаряда:", &eeprom_reload_duration, 8);
	get_ir_power_settings();
	curr_ir_pin=eeprom_read_byte(&eeprom_curr_ir_pin);
	get_friendly_fire_settings();
}




uint8_t get_command_index(void)//Check for a command that came from the UART
{
volatile uint16_t delta;
volatile char* cursor_pos;
volatile char* buff_pos;

volatile char cmd_buff[32];
uint8_t comand_len;
//char test_str[]="Also firmware";

	

	for (int index=0; index <(sizeof(commandsPointers)/sizeof(char*)); index++)
	{
		
		unsigned char sym;
		uint8_t sym_index = 0;
		char* psym; //pointer to the first character of the command in PROGMEM
//		char* pos;
//		char* pos_buff;
		psym = (char*)(pgm_read_word(&(commandsPointers[index])));
		comand_len = 0;
		while((pgm_read_byte(psym)!=0))
		{
			sym = pgm_read_byte(psym);
			cmd_buff[sym_index++] = sym; //copy command to clipboard
			comand_len++;
			psym++;
		}
		
		cursor_pos = memmem(&(usartRxBuf[rxBufHead]),rxCount,cmd_buff,comand_len);
		buff_pos = &usartRxBuf[0];
		if (memmem(&(usartRxBuf[rxBufHead]),rxCount,cmd_buff,comand_len)!=NULL) 
		{
			delta = (uint16_t)(cursor_pos) - (uint16_t)(buff_pos);
			
			rxBufHead = rxBufHead+comand_len+(unsigned char)(delta);
			rxCount = rxCount-comand_len-(unsigned char)(delta);
//			cursor_pos+=comand_len;
			return index;	
		}
//		if (memmem(test_str,13,cmd_buff,comand_len)!=NULL) return 33;	
		
		//while (sym = pgm_read_byte(p))


	
	}

	return 255; //No such command found in the command-list
}






void get_int_argument_value(uint8_t* var_adress, uint8_t min_val, uint8_t max_val)
{

bool param_not_empty = false;
volatile char ch_tmp;
volatile uint8_t result = 0;

while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStrP(parameter_invalid_error);
			return;
			
			//USART_SendStr("ERROR\n\r");
			//return invalid;//Invalid argument
		}
	}
if (!param_not_empty) 
	{
		USART_SendStrP(parameter_empty_error);
		return;//Empty argument
	}

if ((result>max_val)||(result<min_val))
	{
		USART_SendStrP(parameter_out_of_range_error);
		return;//Argument out of bounds of permitted values
	} 

eeprom_write_byte(var_adress, result);	
USART_SendStr("OK\r\n");


}


void get_word_argument_value(uint8_t* var_adress, uint16_t min_val, uint16_t max_val)
{

bool param_not_empty = false;
volatile char ch_tmp;
volatile uint16_t result = 0;

while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStrP(parameter_invalid_error);
			return;
			
			//USART_SendStr("ERROR\n\r");
			//return invalid;//invalid argument

		}
	}
if (!param_not_empty) 
	{
		USART_SendStrP(parameter_empty_error);
		return;//empty argument
	}

if ((result>max_val)||(result<min_val))
	{
		USART_SendStrP(parameter_out_of_range_error);
		return;//argument out of bounds of permitted values
	} 

eeprom_write_word(var_adress, result);	
USART_SendStr("OK\r\n");


}






void command_0_slot(void){//bullets_in_clip=
get_int_argument_value(&eeprom_bullets_in_clip, 0, MAX_BULL_IN_CLIP);
/*
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint8_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
//lcd_gotoxy(7, 0);
//lcd_puts(int_to_str(result,0));
eeprom_write_byte(&eeprom_bullets_in_clip, result);	
USART_SendStr("OK\r\n");
*/
}




void command_1_slot(void){
USART_SendStr(int_to_str(eeprom_read_byte(&eeprom_bullets_in_clip),0));
USART_SendStr("\r\nOK\r\n");





}
void command_2_slot(void){
USART_FlushTxBuf();
USART_SendStrP(protocol);
USART_SendStr("OK\r\n");

}
uint8_t char_to_int(char c)
     {
       switch(c)
       {
         case '0': return 0;
         case '1': return 1;
         case '2': return 2;
         case '3': return 3;
         case '4': return 4;
         case '5': return 5;
         case '6': return 6;
         case '7': return 7;
         case '8': return 8;
         case '9': return 9;
         default : return 0x55;
         
       }
     }


void command_3_slot(void){
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint8_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
//lcd_gotoxy(7, 0);
//lcd_puts(int_to_str(result,0));
eeprom_write_byte(&eeprom_clips, result);	
USART_SendStr("OK\r\n");
}

void command_4_slot(void){
USART_SendStr(int_to_str(eeprom_read_byte(&eeprom_clips),0));
USART_SendStr("\r\nOK\r\n");
}


void command_5_slot(void){
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint16_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
if (result>512) 
{
	USART_SendStrP(parameter_out_of_range_error);
	return;
}
USART_FlushRxBuf();
//uart_timer = 0;
//while (uart_timer < 40);
USART_SendStr("OK\r\n");

uart_timer = 0;
while ((uart_timer < 650)&&(rxCount<128)); //read until 128 bytes have been received
if (rxCount>=128)
	{
		if(eeWriteBytes(&usartRxBuf[0], result*128, 128)) 
		{
//			uart_timer = 0;
//			while (uart_timer < 40);
			USART_SendStr("OK\r\n");
		}
		else 
		{
			USART_SendStr("ERROR:eeprom write error\r\n");
		}
	} 
else 
	{
		USART_SendStr("ERROR\r\n");
	}




}

void command_6_slot(void){
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint16_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //Ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
if (result>512) 
{
	USART_SendStrP(parameter_out_of_range_error);
	return;
}
USART_FlushTxBuf();
	if (eeReadBytes(&usartTxBuf[0],result*128,128))
		{
	
			for (int i = 0; i<128; i++)
			{
				USART_PutChar(usartTxBuf[i]);

			}
//				USART_SendStr("OK\r\n");

				/*
				txCount   = 127;
				
				if(((UCSRA & (1<<UDRE)) != 0)) 
  				{
  					UDR = usartTxBuf[0];
					while (txCount); //until all bytes are sent.
					USART_SendStr("OK\r\n");
  				}
  				else
				{
					
				}
				*/

	
		}
	else
		{
				USART_SendStr("ERROR:eeprom read error\r\n");
		}

}











uint16_t str_to_int()
{
	
}

void apply_sound_get_command(uint16_t* var_adress) {
USART_SendStr(long_int_to_str(eeprom_read_word(var_adress),0));
USART_SendStr("\r\nOK\r\n");
}



void apply_sound_set_command(uint16_t* var_adress) {
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint16_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //Ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
//lcd_gotoxy(7, 0);
//lcd_puts(int_to_str(result,0));
eeprom_write_word(var_adress, result);	
USART_SendStr("OK\r\n");
}

void command_7_slot(void){//"sound_1_adress=";
apply_sound_set_command(&sound_1_adress);
}


void command_8_slot(void){//"sound_1_adress?";
apply_sound_get_command(&sound_1_adress);
}


void command_9_slot(void){//"sound_1_size="
apply_sound_set_command(&sound_1_size);
}



void command_10_slot(void){//"sound_1_size?"
apply_sound_get_command(&sound_1_size);
}


void command_11_slot(void){//"sound_2_adress="
apply_sound_set_command(&sound_2_adress);
}

void command_12_slot(void){//"sound_2_adress?"
apply_sound_get_command(&sound_2_adress);
}


void command_13_slot(void){//"sound_2_size="
apply_sound_set_command(&sound_2_size);


}

void command_14_slot(void){//"sound_2_size?";
apply_sound_get_command(&sound_2_size);
}


void command_15_slot(void){//"sound_3_adress="
apply_sound_set_command(&sound_3_adress);
}

void command_16_slot(void){//"sound_3_adress?"
apply_sound_get_command(&sound_3_adress);
}


void command_17_slot(void){//"sound_3_size="
apply_sound_set_command(&sound_3_size);


}

void command_18_slot(void){//"sound_3_size?"
apply_sound_get_command(&sound_3_size);
}

void command_19_slot(void){//"sound_4_adress="
apply_sound_set_command(&sound_4_adress);
}

void command_20_slot(void){//"sound_4_adress?"
apply_sound_get_command(&sound_4_adress);
}


void command_21_slot(void){//"sound_4_size=";
apply_sound_set_command(&sound_4_size);
}

void command_22_slot(void){//"sound_4_size?"
apply_sound_get_command(&sound_4_size);
}

void command_23_slot(void){//"sound_5_adress="
apply_sound_set_command(&sound_5_adress);
}

void command_24_slot(void){//"sound_5_adress?"
apply_sound_get_command(&sound_5_adress);
}


void command_25_slot(void){//"sound_5_size="
apply_sound_set_command(&sound_5_size);
}

void command_26_slot(void){//"sound_5_size?"
apply_sound_get_command(&sound_5_size);
}


void command_27_slot(void){//"sound_6_adress="
apply_sound_set_command(&sound_6_adress);
}

void command_28_slot(void){//"sound_6_adress?"
apply_sound_get_command(&sound_6_adress);
}


void command_29_slot(void){//"sound_6_size="
apply_sound_set_command(&sound_6_size);
}

void command_30_slot(void){//"sound_6_size?"
apply_sound_get_command(&sound_6_size);
}


void command_31_slot(void){//"play_sound"
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint8_t result = 0;
while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //Ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStr("ERROR\n\r");
			return;
		}
	}

if (!param_not_empty) 
{
	USART_SendStrP(parameter_empty_error);
	return;
}
if ((result > 6)||(result==0))
{
	USART_SendStrP(parameter_out_of_range_error);
	return;
}
//lcd_gotoxy(7, 0);
//lcd_puts(int_to_str(result,0));
switch (result)
	{
		case 1:
		{
			play_sound_1();
		}
		break;
		case 2:
		{
			play_sound_2();
		}
		break;
		case 3:
		{
			play_sound_3();
		}
		break;
		case 4:
		{
			play_sound_4();
		}
		break;
		case 5:
		{
			play_sound_5();
		}
		break;
		case 6:
		{
			play_sound_6();
		}
		break;
		default:
		break;
	}



	
USART_SendStr("OK\r\n");
}




void command_32_slot(void){//"play_shot_sound"
play_sound_1();
//playhitsound();
//play_shot_sound();
USART_SendStr("OK\r\n");
}


void command_33_slot(void){//player_id=
get_int_argument_value(&eeprom_player_id, 0, 127);


}


void command_34_slot(void){//player_id?
USART_SendStr(int_to_str(eeprom_read_byte(&eeprom_player_id),0));
USART_SendStr("\r\nOK\r\n");

}

void command_35_slot(void){//damage=
get_int_argument_value(&eeprom_damage, 0, 15);


}


void command_36_slot(void){//damage?
USART_SendStr(int_to_str(eeprom_read_byte(&eeprom_damage),0));
USART_SendStr("\r\nOK\r\n");

}

void command_37_slot(void){//ir_power=
//get_int_argument_value(&eeprom_damage, 0, 15);
bool param_not_empty = false;
volatile char ch_tmp;
volatile uint8_t result = 0;

while(usartRxBuf[rxBufHead] !='\r')

	{

		ch_tmp = usartRxBuf[rxBufHead++];
		if (ch_tmp==' ') continue; //Ignore whitespace
		if ((ch_tmp >= '0')&&(ch_tmp<= '9'))
		{
			result = result*10+char_to_int(ch_tmp);
			param_not_empty = true;
		}		
		else 
		{
			USART_SendStrP(parameter_invalid_error);
			return;
			
			//USART_SendStr("ERROR\n\r");
			//return invalid;//Invalid argument

		}
	}
if (!param_not_empty) 
	{
		USART_SendStrP(parameter_empty_error);
		return;//Empty argument
	}

if ((result>1)||(result<0))
	{
		USART_SendStrP(parameter_out_of_range_error);
		return;//argument out of bounds of permitted values
	} 

switch (result)
	{
		case 0: eeprom_write_byte(&eeprom_curr_ir_pin, IR_LED_LOW_POWER_PIN);	
		break;
		case 1:  eeprom_write_byte(&eeprom_curr_ir_pin, IR_LED_HIGH_POWER_PIN);
		break;
	}
		

USART_SendStrP(ok_string);


}


void command_38_slot(void){//ir_power?

switch (eeprom_read_byte(&eeprom_curr_ir_pin))
	{
		case IR_LED_LOW_POWER_PIN: USART_SendStr("0");
		break;
		case IR_LED_HIGH_POWER_PIN: USART_SendStr("1");
	} 

USART_SendStrP(ok_string);

}



void command_39_slot(void){//friendly_fire=
get_int_argument_value(&friendly_fire_enable, 0, 1);
}

void command_40_slot(void){//friendly_fire?
USART_SendStr(int_to_str(eeprom_read_byte(&friendly_fire_enable),0));
USART_SendStrP(ok_string);
}


void command_41_slot(void){//team_id=
get_int_argument_value(&eeprom_team_id, 0, 3);

}

void command_42_slot(void){//team_id?
USART_SendStr(int_to_str(eeprom_read_byte(&eeprom_team_id),0));
USART_SendStrP(ok_string);
}


void command_43_slot(void){//batt_full_voltage=
get_word_argument_value(&eeprom_batt_full_voltage, 0, 45000);
}

void command_44_slot(void){//batt_full_voltage?
USART_SendStr(long_int_to_str(eeprom_read_word(&eeprom_batt_full_voltage),0));
USART_SendStrP(ok_string);
}

void command_45_slot(void){//batt_low_voltage=
get_word_argument_value(&eeprom_batt_low_voltage, 4500, 45000);
}

void command_46_slot(void){//batt_low_voltage?
USART_SendStr(long_int_to_str(eeprom_read_word(&eeprom_batt_low_voltage),0));
USART_SendStrP(ok_string);
}


void parsing_command(void)
{
uint8_t cmd_index;
					DisableRxInt();
					cmd_index = get_command_index();
					switch(cmd_index)
					{
						case 0: command_0_slot();
						break;
						case 1: command_1_slot();
						break;
						case 2: command_2_slot();
						break;
						case 3: command_3_slot();
						break;
						case 4: command_4_slot();
						break;
						case 5: command_5_slot();
						break;
						case 6: command_6_slot();
						break;
						case 7: command_7_slot();
						break;
						case 8: command_8_slot();
						break;
						case 9: command_9_slot();
						break;
						case 10: command_10_slot();
						break;
						case 11: command_11_slot();
						break;
						case 12: command_12_slot();
						break;
						case 13: command_13_slot();
						break;
						case 14: command_14_slot();
						break;
						case 15: command_15_slot();
						break;
						case 16: command_16_slot();
						break;
						case 17: command_17_slot();
						break;
						case 18: command_18_slot();
						break;
						case 19: command_19_slot();
						break;
						case 20: command_20_slot();
						break;
						case 21: command_21_slot();
						break;
						case 22: command_22_slot();
						break;
						case 23: command_23_slot();
						break;
						case 24: command_24_slot();
						break;
						case 25: command_25_slot();
						break;
						case 26: command_26_slot();
						break;
						case 27: command_27_slot();
						break;
						case 28: command_28_slot();
						break;
						case 29: command_29_slot();
						break;
						case 30: command_30_slot();
						break;
						case 31: command_31_slot();
						break;		
						case 32: command_32_slot();
						break;		
						case 33: command_33_slot();
						break;
						case 34: command_34_slot();
						break;
						case 35: command_35_slot();
						break;
						case 36: command_36_slot();
						break;
						case 37: command_37_slot();
						break;
						case 38: command_38_slot();
						break;
						case 39: command_39_slot();
						break;
						case 40: command_40_slot();
						break;
						case 41: command_41_slot();
						break;
						case 42: command_42_slot();
						break;
						case 43: command_43_slot();
						break;
						case 44: command_44_slot();
						break;	
						case 45: command_45_slot();
						break;
						case 46: command_46_slot();
						break;	
						
						
														
						default:
						{
							USART_SendStrP(unknown_command_error);
						}

					}
				
					cr_received = false;
					USART_FlushRxBuf();


}



bool play_sound_from_eeprom(uint16_t address, uint16_t data_size) //plays sound directly from EEPROM

{ 

//	    uint8_t data; //Variable to which we write the first byte
	 
	//The same piece of code as  eeWriteByte...
	/*****Establishes a link with the EEPROM********/
	    do
	    {
	        TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	        while(!(TWCR & (1<<TWINT)));
	 
	        if((TWSR & 0xF8) != TW_START)
	            return false;
	 
	        TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + WRITEFLAG;
	        TWCR=(1<<TWINT)|(1<<TWEN);
	 
	        while(!(TWCR & (1<<TWINT)));
	 
	    }while((TWSR & 0xF8) != TW_MT_SLA_ACK);
	 
	/*****pass the address to READ from********/
	    TWDR=(address>>8);
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
	        return false;
	 
	    TWDR=(address);
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
	        return false;
	 
	/*****Switch to READ mode********/
	/*This is required to 'contact' the slave. First we sent out the chip address (slaveAddressConst << 4) + (slaveAddressVar << 1) + WRITEFLAG to tell the chip which address we want to read a data-byte from. Now we switch to read (because we want to read that byte). This by writing '(slaveAddressConst << 4) + (slaveAddressVar << 1) + READFLAG to the bus*/
	 
	    //We again initialize the bus with a 'repeated start condition'
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//... and wait for the operation to finish
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; the 'repeated start condition' should be detected. (0x10 = TW_REP_START) */
	    if((TWSR & 0xF8) != TW_REP_START)
	        return false;
	 
	    /*Write the address (7bits) and the end-bit (1bit)*/
	    //TWDR=0b1010’000’1;
	    TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG;        
	 
	//Send...
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Check to see if a device was detected at address 0b1010'000 and if it's ready to read from*/
	    if((TWSR & 0xF8) != TW_MR_SLA_ACK)
	        return false;
	 
	/*****Reads a byte of data********/
	 for(uint16_t i=0; i < (data_size - 1); i++)//find all bytes, except for the last
	{

	/*Start data-reception by clearing the interrupt flag TWINT. TWDR will contain the byte that is read*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	 
	    //Wait for reception to finish...
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_ACK)
	        return false;
	 
	    /*Take the byte out of the i2c data-register (TWDR) and stick it into a variable.*/
	    while (fcurr_simple_prepared); //Wait for the interrupt to have set the flag
		curr_simple = TWDR; //take the data out...
		fcurr_simple_prepared = true;//and set flag to push the sample-byte out through the DAC
//		*buffer = TWDR;
//		buffer++;
		//data=TWDR;
	}

	 /*Start data-reception by clearing interrupt flag TWINT. Received byte is written to TWDR*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //Wait for reception to finish...
	    while(!(TWCR & (1<<TWINT)));
	 
	/* We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /* Take the byte out of the i2c data-register (TWDR) and stick it into a variable.*/
	    

		while (fcurr_simple_prepared); //Wait for the interrupt to have set the flag.
		curr_simple = TWDR; //take the data out...
		fcurr_simple_prepared = true;//and set flag to push the sample-byte out through the DAC
//		*buffer = TWDR;


	    /*Set the STOP condition to indicate we're done reading data.*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //We wait for the control-register to have updated.
	    while(TWCR & (1<<TWSTO));
	 
    //Read the byte.
    OCR0 = 0;
	curr_simple=0;
	fcurr_simple_prepared=false;
	return true;
}

void play_sound_1(){//reproduce sound number 1
play_sound_from_eeprom(eeprom_read_word(&sound_1_adress),eeprom_read_word(&sound_1_size));
}
void play_sound_2(){//reproduce sound number 2
play_sound_from_eeprom(eeprom_read_word(&sound_2_adress),eeprom_read_word(&sound_2_size));
}

void play_sound_3(){//reproduce sound number 3
play_sound_from_eeprom(eeprom_read_word(&sound_3_adress),eeprom_read_word(&sound_3_size));
}
void play_sound_4(){//reproduce sound number 4
play_sound_from_eeprom(eeprom_read_word(&sound_4_adress),eeprom_read_word(&sound_4_size));
}
void play_sound_5(){//reproduce sound number 5
play_sound_from_eeprom(eeprom_read_word(&sound_5_adress),eeprom_read_word(&sound_5_size));
}

void play_sound_6(){//reproduce sound number 6
play_sound_from_eeprom(eeprom_read_word(&sound_6_adress),eeprom_read_word(&sound_6_size));
}

/*
void sound_buffer_update(){//Put new bytes into the sound-buffer
switch (curr_sound_buffer)
	{
		case sound_buffer_1://sample-data read from the first buffer
		{
			eeReadBytes(&usartTxBuf[0],curr_adress_in_eeprom,256);

		}
		break;
		case sound_buffer_2://sample-data read from the second buffer
		{
			eeReadBytes(&usartRxBuf[0],curr_adress_in_eeprom,256);

		}
		break;
	}

curr_adress_in_eeprom =curr_adress_in_eeprom + 256; 
update_suond_buffer_now = false;//Event has been handled; clear the flag	
}
*/



/*

void play_shot_sound(void){
curr_pos_in_sound_buff=0;
curr_sound_buffer=sound_buffer_1;
update_suond_buffer_now = false;
curr_adress_in_eeprom = eeprom_read_word(&sound_1_adress);
eeReadBytes(&usartRxBuf[0],curr_adress_in_eeprom,256);//Fill buffer 1
curr_adress_in_eeprom = curr_adress_in_eeprom + 256; 
eeReadBytes(&usartTxBuf[0],curr_adress_in_eeprom,256);//Fill buffer 2
curr_adress_in_eeprom = curr_adress_in_eeprom + 256; 
simples_in_queue = eeprom_read_word(&sound_1_size);//done :-)
};

*/
