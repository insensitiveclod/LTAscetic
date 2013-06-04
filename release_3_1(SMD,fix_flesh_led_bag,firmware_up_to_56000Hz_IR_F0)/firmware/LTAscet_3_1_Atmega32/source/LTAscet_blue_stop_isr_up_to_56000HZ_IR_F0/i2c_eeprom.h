/* Note: the code used in this file seems to have been taken from http://nagits.wordpress.com/2010/12/18/avr_i2c_eeprom/
   It would be good to check copyright on this code.
   No english translation seems to exist at present: presenting back the english version might be the least I can do
   -- Arnd; 20130604
*/

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
//Initialize the Register of the bus control in Remote Box (FIXME: this translation makes no sense)
/*To start any data-transfer, it's required to create a so called 'start condition'. When at rest, the SCL and SDA lines of the bus are pulled HIGH. The master device (the AVR in our case) pulls the SDA line to LOW to indicate the start of a data-transfer.*/
 
/*
а)reset the TWINT interrupt flag (write a '1' into it) to enable a new data-transfer
b)Set 'Two Wire start' condition bit TWSTA
c)Set 'Two Wire Enable' bit TWEN
*/
        TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
 
//Ждем, пока шина даст добро (возможно, линия пока еще занята, ждем)
//TWINT бит устанавливается аппаратно, если TWI завершает текущее задание и ожидает реакции программы
        while(!(TWCR & (1<<TWINT)));
 
        /*Проверяем регистр статуса, а точнее биты TWS3-7,
        которые доступны только для чтения. Эти пять битов
        отражают состояние шины. TWS2-0 «отсекаем» с помощью операции «И         0xF8». Если TWS7-3 = 0x08, то СТАРТ был успешным.*/
        if((TWSR & 0xF8) != TW_START)
           return false;
 
/*К шине I2C может быть подключено множество подчиненных устройств (к примеру, много микросхем внешней памяти EEPROM). Для того, чтобы все микросхемы и контроллер знали, от кого и кому передается информация, в протоколе реализована Адресация ведомых устройств. В каждой микросхеме, предназначенной для работы с I2C, на заводе "зашит" определенный адрес. Мы этот адрес передаем по всей шине, т.е. всем ведомым. Каждый ведомый получает этот адрес и смотрит, типа мой это или чужой. Если мой, то О КРУТО, со мной хочет работать контроллер AVR. Так вот и происходит "рукопожатие" между ведущим и ведомым.*/
 
/*Так вот, мы хотим работать с микросхемой памяти 24LC64, поэтому по шине нам надо передать ее адрес. Она узнает свой адрес, и будет знать, что данные на запись адресуются именно ей. А остальные микросхемы, если они есть, эти данные будут просто игнорировать.*/
 
/*Постоянная часть адреса 24LC64 – 1010 (см. даташит на 24XX64), 3 бита - переменные (если вдруг мы захотим подключить несколько одинаковых микросхем c одинаковыми заводскими адресами, они пригодятся; в ином(нашем) случае выставляем нули), далее бит 0 - если хотим записывать в память или 1 - если читаем данные из памяти I2C EEPROM*/
 
       //TWDR = 0b1010‘000‘0;
        TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + (WRITEFLAG);
	 
/*Говорим регистру управления, что мы хотим передать данные, содержащиеся в регистре данных TWDR*/
        TWCR=(1<<TWINT)|(1<<TWEN);
	 
        //Ждем окончания передачи данных
        while(!(TWCR & (1<<TWINT)));
	 
	/*Если нет подтверждения от ведомого, делаем все по-новой (либо неполадки с линией, либо ведомого с таким адресом нет).
	Если же подтверждение поступило, то регистр статуса установит биты в 0x18=TW_MT_SLA_ACK (в случае записи) или 0x40=TW_MR_SLA_ACK (в случае чтения).
Грубо говоря, если TW_MT_SLA_ACK, то ведомый "говорит" нам, что его адрес как раз 1010’000 и он готов для записи (чтения, если TW_MR_SLA_ACK).*/
    }while((TWSR & 0xF8) != TW_MT_SLA_ACK);
 
/*Здесь можем уже уверенно говорить, что ведущий и ведомый друг друга видят и понимают. Вначале скажем нашей микросхеме памяти, по какому адресу мы хотим записать байт данных*/
 
/*****ПЕРЕДАЕМ АДРЕС ЗАПИСИ********/
	 
/*Записываем в регистр данных старший разряд адреса (адрес 16-битный, uint16_t))..*/
    TWDR=(address>>8);
 
    //..и передаем его
    TWCR=(1<<TWINT)|(1<<TWEN);
 
    //ждем окончания передачи
    while(!(TWCR & (1<<TWINT)));
	 
/*Проверяем регистр статуса, принял ли ведомый данные. Если ведомый данные принял, то он передает "Подтверждение", устанавливая SDA в низкий уровень. Блок управления, в свою очередь, принимает подтверждение, и записывает в регистр статуса 0x28= TW_MT_DATA_ACK. В противном случае 0x30= TW_MT_DATA_NACK */
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
	 
    //Далее тоже самое для младшего разряда адреса
    TWDR=(address);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
/*****ЗАПИСЫВАЕМ БАЙТ ДАННЫХ********/
 
    //Аналогично, как и передавали адрес, передаем байт данных
    TWDR=(data);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
    /*Устанавливаем условие завершения передачи данных (СТОП)
    (Устанавливаем бит условия СТОП)*/
    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
 
    //Ждем установки условия СТОП
    while(TWCR & (1<<TWSTO));
 
    return true;
}






uint8_t eeReadByte(uint16_t address)
	{
	    uint8_t data; //Переменная, в которую запишем прочитанный байт
	 
	//Точно такой же кусок кода, как и в eeWriteByte...
	/*****УСТАНАВЛИВАЕМ СВЯЗЬ С ВЕДОМЫМ********/
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
	 
	/*****ПЕРЕДАЕМ АДРЕС ЧТЕНИЯ********/
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
	 
	/*****ПЕРЕХОД В РЕЖИМ ЧТЕНИЯ********/
	/*Необходимо опять «связаться» с ведомым, т.к. ранее мы отсылали адресный пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + WRITEFLAG, чтобы записать адрес чтения байта данных. А теперь нужно перейти в режим чтения (мы же хотим прочитать байт данных), для этого отсылаем новый пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG.*/
	 
	    //Повтор условия начала передачи
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//ждем выполнения текущей операции
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. Условие повтора начала передачи (0x10=TW_REP_START) должно подтвердиться*/
	    if((TWSR & 0xF8) != TW_REP_START)
	        return false;
	 
	    /*Записываем адрес ведомого (7 битов) и в конце бит чтения (1)*/
	    //TWDR=0b1010’000’1;
	    TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG;        
	 
	//Отправляем..
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем, нашелся ли ведомый с адресом 1010’000 и готов ли он работать на чтение*/
	    if((TWSR & 0xF8) != TW_MR_SLA_ACK)
	        return false;
	 
	/*****СЧИТЫВАЕМ БАЙТ ДАННЫХ********/
	 
	/*Начинаем прием данных с помощью очистки флага прерывания TWINT. Читаемый байт записывается в регистр TWDR.*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //Ждем окончания приема..
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. По протоколу, прием данных должен оканчиваться без подтверждения со стороны ведущего (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*Присваиваем переменной data значение, считанное в регистр данных TWDR*/
	    data=TWDR;
	 
	    /*Устанавливаем условие завершения передачи данных (СТОП)*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //Ждем установки условия СТОП
	    while(TWCR & (1<<TWSTO));
	 
    //Возвращаем считанный байт
    return data;
}














bool eeWriteBytes(uint8_t *buffer, uint16_t address, uint16_t data_size)
{
 //volatile uint8_t data_tmp; //Переменная, в которую запишем прочитанный байт
/*****УСТАНАВЛИВАЕМ СВЯЗЬ С ВЕДОМЫМ********/
 
    do
    {
//Инициализация Регистра управления шиной в Блоке управления
/*Перед началом передачи данных необходимо сформировать т.н. условие начала. В состоянии покоя линии SCL и SDA находятся на высоком уровне. Ведущее устройство (Контроллер AVR в нашем примере), которое хочет начать передачу данных, изменяет состояние линии SDA к низкому уровню. Это и есть условие начала передачи данных.*/
 
/*
а)Сброс флага прерывания TWINT (Флаг TWINT сбрасывается программно путем записи в него логической 1) для разрешения начала новой передачи данных
б)Уст. бит условия СТАРТ
в)Уст. бит разрешения работы TWI
*/
        TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
 
//Ждем, пока шина даст добро (возможно, линия пока еще занята, ждем)
//TWINT бит устанавливается аппаратно, если TWI завершает текущее задание и ожидает реакции программы
        while(!(TWCR & (1<<TWINT)));
 
        /*Проверяем регистр статуса, а точнее биты TWS3-7,
        которые доступны только для чтения. Эти пять битов
        отражают состояние шины. TWS2-0 «отсекаем» с помощью операции «И         0xF8». Если TWS7-3 = 0x08, то СТАРТ был успешным.*/
        if((TWSR & 0xF8) != TW_START)
           return false;
 
/*К шине I2C может быть подключено множество подчиненных устройств (к примеру, много микросхем внешней памяти EEPROM). Для того, чтобы все микросхемы и контроллер знали, от кого и кому передается информация, в протоколе реализована Адресация ведомых устройств. В каждой микросхеме, предназначенной для работы с I2C, на заводе "зашит" определенный адрес. Мы этот адрес передаем по всей шине, т.е. всем ведомым. Каждый ведомый получает этот адрес и смотрит, типа мой это или чужой. Если мой, то О КРУТО, со мной хочет работать контроллер AVR. Так вот и происходит "рукопожатие" между ведущим и ведомым.*/
 
/*Так вот, мы хотим работать с микросхемой памяти 24LC64, поэтому по шине нам надо передать ее адрес. Она узнает свой адрес, и будет знать, что данные на запись адресуются именно ей. А остальные микросхемы, если они есть, эти данные будут просто игнорировать.*/
 
/*Постоянная часть адреса 24LC64 – 1010 (см. даташит на 24XX64), 3 бита - переменные (если вдруг мы захотим подключить несколько одинаковых микросхем c одинаковыми заводскими адресами, они пригодятся; в ином(нашем) случае выставляем нули), далее бит 0 - если хотим записывать в память или 1 - если читаем данные из памяти I2C EEPROM*/
 
       //TWDR = 0b1010‘000‘0;
        TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + (WRITEFLAG);
	 
/*Говорим регистру управления, что мы хотим передать данные, содержащиеся в регистре данных TWDR*/
        TWCR=(1<<TWINT)|(1<<TWEN);
	 
        //Ждем окончания передачи данных
        while(!(TWCR & (1<<TWINT)));
	 
	/*Если нет подтверждения от ведомого, делаем все по-новой (либо неполадки с линией, либо ведомого с таким адресом нет).
	Если же подтверждение поступило, то регистр статуса установит биты в 0x18=TW_MT_SLA_ACK (в случае записи) или 0x40=TW_MR_SLA_ACK (в случае чтения).
Грубо говоря, если TW_MT_SLA_ACK, то ведомый "говорит" нам, что его адрес как раз 1010’000 и он готов для записи (чтения, если TW_MR_SLA_ACK).*/
    }while((TWSR & 0xF8) != TW_MT_SLA_ACK);
 
/*Здесь можем уже уверенно говорить, что ведущий и ведомый друг друга видят и понимают. Вначале скажем нашей микросхеме памяти, по какому адресу мы хотим записать байт данных*/
 
/*****ПЕРЕДАЕМ АДРЕС ЗАПИСИ********/
	 
/*Записываем в регистр данных старший разряд адреса (адрес 16-битный, uint16_t))..*/
    TWDR=(address>>8);
 
    //..и передаем его
    TWCR=(1<<TWINT)|(1<<TWEN);
 
    //ждем окончания передачи
    while(!(TWCR & (1<<TWINT)));
	 
/*Проверяем регистр статуса, принял ли ведомый данные. Если ведомый данные принял, то он передает "Подтверждение", устанавливая SDA в низкий уровень. Блок управления, в свою очередь, принимает подтверждение, и записывает в регистр статуса 0x28= TW_MT_DATA_ACK. В противном случае 0x30= TW_MT_DATA_NACK */
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
	 
    //Далее тоже самое для младшего разряда адреса
    TWDR=(address);
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
 
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return false;
 
/*****ЗАПИСЫВАЕМ БАЙТЫ ДАННЫХ********/

    //Аналогично, как и передавали адрес, передаем байты данных

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

 
    /*Устанавливаем условие завершения передачи данных (СТОП)
    (Устанавливаем бит условия СТОП)*/
    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
 
    //Ждем установки условия СТОП
    while(TWCR & (1<<TWSTO));
 
    return true;
}

bool eeReadBytes(uint8_t *buffer, uint16_t address, uint16_t data_size)
	{
//	    uint8_t data; //Переменная, в которую запишем прочитанный байт
	 
	//Точно такой же кусок кода, как и в eeWriteByte...
	/*****УСТАНАВЛИВАЕМ СВЯЗЬ С ВЕДОМЫМ********/
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
	 
	/*****ПЕРЕДАЕМ АДРЕС ЧТЕНИЯ********/
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
	 
	/*****ПЕРЕХОД В РЕЖИМ ЧТЕНИЯ********/
	/*Необходимо опять «связаться» с ведомым, т.к. ранее мы отсылали адресный пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + WRITEFLAG, чтобы записать адрес чтения байта данных. А теперь нужно перейти в режим чтения (мы же хотим прочитать байт данных), для этого отсылаем новый пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG.*/
	 
	    //Повтор условия начала передачи
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//ждем выполнения текущей операции
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. Условие повтора начала передачи (0x10=TW_REP_START) должно подтвердиться*/
	    if((TWSR & 0xF8) != TW_REP_START)
	        return false;
	 
	    /*Записываем адрес ведомого (7 битов) и в конце бит чтения (1)*/
	    //TWDR=0b1010’000’1;
	    TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG;        
	 
	//Отправляем..
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем, нашелся ли ведомый с адресом 1010’000 и готов ли он работать на чтение*/
	    if((TWSR & 0xF8) != TW_MR_SLA_ACK)
	        return false;
	 
	/*****СЧИТЫВАЕМ БАЙТ ДАННЫХ********/
	 for(uint16_t i=0; i < (data_size - 1); i++)//считаем все байты, кроме последнего
	{

	/*Начинаем прием данных с помощью очистки флага прерывания TWINT. Читаемый байт записывается в регистр TWDR.*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	 
	    //Ждем окончания приема..
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. По протоколу, прием данных должен оканчиваться без подтверждения со стороны ведущего (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_ACK)
	        return false;
	 
	    /*Присваиваем переменной data значение, считанное в регистр данных TWDR*/
	    
		*buffer = TWDR;
		buffer++;
		//data=TWDR;
	}

	 /*Начинаем прием данных с помощью очистки флага прерывания TWINT. Читаемый байт записывается в регистр TWDR.*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //Ждем окончания приема..
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. По протоколу, прием данных должен оканчиваться без подтверждения со стороны ведущего (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*Присваиваем переменной data значение, считанное в регистр данных TWDR*/
	    
		*buffer = TWDR;


	    /*Устанавливаем условие завершения передачи данных (СТОП)*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //Ждем установки условия СТОП
	    while(TWCR & (1<<TWSTO));
	 
    //Возвращаем считанный байт
    return true;
}



bool open_eeprom( uint16_t address){
	//Точно такой же кусок кода, как и в eeWriteByte...
	/*****УСТАНАВЛИВАЕМ СВЯЗЬ С ВЕДОМЫМ********/
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
	 
	/*****ПЕРЕДАЕМ АДРЕС ЧТЕНИЯ********/
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
	 
	/*****ПЕРЕХОД В РЕЖИМ ЧТЕНИЯ********/
	/*Необходимо опять «связаться» с ведомым, т.к. ранее мы отсылали адресный пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + WRITEFLAG, чтобы записать адрес чтения байта данных. А теперь нужно перейти в режим чтения (мы же хотим прочитать байт данных), для этого отсылаем новый пакет (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG.*/
	 
	    //Повтор условия начала передачи
	    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//ждем выполнения текущей операции
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. Условие повтора начала передачи (0x10=TW_REP_START) должно подтвердиться*/
	    if((TWSR & 0xF8) != TW_REP_START)
	        return false;
	 
	    /*Записываем адрес ведомого (7 битов) и в конце бит чтения (1)*/
	    //TWDR=0b1010’000’1;
	    TWDR = (slaveAddressConst<<4) + (slaveAddressVar<<1) + READFLAG;        
	 
	//Отправляем..
	    TWCR=(1<<TWINT)|(1<<TWEN);
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем, нашелся ли ведомый с адресом 1010’000 и готов ли он работать на чтение*/
	    if((TWSR & 0xF8) != TW_MR_SLA_ACK)
	        return false;
		return true;

}


bool read_eeprom_byte(uint8_t *data){
/*Начинаем прием данных с помощью очистки флага прерывания TWINT. Читаемый байт записывается в регистр TWDR.*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	 
	    //Ждем окончания приема..
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. По протоколу, прием данных должен оканчиваться без подтверждения со стороны ведущего (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_ACK)
	        return false;
	 
	    /*Присваиваем переменной data значение, считанное в регистр данных TWDR*/
	    
		*data = TWDR;
		return true;
		//data=TWDR;


}

bool close_eeprom(uint8_t *data){
/*Начинаем прием данных с помощью очистки флага прерывания TWINT. Читаемый байт записывается в регистр TWDR.*/
	    TWCR=(1<<TWINT)|(1<<TWEN);
	 
	    //Ждем окончания приема..
	    while(!(TWCR & (1<<TWINT)));
	 
	/*Проверяем статус. По протоколу, прием данных должен оканчиваться без подтверждения со стороны ведущего (TW_MR_DATA_NACK = 0x58)*/
	    if((TWSR & 0xF8) != TW_MR_DATA_NACK)
	        return false;
	 
	    /*Присваиваем переменной data значение, считанное в регистр данных TWDR*/
	    
		*data = TWDR;


	    /*Устанавливаем условие завершения передачи данных (СТОП)*/
	    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	 
	    //Ждем установки условия СТОП
	    while(TWCR & (1<<TWSTO));
	 
    //Возвращаем считанный байт
    return true;
}

