#define IR_START_BIT_DURATION 2400	// duration of START_BIT (in msecs)
#define IR_ONE_BIT_DURATION 1200	// duration of a 'ONE' bit  (in msecs)
#define IR_ZERO_BIT_DURATION 600	// duration of a 'ZERO' bit (in msecs)
#define IR_SPACE_DURATION 600		// duration of if inter-bit interval (gap between bits)
#define IR_F0 56000			// Carrier frequency of IR-receiver (F0)
//#define IR_F0 36000

#define IR_ZERO (IR_ZERO_BIT_DURATION*2*IR_F0)/1000000 	//Amount of timer-ticks for a 'ZERO' bit (TX)

#define RX_IR_ZERO (IR_ZERO_BIT_DURATION*8)/1000 	//Amount of timer-ticks for a 'ZERO' bit (RX)

#define IR_ONE (IR_ONE_BIT_DURATION*2*IR_F0)/1000000 	//Amount of timer-ticks for a 'ONE' bit (TX)

#define RX_IR_ONE (IR_ONE_BIT_DURATION*8)/1000 		//Amount of timer-ticks for a 'ONE' bit (RX)

#define IR_START (IR_START_BIT_DURATION*2*IR_F0)/1000000 	//Amount of timer-ticks for the 'START_BIT' (TX)

#define RX_IR_START (IR_START_BIT_DURATION*8)/1000 		//Amount of timer-ticks for the 'START_BIT' (RX)

#define IR_SPACE  (IR_SPACE_DURATION*2*IR_F0)/1000000 		//Amount of timer-ticks for inter-bit interval (gap between bits) (TX)

#define RX_IR_SPACE  (IR_SPACE_DURATION*8)/1000 		//Amount of timer-ticks for inter-bit interval (gap between bits) (RX)

#define ERROR_TOLERANCE 22					//(At F0=56khz)Tick-tolerance to use for checking bit-validity (expressed in timer-ticks)

//#define ERROR_TOLERANCE 14 					//(At F0=36Khz)Tick-tolerance to use for checking bit-validity (expressed in timer-ticks)


//---------------------------------------------------------------------//

enum Team_Color {		//Define an enumerated type for the colors of the teams
	Red, 	//00 = Red
	Blue, 	//01 = Blue		
	Yellow, //10 = Yellow
	Green}; //11 = Green

typedef enum Team_Color tteam_color;			

//---------------------------------------------------------------------//


//Define an enumerated type for use with the gun-damage
enum GunDamage {	
	
	Damage_1,  //0000 = 1
	Damage_2,  //0001 = 2
	Damage_4,  //0010 = 4
	Damage_5,  //0011 = 5
	Damage_7,  //0100 = 7
	Damage_10, //0101 = 10
	Damage_15, //0110 = 15
	Damage_17, //0111 = 17
	Damage_20, //1000 = 20
	Damage_25, //1001 = 25
	Damage_30, //1010 = 30
	Damage_35, //1011 = 35
	Damage_40, //1100 = 40
	Damage_50, //1101 = 50
	Damage_75, //1110 = 75
	Damage_100 //1111 = 100
	};
	
	    								
typedef enum GunDamage tgun_damage; 	


//---------------------------------------------------------------------//

/*
union player_id_union {
tplayer_id bits;
uint8_t data [8];
};
*/

