#include <avr/io.h>        // Imports the names for the inputs in PORTA 
#include <util/delay.h>    // Supplies delay-routines
#include <avr/interrupt.h> // We will use the interrupt handling
#include <avr/pgmspace.h>  // We define constants in program-memory
#include <avr/eeprom.h>
#include <string.h>
//#include <util/uart.h>

#include "definition_of_ports_atmega16.h"
#include "hal.h"
#include "miles_protocol.h"
#include "types.h"
#include "usart.h"
//#include "shift_regist_driver.h"






//Here we declare our functions
//extern void init_shift_register(void);
void configuring_ports(void);		// Initialise all the AVR-PORTS for proper Input and Output settings
void init_timer2(void); 			// Initialise timer2 which we use for helping us receive and transmit IR packets
void init_int0(void);				//Configure the external interrupt INT0 (FIXME: for trigger/buttons ?)
void init_tm(void);				//Configure the external interrupt INT1 (for TouchMemory keys)
void set_buffer_bit(uint8_t, bool);	//Set a bit in the buffer of the IR-transmit buffer
void send_ir_package(void);			//Send a prepared IR-packet out via the IR-LED
void set_player_id(uint8_t);		//Sets the player-ID in our IR-packet data
void set_team_color(tteam_color);	//Sets the TEAM-id (color) in our IR-packet data
void set_gun_damage(tgun_damage);	//Sets the amount of damage in our IR-packet (the bullet-damage)
void init_var(void);				//Initialize variables
bool get_buffer_bit(uint8_t);		//Get a bit from the IR-receiver buffer
inline trx_packet get_packet_value(void); //Get data from a received IR-packet
tteam_color team_id(void);//Team ID (color)
tgun_damage gun_damage(void);//The amount of damage in our received 'shot'
void init_timer0(void); //Initialise timer0 to 'FASTPWM' output mode
void init_timer1(void); //Initialise timer1 to the speed of our sample frequency (8Khz)
void display_life(uint8_t life_value);//Display health-status on our status-LEDs
inline  TKEYBOARD_STATUS get_keyboard_status(void); //Check to see if trigger has been pressed
inline  TKEYBOARD_EVENT test_keyboard(void);//Check for keyboard-events
inline  TKEYBOARD_STATUS get_reload_key_status(void);//Check to see if 'RELOAD' was pressed
inline  TKEYBOARD_EVENT test_reload_key(void);//Check to see if the 'fire-mode' button was pressed (to switch between single/burst)
uint8_t bullets_limit(void);//Determine the amount of rounds in a clip from DIPSWITCH-settings (FIXME: depracated on 3.1 hardware)
TFIRE_MODE_STATUS fire_mode(void);//Determine the current fire-mode setting
void beep(uint16_t, uint16_t, uint8_t); //Plays a beep-sound (frequency,duration,amplitude)
void damage_beep(void); // Plays a sound indicating player has been damaged
void playhitsound(void); // Plays a sound indicating player has been hit
void playclipinsound(void); //Play a sound indicating clip has been loaded
void playclipoutsound(void); //Play a sound indicating clip has been removed
void write_team_id_to_eeprom(tteam_color);
void invite(void); //Inviting in the settings-menu
//inline trx_packet get_packet_value(void); //Read data from the receiver
char* int_to_str(uint8_t x, uint8_t digits);//Convert a number to a string
char* long_int_to_str(uint16_t x, uint8_t digits);//Convert a long-int number to a string
volatile void get_int_settings(char* text, uint8_t* var_adress, uint8_t max_value);//Get/set option values via LCD and keyboard
void check_eeprom(void);//Read EEPROM and check validity of data
volatile void get_enum_settings(char* text, uint8_t* var_adress, uint8_t* arr_adress, uint8_t max_value);
void display_status(void);//Display the status of life, ammunition and clips left
void display_life_update(void);//Updates the value of LIFE (health) on the display
void display_bullets_update(void);//Update the amount of bullets left on the display
void display_clips_update(void);//Update the amount of clips left on the display
void init_adc(void);
uint16_t ReadADC(uint8_t ch);
void display_voltage_update(void);
void display_hit_data(void);//Display who shot us and what damage was done
//void generate_batt_symbols(void);//Create the characters for in the LCD to display battery-life
void get_ir_power_settings(void);
void get_friendly_fire_settings(void);//enable/disable 'friendly fire' settings
void get_all_setings(void);
uint8_t get_command_index(void);//Check for commands received on the UART
uint8_t char_to_int(char);//Convert a character to an integer
//void command_1_slot(void);
bool play_sound_from_eeprom(uint16_t address, uint16_t data_size); //Play a sound directly from the external EEPROM
void play_sound_1(void);//Reproduce sound 1
void play_sound_2(void);//Reproduce sound 2
void play_sound_3(void);//Reproduce sound 3
void play_sound_4(void);//Reproduce sound 4
void play_sound_5(void);//Reproduce sound 5
void play_sound_6(void);//Reproduce sound 6
//void sound_buffer_update(void);//Update the sound-buffer with a byte read from the EEPROM
//uint8_t read_seimpl_from_sound_buffer(void);//Read a byte from the sound-buffer
//void play_shot_sound(void);//Play the 'shot' sound

//Global Variables

extern volatile uint16_t timer1; 
extern volatile uint16_t timer2; 

extern volatile bool start_bit_received;				//This flag is set when a START bit has been received on the IR-receiver
extern volatile uint16_t high_level_counter; 			//Duration counter for timing HI-level at IR-reciever pin
extern volatile uint16_t bit_in_rx_buff; 				//The number of bits recieved in IR-reciever RX-buffer
extern volatile trx_event rx_event; 				//IR-reciever RX-event flag
extern volatile uint16_t low_level_counter; 			//Duration counter for timing LO-level at IR-reciever pin
extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE]; 		//IR-reciever RX-buffer
extern volatile bool ir_transmitter_on;				//when 'true', allows transfer of data via IR-LED
extern volatile uint16_t ir_pulse_counter; 			//counter for timing pulses of the IR-LED
extern volatile int ir_space_counter; 				//counter for timing 'spaces' of the IR-led (inter-bit gap)
//extern volatile trx_packet rx_packet;

extern volatile union data_packet_union  data_packet; 	//Contains the data to be sent via the IR-LED
extern volatile uint8_t cursor_position;				//index-variable to the location inside data_packet.data[x]
extern volatile union data_packet_union  data_packet; 	//Contains the data to be sent via the IR-LED (FIXME: double-define?)
extern uint8_t damage_value [] PROGMEM;
extern volatile trx_packet rx_packet;
extern volatile uint8_t life; //life left (HEALTH)
extern volatile uint8_t life_leds_status[4]; //This array will contain the status of the LIFE indication LEDS
extern volatile TKEYBOARD_EVENT  keyboard_event; //keyboard events (FIXME: trigger ?)
extern volatile TKEYBOARD_EVENT  reload_key_event; //Reload key event
extern volatile TJOYSTICK_EVENT  joystick_event; //'joystick' event
extern volatile struct pressing_duration key_pressing_duration;//Structures store key/joystick press-durations (FIXME: for debouncing ?)
extern volatile struct joystick_pressing_duration joystick_key_pressing_duration;
extern volatile uint8_t fire_led_status; //Status of FIRE-LED (note: muzzle-flash, not IR-LED)
extern volatile uint16_t bullets;  //Number of bullets left in loaded clip
extern volatile uint16_t last_simple;					//Index of byte of last-used sample
extern volatile bool play_hit_snd; //When set to 'false' this flag inhibits playing additional sounds
extern volatile tsnd_adress snd_adress; //tsnd_address contains address of array with audio to be played
//extern volatile uint8_t shift_register_buffer; //temporary buffer for use with shift-register
//extern const unsigned char pSnd_hit[];

extern volatile EEMEM tteam_color eeprom_team_id;
extern volatile EEMEM uint8_t eeprom_player_id;
extern volatile EEMEM tgun_damage eeprom_damage; // Initializes gun-damage from EEPROM
extern volatile EEMEM uint8_t eeprom_bullets_in_clip; // Initializes number of bullets in clip left from EEPROM
extern volatile EEMEM uint8_t eeprom_clips; // Initializes number of clips left from EEPROM
extern volatile EEMEM uint8_t eeprom_reload_duration; // Initializes required time for a reload event from EEPROM
extern volatile uint8_t clips;//remaining clips
extern volatile life_in_percent;// Life/health in 'percents'
extern volatile uint16_t chit_detected_counter; // (FIXME: 'length counter bandages of')
extern volatile bool chit_detected; // (FIXME: 'flag with a value of true, if recorded off bandages)
extern volatile bool tm_connect; //Flag set to true a TouchMemory key has been pressed against the reader
//extern volatile TTM_EVENT tm_event; //TouchMemory key event
extern volatile EEMEM uint16_t eeprom_batt_full_voltage; // ADC-value of a fully-charged battery
extern volatile EEMEM uint16_t eeprom_batt_low_voltage; // ADC-value of a fully discharged battery
extern volatile uint8_t display_bullets_update_now; //flag to indicate it is time to update the display of remaining rounds
extern volatile TDISPLAY_BATT_MODE display_batt_mode; //display-mode of battery-status (icons/numbers)
extern volatile EEMEM uint8_t eeprom_curr_ir_pin; // Determines the current transmit-power of the IR-led; read from EEPROM
extern volatile uint8_t curr_ir_pin; //Current setting of the IR-transmit power
extern volatile EEMEM ttm_serial_num eeprom_tm_serial_num;


extern volatile bool cr_received; //flag that indicates that a character has been received on the UART
extern volatile unsigned char usartTxBuf[SIZE_BUF];
extern volatile unsigned char txBufTail;
extern volatile unsigned char txBufHead;
extern volatile unsigned char txCount;

//Receive buffer of UART
extern volatile unsigned char usartRxBuf[SIZE_BUF];
extern volatile unsigned char rxBufTail;
extern volatile unsigned char rxBufHead;
extern volatile unsigned char rxCount;
extern volatile uint16_t uart_timer; //setting of 'timeout' value for UART transactions

extern volatile bool fcurr_simple_prepared; //Flag to indicate that the sample-byte has been set ready for playback
extern volatile uint8_t curr_simple; //current sound-byte

extern volatile EEMEM uint16_t sound_1_adress;//Address of sound 1 in EEPROM
extern volatile EEMEM uint16_t sound_1_size;//Size of sound 1 in EEPROM
extern volatile EEMEM uint16_t sound_2_adress;//Address of sound 2 in EEPROM
extern volatile EEMEM uint16_t sound_2_size;//Size of sound 2 in EEPROM
extern volatile EEMEM uint16_t sound_3_adress;//Address of sound 3 in EEPROM
extern volatile EEMEM uint16_t sound_3_size;//Size of sound 3 in EEPROM
extern volatile EEMEM uint16_t sound_4_adress;//Address of sound 4 in EEPROM
extern volatile EEMEM uint16_t sound_4_size;//Size of sound 4 in EEPROM
extern volatile EEMEM uint16_t sound_5_adress;//Address of sound 5 in EEPROM
extern volatile EEMEM uint16_t sound_5_size;//Size of sound 5 in EEPROM
extern volatile EEMEM uint16_t sound_6_adress;//Address of sound 6 in EEPROM
extern volatile EEMEM uint16_t sound_6_size;//Size of sound 6 in EEPROM
extern volatile EEMEM bool friendly_fire_enable; //Flag that indicates if friendly fire is allowed (true) or not (false)
//extern volatile TSOUND_BUFFER curr_sound_buffer; //Current buffer from which to read sound-byte
extern volatile uint16_t curr_adress_in_eeprom;//Current address in the external EEPROM from which to read sound-byte
//extern volatile uint8_t curr_pos_in_sound_buff;//current position in the audio playback buffer
extern volatile uint16_t simples_in_queue;//how much is left to play

//extern volatile uint8_t update_suond_buffer_now;//flag indicating that the audio-buffer needs to be updated
extern volatile bool eeprom_is_open;//flag indicating that access to the EEPROM is open at the moment
extern volatile uint16_t cut_off_sound; //number of unplayed sound-bytes at which it's time to cut off the sound
extern const unsigned char Decode2Rus[255-192+1] PROGMEM;
extern volatile bool receiver_on;//flag that indicates the receiver is enabled
