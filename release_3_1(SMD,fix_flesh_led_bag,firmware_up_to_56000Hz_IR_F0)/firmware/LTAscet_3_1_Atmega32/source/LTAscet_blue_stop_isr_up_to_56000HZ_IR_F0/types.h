#include "joystick_driver_types.h"

#define RX_BUFFER_SIZE   8 	//Size of receiver buffer (RX)
#define TX_BUFFER_SIZE   8 	//size of receiver buffer (TX)

#ifndef bool

                        #define bool unsigned char
                        #define true 1
                        #define false 0

#endif

//---------------------------------------------------------------------//

//Define an enumerated type for IR-Receiver events
enum Rx_Event {	
	NOT_EVENT, 		//No events
	RX_COMPLETE, 		//Packet reception completed
	RX_ERROR		//Packet reception error
}; 

typedef enum Rx_Event trx_event;

//---------------------------------------------------------------------//

//Define a structure to store player identification
//Struct-members contain pulse-duration in timer-ticks (IR_ZERO or IR_ONE)
typedef struct PlayerID {
	uint16_t bit_7;	//the last bit (should always be 'IR_ZERO')
	uint16_t bit_6;
	uint16_t bit_5;
	uint16_t bit_4;
	uint16_t bit_3;
	uint16_t bit_2;
	uint16_t bit_1;
	uint16_t bit_0;	//First bit
} tplayer_id;

/*
union player_id_union {
	tplayer_id bits;
	uint8_t data [8];
};
*/


//Define a structure to store TEAM id (color)
//struct-members contain pulse-duration in timer-ticks (IR_ZERO or IR_ONE)
typedef struct TeamID{
	uint16_t bit_1;
	uint16_t bit_0;
} tteam_id;
 

typedef struct Damage{
	uint16_t bit_3; 
	uint16_t bit_2; 
	uint16_t bit_1; 
	uint16_t bit_0;
} tdamage;


//Here we define a data-packet structure
typedef struct DataPacket {
	uint16_t header;	//A data-packet should always start with a 'IR_START' header (1 bit)
	tplayer_id player_id;	//Then comes the player_id (8 bits)
	tteam_id team_id;	//then team_id (2 bits)
	tdamage damage;		//followed by the damage done (4 bits; see miles_protocol.h)
	uint16_t end_of_data;	//and a marker to indicate that the data should be sent (always 0; one bit)
} tdata_packet;

//---------------------------------------------------------------------//

union data_packet_union{
	tdata_packet packet;
uint16_t data[16];
};

//Definition of a structure in which we store received packet-data
typedef struct RX_Packet {
	uint8_t player_id;	//player_id of the player the shot came from...
	uint8_t team_id;	//.. and his team_id..
	uint8_t damage;		//.. as well as how much damage he did to us.
} trx_packet;


//Definition of a structure in which we store addresses relating to audio playback
	typedef struct SoundAddres {
	uint8_t *start_adress;	//address of the first sample-byte
	uint8_t *end_adress;	//address of the last sample-byte
	uint8_t *curr_adress;	//address of the next sample-byte you want to play
} tsnd_adress;


enum typkeyboard_status {
			no_key_pressed,
			key_pressed
			};
typedef enum typkeyboard_status TKEYBOARD_STATUS;

enum typkeyboard_event  {
			no_key_pressing,
			key_pressing,
	        };
typedef enum typkeyboard_event TKEYBOARD_EVENT;


struct pressing_duration
{ 
	uint16_t key_1; 		//duration for the continuous pressing of button 1 ('FIRE' trigger)
	unsigned int key_1_inc:1;	//single bit, allowing/inhibiting incrementing of 'pressed' counter
					//for button 1 ('FIRE' trigger)
	uint16_t key_2; 		//Duration for the continous pressing of button 2 ('RELOAD')
	unsigned int key_2_inc:1;	//single bit, allowing/inhibiting incrementing of 'pressed' counter
					//for button 2 ('RELOAD)
	unsigned int no_key;
};


enum tfire_mode_status {
	single,
	queues
};
typedef enum tfire_mode_status TFIRE_MODE_STATUS;


enum tdisplay_batt_mode {
	icon,
	digit
};
typedef enum tdisplay_batt_mode TDISPLAY_BATT_MODE;


//Define a structure to hold the serial number for the ADMIN Touch-Memory key
typedef struct TM_serial_num {
	unsigned char device_code;	//Device code
	unsigned char serial[6];	//serial number of the key
} ttm_serial_num;

/*

enum sound_buffer{
	sound_buffer_1,
	sound_buffer_2
};
typedef enum sound_buffer TSOUND_BUFFER;
*/



