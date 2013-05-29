/**************************************************************************************
* Global Variables 
***************************************************************************************/
#include <avr/io.h>        // Imports the names for the inputs in PORTA 
#include <util/delay.h>    // Supplies delay-routines
#include <avr/interrupt.h> // We will use the interrupt handling
#include <avr/pgmspace.h>  // We define constants in program-memory
#include <avr/eeprom.h>
#include "types.h"
#include "miles_protocol.h"
//volatile uint16_t timer1; 
volatile uint16_t timer2; 


volatile bool start_bit_received;		//This flag is set when start-bit detected
volatile uint16_t high_level_counter; 		//Duration counter for timing HI-level at IR-reciever pin
										
						//You want to transfer the following

volatile uint16_t bit_in_rx_buff;		//The number of bits recieved in IR-reciever RX-buffer

volatile trx_event rx_event; 			//IR-reciever RX-event flag

volatile uint16_t low_level_counter; 		//Duration counter for timing LO-level at IR-reciever pin

						//You want to transfer the following


volatile uint8_t rx_buffer[RX_BUFFER_SIZE]; 	//IR-reciever RX-buffer

volatile uint8_t tx_buffer[TX_BUFFER_SIZE]; 	//IR-LED TX-buffer


volatile bool ir_transmitter_on;		//when 'true', allows transfer of data via IR-LED

volatile uint16_t ir_pulse_counter; 		//counter of number of pulses of the IR-LED

volatile int ir_space_counter; 			//counter of number of 'spaces' of the IR-LED

volatile union data_packet_union  data_packet; 	//Contains the data to be sent via the IR-LED


volatile uint8_t cursor_position;		//index-variable to the location inside data_packet.data[x]

volatile union data_packet_union  data_packet; 	//Contains the data to be sent via the IR-LED (FIXME: double define ?)
volatile trx_packet rx_packet;			// FIXME: no comment ?

volatile uint8_t life; 				//Life left (health)

volatile uint8_t life_leds_status[4]; 		//Array used to store state of life-indicator LEDs
volatile TKEYBOARD_EVENT  keyboard_event; 	//Keyboard events
volatile TKEYBOARD_EVENT  reload_key_event; 	//'reload' key event
volatile TJOYSTICK_EVENT  joystick_event; 	//joystick event

volatile struct pressing_duration key_pressing_duration;//Structures store key/joystick press-durations (FIXME: for debouncing ?)

volatile struct joystick_pressing_duration joystick_key_pressing_duration;

volatile uint8_t fire_led_status; 		//Status of FIRE-LED (note: muzzle-flash, not IR-LED)

volatile uint8_t bullets;  			//Number of bullets left in loaded clip

volatile uint16_t last_simple;			//index of the last used sample
volatile bool play_hit_snd; 			//When set to 'false' this flag inhibits playing additional sounds

volatile tsnd_adress snd_adress; 		//tsnd_address contains address of array with audio to be played


volatile EEMEM tteam_color eeprom_team_id;	// Initializes team_id from EEPROM
//volatile uint8_t shift_register_buffer; 	//temporary buffer for use with shift-register
volatile EEMEM uint8_t eeprom_player_id;

volatile EEMEM tgun_damage eeprom_damage;	// Initializes gun-damage from EEPROM

volatile EEMEM uint8_t eeprom_bullets_in_clip; 	// Initializes number of bullets in clip left from EEPROM

volatile EEMEM uint8_t eeprom_clips; 		// Initializes number of clips left from EEPROM

volatile EEMEM uint8_t eeprom_reload_duration; 	// Initializes required time for a reload event from EEPROM

volatile EEMEM uint16_t eeprom_batt_full_voltage;	// fully charged battery voltage (expressed in ADC value) from EEPROM

volatile EEMEM uint16_t eeprom_batt_low_voltage;	// fully discharged battery voltage (expressed in ADC value) FROM EEPROM

volatile EEMEM uint8_t eeprom_curr_ir_pin; 	// Initializes IR-LED transmit power from EEPROM

volatile EEMEM ttm_serial_num eeprom_tm_serial_num;

volatile uint8_t clips;				// remaining clips
volatile life_in_percent;			// Life/health in 'percents'

volatile uint16_t chit_detected_counter; 	// FIXME: "length counter bandages off"
volatile bool chit_detected; 			// FIXME: "flag with a value of true, if recorded off bandages"
volatile bool tm_connect; 			// Flag set to 'true' if touch-memory device is connected to the reader


volatile uint8_t display_bullets_update_now; 	// Flag set to force updating of 'bullets left' display
//volatile TTM_EVENT tm_event; 			//Event for touch-memory connection



volatile TDISPLAY_BATT_MODE display_batt_mode; 	//Display-mode of battery-charge (icons or numbers)


volatile uint8_t curr_ir_pin; 			//current of IR-LED pin; will determine the power of IR-radiation from LED


//Array to get the real value of damage done. Real damage = damage_value[Damage_xx]
uint8_t damage_value [] PROGMEM = 
{
	1,   // 0000 = 1   (Damage_1)
	2,   // 0001 = 2   (Damage_2)
	4,   // 0010 = 4   (Damage_4)
	5,   // 0011 = 5   (Damage_5)
	7,   // 0100 = 7   (Damage_7)
	10,  // 0101 = 10  (Damage_10)
	15,  // 0110 = 15  (Damage_15)
	17,  // 0111 = 17  (Damage_17)
	20,  // 1000 = 20  (Damage_20)
	25,  // 1001 = 25  (Damage_25)
	30,  // 1010 = 30  (Damage_30)
	35,  // 1011 = 35  (Damage_35)
	40,  // 1100 = 40  (Damage_40)
	50,  // 1101 = 50  (Damage_50)
	75,  // 1110 = 75  (Damage_75)
	100, // 1111 = 100 (Damage_100)

};

volatile bool cr_received; 		//flag indicating that UART recieved CR ("\r") 

volatile uint16_t uart_timer; 		//counter for UART-timeout
volatile bool fcurr_simple_prepared; 	//flag to indicate that current sound-sample has been prepared and is ready for playback
volatile uint8_t curr_simple; 		//Current sound-sample
volatile EEMEM uint16_t sound_1_adress;	//address of sound 1 in EEPROM
volatile EEMEM uint16_t sound_1_size;	//length in bytes of sound 1 in EEPROM
volatile EEMEM uint16_t sound_2_adress;	//address of sound 2 in EEPROM
volatile EEMEM uint16_t sound_2_size;	//length in bytes of sound 2 in EEPROM
volatile EEMEM uint16_t sound_3_adress;	//address of sound 3 in EEPROM
volatile EEMEM uint16_t sound_3_size;	//length in bytes of sound 3 in EEPROM
volatile EEMEM uint16_t sound_4_adress;	//address of sound 4 in EEPROM
volatile EEMEM uint16_t sound_4_size;	//length in bytes of sound 4 in EEPROM
volatile EEMEM uint16_t sound_5_adress;	//address of sound 5 in EEPROM
volatile EEMEM uint16_t sound_5_size;	//length in bytes of sound 5 in EEPROM
volatile EEMEM uint16_t sound_6_adress;	//address of sound 6 in EEPROM
volatile EEMEM uint16_t sound_6_size;	//length in bytes of sound 6 in EEPROM
volatile EEMEM bool friendly_fire_enable; 	//flag set to 'true' to allow 'friendly fire'

//volatile TSOUND_BUFFER curr_sound_buffer; 	//current buffer sound (FIXME: UNUSED ?)
volatile uint16_t curr_adress_in_eeprom;	//address in EEPROM from which data can be read into the sound-buffer
//volatile uint8_t curr_pos_in_sound_buff;	//current position inside audio-buffer (FIXME: UNUSED ?)
volatile uint16_t simples_in_queue;		//how much of sample is left in queue
//volatile uint8_t update_suond_buffer_now;	//flag that forces updating data in audio-buffer (FIXME: UNUSED ?)
volatile bool eeprom_is_open;			//flag set when access to EEPROM is active at the moment
volatile uint16_t cut_off_sound; 		//number of unplayed sample-bytes at which it's time to cut off playback
volatile bool receiver_on;			//flag set when there is input (FIXME: 'reception IC package ?)
