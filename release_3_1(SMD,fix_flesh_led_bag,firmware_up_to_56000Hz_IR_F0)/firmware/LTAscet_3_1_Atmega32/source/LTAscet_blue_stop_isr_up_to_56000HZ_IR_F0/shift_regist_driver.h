#define SHIFT_REGISTER_DS_PORT PORTB 	//Port connected to DS-line (data) of shift-register
#define SHIFT_REGISTER_SH_CP_PORT PORTB //Port connected to SH_CP (clock) of shift-register
#define SHIFT_REGISTER_ST_CP_PORT PORTB //Port connected to ST_CP (latch) of shift register

#define SHIFT_REGISTER_DS_DDR DDRB 	//control_register associated with DS-line (data) of shift-register
#define SHIFT_REGISTER_SH_CP_DDR DDRB 	//control_register associated with SH_CP (clock) of shift-register
#define SHIFT_REGISTER_ST_CP_DDR DDRB 	//control_register associated with ST_CP (latch) of shift-register

#define SHIFT_REGISTER_DS_PIN (1<<5) 	//pin of output port connected to DS-line (data) of shift-register
#define SHIFT_REGISTER_SH_CP_PIN (1<<7) //pin of output port connected to SH_CP (clock) of shift-register
#define SHIFT_REGISTER_ST_CP_PIN (1<<6) //pin of output port connected to ST_CP (latch) of shift-register




//Do not edit below! Override pins and ports above!

#define SIHFT_REGISTER_DS_ON SHIFT_REGISTER_DS_PORT|=SHIFT_REGISTER_DS_PIN 	//Put a '1' on DS (FIXME: define-name typo!)
#define SIHFT_REGISTER_DS_OFF SHIFT_REGISTER_DS_PORT&=~SHIFT_REGISTER_DS_PIN 	//Put a '0' on DS

#define SHIFT_REGISTER_SH_CP_ON SHIFT_REGISTER_SH_CP_PORT|=SHIFT_REGISTER_SH_CP_PIN	//Put a '1' on SH_CP
#define SHIFT_REGISTER_SH_CP_OFF SHIFT_REGISTER_SH_CP_PORT&=~SHIFT_REGISTER_SH_CP_PIN	//Put a '0' on SH_CP

#define SHIFT_REGISTER_ST_CP_ON SHIFT_REGISTER_ST_CP_PORT|=SHIFT_REGISTER_ST_CP_PIN 	//Put a '1' on ST_CP
#define SHIFT_REGISTER_ST_CP_OFF SHIFT_REGISTER_ST_CP_PORT&=~SHIFT_REGISTER_ST_CP_PIN	//Put a '0' on ST_CP 


//Set up all pins used for connecting to the shift-register as 'OUTPUT's
void init_shift_register(void){ //
	SHIFT_REGISTER_DS_DDR|=SHIFT_REGISTER_DS_PIN;
	SHIFT_REGISTER_SH_CP_DDR|= SHIFT_REGISTER_SH_CP_PIN;
	SHIFT_REGISTER_ST_CP_DDR|= SHIFT_REGISTER_ST_CP_PIN;

	SIHFT_REGISTER_DS_OFF;
	SHIFT_REGISTER_SH_CP_OFF;
	SHIFT_REGISTER_ST_CP_OFF;
};

void shift_register_set_data(volatile uint8_t shift_data){
	volatile uint8_t mask;
	mask = 0b10000000;
	for (int i=0; i<8; i++)	
	{
		if ((mask&shift_data) ==0) 
		{SIHFT_REGISTER_DS_OFF;}	
//		if ((mask&shift_data)!=0) 
//		{SIHFT_REGISTER_DS_ON;}	
		
		else 
		{SIHFT_REGISTER_DS_ON;}
		SHIFT_REGISTER_SH_CP_ON;
//--//		asm("nop");
		SHIFT_REGISTER_SH_CP_OFF;
		mask = (mask>>1);
	}
	
	SHIFT_REGISTER_ST_CP_OFF;
//--//	asm("nop");
	SHIFT_REGISTER_ST_CP_ON;
}


void shift_register_clean(void){
	shift_register_set_data(0);
}
