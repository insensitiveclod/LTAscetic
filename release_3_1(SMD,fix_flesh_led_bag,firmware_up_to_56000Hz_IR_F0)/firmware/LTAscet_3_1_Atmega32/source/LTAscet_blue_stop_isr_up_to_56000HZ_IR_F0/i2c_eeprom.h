#include <avr/io.h>        /* Imports the names for the inputs in PORTA	*/
#include <util/delay.h>    /* Supplies delay-routines			*/
#include <avr/interrupt.h> /* We will use the interrupt handling	*/
#include <avr/pgmspace.h>  // We define constants in program-memory

#ifndef bool
                        #define bool unsigned char
                        #define true 1
                        #define false 0
#endif


//#define slaveF_SCL 100000		//100 Khz
#define slaveF_SCL 400000		//400 Khz
	 
#define slaveAddressConst 0b1010 	//Constant of slave-address
#define slaveAddressVar 0b000		//Variable offset of slave-address
 
//Byte data direction
#define READFLAG 1			//Read
#define WRITEFLAG 0			//Write
	 
void eeInit(void);					//Setup Two-Wire-Interface (TWI)
uint8_t eeWriteByte(uint16_t address,uint8_t data);	//Writes a byte into the EEPROM
uint8_t eeReadByte(uint16_t address);			//Read a byte from the EEPROM
bool eeReadBytes(uint8_t *buffer, uint16_t address, uint16_t data_size);
bool eeWriteBytes(uint8_t *buffer, uint16_t address, uint16_t data_size);
 
// TWSR values (not bits)
// (taken from avr-libc twi.h - thank you Marek Michalkiewicz)
// Master
#define TW_START                    0x08
#define TW_REP_START                0x10
// Master Transmitter
#define TW_MT_SLA_ACK               0x18
#define TW_MT_SLA_NACK              0x20
#define TW_MT_DATA_ACK              0x28
#define TW_MT_DATA_NACK             0x30
#define TW_MT_ARB_LOST              0x38
// Master Receiver
#define TW_MR_ARB_LOST              0x38
#define TW_MR_SLA_ACK               0x40
#define TW_MR_SLA_NACK              0x48
#define TW_MR_DATA_ACK              0x50
#define TW_MR_DATA_NACK             0x58
// Slave Transmitter
#define TW_ST_SLA_ACK               0xA8
#define TW_ST_ARB_LOST_SLA_ACK      0xB0
#define TW_ST_DATA_ACK              0xB8
#define TW_ST_DATA_NACK             0xC0
#define TW_ST_LAST_DATA             0xC8
// Slave Receiver
#define TW_SR_SLA_ACK               0x60
#define TW_SR_ARB_LOST_SLA_ACK      0x68
#define TW_SR_GCALL_ACK             0x70
#define TW_SR_ARB_LOST_GCALL_ACK    0x78
#define TW_SR_DATA_ACK              0x80
#define TW_SR_DATA_NACK             0x88
#define TW_SR_GCALL_DATA_ACK        0x90
#define TW_SR_GCALL_DATA_NACK       0x98
#define TW_SR_STOP                  0xA0
// Misc
#define TW_NO_INFO                  0xF8
#define TW_BUS_ERROR                0x00

void eeInit(){ 				//Sets up TWI bus-speed for EEPROM access

/*Set up TwoWire bus-rate*/
TWBR = (F_CPU/slaveF_SCL - 16)/(2 * /* TWI_Prescaler= 4^TWPS */1);
/*
If device's TWI operates in 'master' mode , the value of TWBR should be at least 10. If the value is less , operation as a bus-master might fail due to incorrectly generated SDA and SCL waveforms during transmission of a byte.
*/
    if(TWBR < 10)
        TWBR = 10;
 
/*
Setting prescaler value in the status register
Bits TWPS0 and TWPS1 are cleared; setting the value of the prescaler to '1'
 */
    TWSR &= (~((1<<TWPS1)|(1<<TWPS0)));
}



uint8_t eeWriteByte(uint16_t address,uint8_t data)
{
 
/*****Establish a connection to memory; writing just one byte********/
 
    do
    {
//Initialize the control-Register of the TWI-bus (TWCR)
/*To start any data-transfer, it's required to create a so called 'start condition'. When at rest, the SCL and SDA lines of the bus are pulled HIGH. The master device (the AVR in our case) pulls the SDA line to LOW to indicate the start of a data-transfer.*/
 
/*
а)reset the TWINT interrupt flag (write a '1' into it) to enable a new data-transfer
b)Set 'Two Wire start' condition bit TWSTA
c)Set 'Two Wire Enable' bit TWEN
*/
        TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
 
//Wait until register has been updated (the line may still be busy; so we we wait)
//TWINT bit is set by hardware when TWI transfer completes.
        while(!(TWCR & (1<<TWINT)));
 
        /*Now we check bits 3-7 of the status-register which are read-only
        These bits reflect the state of the bus.
        TWS2-0 are not read (by using "TWSR & 0xF8). If TWS7-3 = 0x08, then the TWI start was successful.*/
        if((TWSR & 0xF8) != TW_START)
           return false;
 
/*The TWI (i2c) bus can have multiple slave devices connected. (for example, multiple external I2C EEPROMS). To be able to specify which chip to read from, the protocol
implements an addressing-scheme. Each (slave) chip on the bus has it's own address that is set in the factory (but can sometimes be adjusted/altered by connecting certain pins to GND/VCC; check docs). To specify which slave we want to listen to our commands, we first send the address over the bus. All slaves listen to the bus but only one of them goes 'COOL! This is my address! I'd love to talk to you!' and will start a 'handshake' back to the master (AVR)*/
 
/*Now, we want to work with the EEPROM-memory chip 24LC64,so we need to send the address for this chip onto the bus. The chip will see the address and will know that the following data on the bus is addressed to it. The other slaves on the bus (if there are any) will simply ignore the data not meant for them.*/
 
/*The 'fixed' part of the address for the is 24LC64 – 1010 (see datasheet for the 24XX64) and 3 'variable' bits. If we would like to connect multiple similar chips with the same 'fixed'-part, we can assign alternate addresses in this space. This is done by shorting certain pins on the EEPROM-chip to GND or VCC. After the fixed+variable bits comes an end-bit that indicates if you want to write (0) or want to read (1) from the i2c EEPROM*/
 
       //TWDR = 0b1010‘000‘0;
        TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + (WRITEFLAG);
	 
/*Update the CONTROL REGISTER (TWCR) to tell the TWI-controller that we have set data ready in TWDR and want it sent onto the bus*/
        TWCR=(1<<TWINT)|(1<<TWEN);
	 
        //The TWCR register will have the TWINT bit set when it's done
        while(!(TWCR & (1<<TWINT)));
	 
	/*If there is no acknowledgement from the slave, we have to do it all over again. Perhaps there's a problem with the wires or
	there is no slave on the bus with the right address.
	If a confirmation (handshake) was received, the STATUS REGISTER would be set to 0x18=TW_MT_SLA_ACK (in the case of writing)
	or 0x40=TW_MR_SLA_ACK (in the case of reading).
	Roughly speaking, if we see TW_MT_SLA_ACK, it tells us that 1010’000 is the slave's address and it's ready for writing to (Or reading, if it's TW_MR_SLA_ACK).*/
    }while((TWSR & 0xF8) != TW_MT_SLA_ACK);
 
/*Here we can already say with confidence that both master and slave see eachother correctly and understand eachother. So now we can tell the memory-chip to which address inside of it's memory we want to write a byte of data*/
 
/*****Pass the WRITE address********/
	 
/*Write the data in the register (MSB) (address 16-bits, uint16_t))..*/
    TWDR=(address>>8);
 
    //..and send it
    TWCR=(1<<TWINT)|(1<<TWEN);
 
    //and wait for the transmission to complete...
    while(!(TWCR & (1<<TWINT)));
	 
/*Check the STATUS REGISTER to see if the slave took the data. If it received it all, it will send an ACKnowledgement by setting the SDA-line of the bus to LOW. The TWI-bus control-unit will see this and update the STATUS REGISTER. A good ACK is 0x28= TW_MT_DATA_ACK. If we get a NotAcknowledge: 0x30= TW_MT_DATA_NACK */
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
	 
    //Then we do the same for the rest of the address; the LSB
    TWDR=(address);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
/*****Writing the data********/
 
    //Just like how we sent the address, we push the data-byte onto the bus.
    TWDR=(data);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
    /*And we tell the bus we're done sending (STOP)
    (Set the STOP-condition bit in the CONTROL REGISTER)*/
    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
 
    //Wait until the CONTROL REGISTER has updated the STOP-bit
    while(TWCR & (1<<TWSTO));
 
    return true;
}






uint8_t eeReadByte(uint16_t address)
	{
	    uint8_t data; //variable that'll contain the byte we will read from 'address'
	 
	//The same piece of code as eeWriteByte...
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
	/*This is required to 'contact' the slave. First we sent out the chip address (slaveAddressConst << 4) + (slaveAddressVar << 1) + WRITEFLAG to tell the chip which address we want to read a data-byte from. Now we switch to read (because we want to read that byte). This by writing '(slaveAddressConst << 4) + (slaveAddressVar << 1) + READFLAG to the bus.*/
	 
	    //We again initialize the bus with a 'repeated start condition'
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//... and wait for the operation to finish
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; the 'repeated start condition' should be detected. (0x10 = TW_REP_START)*/
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
	 
	/*Start data-reception by clearing the interrupt flag TWINT. TWDR will contain the byte that is read.*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //Wait for reception to finish...
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*Take the byte out of the i2c data-register (TWDR) and stick it into a variable.*/
	    data=TWDR;
	 
	    /*Update the STATUS REGISTER to set a STOP-bit*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //Wait until CONTROL REGISTER has been updated
	    while(TWCR & (1<<TWSTO));
	 
    //And we return the byte we've read.
    return data;
}



bool eeWriteBytes(uint8_t *buffer, uint16_t address, uint16_t data_size)
{
 //volatile uint8_t data_tmp; //Переменная, в которую запишем прочитанный байт
/*****Establish a connection to memory********/
 
    do
    {
//Initialize the control-Register of the TWI-bus (TWCR)
/*To start any data-transfer, it's required to create a so called 'start condition'. When at rest, the SCL and SDA lines of the bus are pulled HIGH. The master device (the AVR in our case) pulls the SDA line to LOW to indicate the start of a data-transfer.*/
 
/*
а)reset the TWINT interrupt flag (write a '1' into it) to enable a new data-transfer
b)Set 'Two Wire start' condition bit TWSTA
c)Set 'Two Wire Enable' bit TWEN
*/
        TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
 
//Wait until register has been updated (the line may still be busy; so we we wait)
//TWINT bit is set by hardware when TWI transfer completes.
        while(!(TWCR & (1<<TWINT)));
 
        /*Now we check bits 3-7 of the status-register which are read-only
        These bits reflect the state of the bus.
        TWS2-0 are not read (by using "TWSR & 0xF8). If TWS7-3 = 0x08, then the TWI start was successful.*/
        if((TWSR & 0xF8) != TW_START)
           return false;
 
/*The TWI (i2c) bus can have multiple slave devices connected. (for example, multiple external I2C EEPROMS). To be able to specify which chip to read from, the protocol
implements an addressing-scheme. Each (slave) chip on the bus has it's own address that is set in the factory (but can sometimes be adjusted/altered by connecting certain pins to GND/VCC; check docs). To specify which slave we want to listen to our commands, we first send the address over the bus. All slaves listen to the bus but only one of them goes 'COOL! This is my address! I'd love to talk to you!' and will start a 'handshake' back to the master (AVR)*/
 
/*Now, we want to work with the EEPROM-memory chip 24LC64,so we need to send the address for this chip onto the bus. The chip will see the address and will know that the following data on the bus is addressed to it. The other slaves on the bus (if there are any) will simply ignore the data not meant for them.*/
 
/*The 'fixed' part of the address for the is 24LC64 – 1010 (see datasheet for the 24XX64) and 3 'variable' bits. If we would like to connect multiple similar chips with the same 'fixed'-part, we can assign alternate addresses in this space. This is done by shorting certain pins on the EEPROM-chip to GND or VCC. After the fixed+variable bits comes an end-bit that indicates if you want to write (0) or want to read (1) from the i2c EEPROM*/
 
       //TWDR = 0b1010‘000‘0;
        TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + (WRITEFLAG);
	 
/*Update the CONTROL REGISTER (TWCR) to tell the TWI-controller that we have set data ready in TWDR and want it sent onto the bus*/
        TWCR=(1<<TWINT)|(1<<TWEN);
	 
        //The TWCR register will have the TWINT bit set when it's done
        while(!(TWCR & (1<<TWINT)));
	 
	/*If there is no acknowledgement from the slave, we have to do it all over again. Perhaps there's a problem with the wires or
	there is no slave on the bus with the right address.
	If a confirmation (handshake) was received, the STATUS REGISTER would be set to 0x18=TW_MT_SLA_ACK (in the case of writing)
	or 0x40=TW_MR_SLA_ACK (in the case of reading).
	Roughly speaking, if we see TW_MT_SLA_ACK, it tells us that 1010’000 is the slave's address and it's ready for writing to (Or reading, if it's TW_MR_SLA_ACK).*/
    }while((TWSR & 0xF8) != TW_MT_SLA_ACK);
 
/*Here we can already say with confidence that both master and slave see eachother correctly and understand eachother. So now we can tell the memory-chip to which address inside of it's memory we want to write a byte of data*/
 
/*****Pass the WRITE address********/
	 
/*Write the data in the register (MSB) (address 16-bits, uint16_t))..*/
    TWDR=(address>>8);
 
    //..and send it
    TWCR=(1<<TWINT)|(1<<TWEN);
 
    //and wait for the transmission to complete...
    while(!(TWCR & (1<<TWINT)));
	 
/*Check the STATUS REGISTER to see if the slave took the data. If it received it all, it will send an ACKnowledgement by setting the SDA-line of the bus to LOW. The TWI-bus control-unit will see this and update the STATUS REGISTER. A good ACK is 0x28= TW_MT_DATA_ACK. If we get a NotAcknowledge: 0x30= TW_MT_DATA_NACK */
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
	 
    //Then we do the same for the rest of the address; the LSB
    TWDR=(address);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
/*****Writing the data********/

    //Just like we sent the address, we start sending bytes

//sound_size = sizeof(pSnd);
for(uint16_t iii=0; iii < data_size; iii++)
	{
        
//		data_tmp = pgm_read_byte(&pSnd[iii]);
		//data_tmp = iii;

		TWDR=(*buffer);
		buffer++;

        TWCR=(1<<TWINT)|(1<<TWEN);
        while(!(TWCR & (1<<TWINT)));
 
        if((TWSR & 0xF8) != TW_MT_DATA_ACK)
            return false;
	}

 
    /*When we're done, we set the STOP condition
    (set the TWSTO bit (STOP)*/
    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
 
    //We wait until the CONTROL REGISTER has been updated
    while(TWCR & (1<<TWSTO));
 
    return true;
}

bool eeReadBytes(uint8_t *buffer, uint16_t address, uint16_t data_size)
	{
//	    uint8_t data; //variable we want the data to be be read into..
	 
	//The same piece of code as a eeWriteByte ...
	/*****Establish a link with the EEPROM********/
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
	/*This is required to 'contact' the slave. First we sent out the chip address (slaveAddressConst << 4) + (slaveAddressVar << 1) + WRITEFLAG to tell the chip which address we want to read a data-byte from. Now we switch to read (because we want to read that byte). This by writing '(slaveAddressConst << 4) + (slaveAddressVar << 1) + READFLAG to the bus.*/
	 
	    //We again initialize the bus with a 'repeated start condition'
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//... and wait for the operation to finish
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; the 'repeated start condition' should be detected. (0x10 = TW_REP_START)*/
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
	 
	/*****Read multple bytes of data********/
	 for(uint16_t i=0; i < (data_size - 1); i++)//Read all except the last byte
	{

	/*Start data-reception by clearing the interrupt flag TWINT. TWDR will contain the byte that is read.*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	 
	    //We wait until CONTROL REGISTER has been updated
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_ACK)
	        return false;
	 
	    /*We read the data from TWDR and add it to our buffer*/
	    
		*buffer = TWDR;
		buffer++;
		//data=TWDR;
	}

	 /*Start data reception by clearing the interrupt-flag TWINT. The byte read from EEPROM will be put into TWDR*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //We wait until the CONTROL REGISTER has been updated
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Check status. According to the protocol, data reception must end without confirmation from the master (TW_MR_DATA_NACK = 0x58) */
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*REad the next byte and add it to the buffer.*/
	    
		*buffer = TWDR;


	    /*Set the STOP condition on the bus (update CONTROL REGISTER with TWSTO)*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //... and wait until it's set.
	    while(TWCR & (1<<TWSTO));
	 
    //And we exit; with the data written to the address that *buffer pointed to
    return true;
}



bool open_eeprom( uint16_t address){
	//The same piece of code as in eeWriteByte...
	/*****Establish a connection to the EEPROM********/
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
	 
	/*****pass the ADDRESS of the device********/
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
	/*This is required to 'contact' the slave. First we sent out the chip address (slaveAddressConst << 4) + (slaveAddressVar << 1) + WRITEFLAG to tell the chip which address we want to read a data-byte from. Now we switch to read (because we want to read that byte). This by writing '(slaveAddressConst << 4) + (slaveAddressVar << 1) + READFLAG to the bus.*/
	 
	    //We again initialize the bus with a 'repeated start condition'
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//... and wait for the operation to finish
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; the 'repeated start condition' should be detected. (0x10 = TW_REP_START)*/
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
		return true;

}


bool read_eeprom_byte(uint8_t *data){
/*Start reception of data by clearing the interrupt flag TWINT in the CONTROL REGISTER. The returned byte will be put into TWDR*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	 
	    //Wait for the CONTROL REGISTER to finish updating
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_ACK)
	        return false;
	 
	    /*Take the byte out of the i2c data-register (TWDR) and put it in the address that *data points to*/
	    
		*data = TWDR;
		return true;
		//data=TWDR;


}

bool close_eeprom(uint8_t *data){
/*We want to close the EEPROM. To do so we need to initliaze data-transfer by clearing the interrupt flag. We do this by setting the TWINT and TWEN flags in the CONTROL REGISTER*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //... and we wait for the CONTROL REGISTER to have been updated
	    while(!(TWCR & (1<<TWINT)));
	 
	/*We check the bus status; according to specs, the status-register (TWSR) should indicate a NACK from the master (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*Take the byte out of the i2c data-register (TWDR) and put it in the address that *data points to*/
	    
		*data = TWDR;


	    /*Update the STATUS REGISTER to set a STOP-bit (TWSTO)*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //and wait for the CONTROL REGISTER to have been updated
	    while(TWCR & (1<<TWSTO));
	 
    //Here we return with the data put in the address *data
    return true;
}

