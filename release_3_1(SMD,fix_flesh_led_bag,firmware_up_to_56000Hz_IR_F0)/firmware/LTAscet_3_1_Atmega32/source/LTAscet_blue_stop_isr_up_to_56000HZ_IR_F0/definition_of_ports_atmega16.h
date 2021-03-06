#define ADC_PORT PORTA
#define ADC_DDR DDRA
#define ADC_PIN (1<<7)
#define ADC_CHANNEL 7


#define IR_LED_PORT PORTC 
#define IR_LED_DDR DDRC 

#define IR_LED_HIGH_POWER_PORT PORTC 
#define IR_LED_HIGH_POWER_DDR DDRC 

#define IR_LED_LOW_POWER_PORT PORTC 
#define IR_LED_LOW_POWER_DDR DDRC 


#define LIFE_LED1_PORT PORTC
#define LIFE_LED1_DDR DDRC
#define LIFE_LED2_PORT PORTC
#define LIFE_LED2_DDR DDRC
#define LIFE_LED3_PORT PORTC
#define LIFE_LED3_DDR DDRC
#define LIFE_LED4_PORT PORTD
#define LIFE_LED4_DDR DDRD

#define FIRE_LED_PORT PORTC
#define FIRE_LED_DDR DDRC
#define BULLETS_OUT_LED_PORT PORTA
#define BULLETS_OUT_LED_DDR DDRA
#define SOUND_PORT PORTB
#define SOUND_DDR DDRB
#define TSOP_PORT PORTD
#define TSOP_DDR DDRD
#define TSOP_IN PIND
#define WOUND_LED_PORT PORTD
#define WOUND_LED_DDR DDRD
#define FIRE_KEY_PORT PORTB
#define FIRE_KEY_DDR DDRB
#define FIRE_KEY_IN PINB

#define RELOAD_KEY_PORT PORTB //This port is connected to the switch "DAMAGE" (done)
#define RELOAD_KEY_DDR DDRB 
#define RELOAD_KEY_IN PINB



#define FIRE_MODE_KEY_PORT PORTB
#define FIRE_MODE_KEY_DDR DDRB
#define FIRE_MODE_KEY_IN PINB

#define SOUND_CONTROL_PORT PORTB
#define SOUND_CONTROL_DDR DDRB


//#define IR_LED_PIN (1<<5)

#define IR_LED_HIGH_POWER_PIN (1<<6)

#define IR_LED_LOW_POWER_PIN (1<<7)


#define FIRE_LED_PIN (1<<5)
#define BULLETS_OUT_LED_PIN (1<<0)
#define LIFE_LED1_PIN (1<<3)
#define LIFE_LED2_PIN (1<<4)
#define LIFE_LED3_PIN (1<<2)
#define LIFE_LED4_PIN (1<<7)
#define WOUND_LED_PIN (1<<4)


#define SOUND_PIN (1<<3)
#define SOUND_CONTROL_PIN (1<<4)

#define TSOP_PIN (1<<2)

#define FIRE_KEY_PIN (1<<2)
#define RELOAD_KEY_PIN (1<<0)
#define FIRE_MODE_KEY_PIN (1<<1)

/*
#define SW_DAMAGE_PORT PORTB //This port is connected to the switch "DAMAGE" (Done)
#define SW_DAMAGE_DDR DDRB 
#define SW_DAMAGE_IN PINB
#define SW_DAMAGE_MASK ((1<<0)|(1<<1))
#define SW_DAMAGE_KEY1_PIN (1<<0)
#define SW_DAMAGE_KEY2_PIN (1<<1)


#define SW_TEAM_PORT PORTA //This port is connected to the switch "TEAM_ID" (team color)
#define SW_TEAM_DDR DDRA
#define SW_TEAM_IN PINA
#define SW_TEAM_MASK ((1<<0)|(1<<1))
#define SW_TEAM_KEY1_PIN (1<<0)
#define SW_TEAM_KEY2_PIN (1<<1)



#define SW_BULLETS_LIMIT_PORT PORTD //This port is connected to the switch "BULLETS_LIMIT" (limit of rounds)
#define SW_BULLETS_LIMIT_DDR DDRD
#define SW_BULLETS_LIMIT_IN PIND
#define SW_BULLETS_LIMIT_MASK ((1<<5)|(1<<6))
#define SW_BULLETS_LIMIT_KEY1_PIN (1<<5)
#define SW_BULLETS_LIMIT_KEY2_PIN (1<<6)
*/
