/*
 * MPU6050_Testing_02.c
 *
 * Created: 2018-07-16 오전 11:57:05
 * Author : juwon
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int T = 1;	// correction value

unsigned char Char_Raw_Data[12] = {'\0', };

unsigned int MPU6050_Raw_Now_Data[6] = {'\0', };
unsigned int MPU6050_Raw_Prior_Data[6] = {0, };

unsigned int MPU6050_Now_Data[6] = {'\0', };
unsigned int MPU6050_Prior_Data[6] = {0, };

void i2c_set_speed (unsigned int khz)
{
	if((khz < 50)||(khz > 400))
	khz = 100;
	TWSR = 0x00;
	TWBR = (8000000/1000-16*khz)/(2*khz);
}

void PORT_set(void)
{
	PORTD = 0x00;
	DDRD = (1 << PORTD7) | (1 << PORTD4) | (1 << PORTD3);
	DDRC = (1 << DDRC4) | (1 << DDRC5);
	PORTC = (1 << DDRC4) | (1 << DDRC5);
}

void Zyro_write(char addr, char dat)
{
	
	TWCR = 0xa4; //Start조건을 출력
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x08)));
	TWDR=0xD0;  //AD+W
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x18)));
	TWDR=addr;  //  RA
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x28)));
	TWDR=dat;
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x28)));
	TWCR = 0x94;
	_delay_us(50);
}


char Zyro_read(char addr)
{
	char dat;
	TWCR = 0xa4; //Start
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x08)));
	TWDR=0xD0;  //AD+W
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x18)));
	TWDR=addr;  //  RA
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x28)));
	
	TWCR=0xa4;   // restart (S)
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x10)));
	TWDR=0xD1;   // AD+R
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x40)));
	
	TWCR=0x84;
	
	while(((TWCR & 0x80) == 0x00 || ((TWSR & 0xf8)!=0x58))); //NO ACK
	dat = TWDR;
	TWCR = 0x94;
	
	return dat;
}

void USART_set(void)
{
	UBRRH = 0;
	UBRRL = 51;//9600
	UCSRA = 0x00;
	UCSRB = 0x98;
	UCSRC = 0x86;
}

unsigned char USART_Receive(void)
{
	while ( !(UCSRA & (1<<RXC)));
	return UDR;
}

void USART_Transmit(char daa)
{
	while (!(UCSRA & (1 << UDRE)));
	UDR = daa;
}

void USART_Transmit_STR(unsigned char *STR)
{
	while (*STR != 0)
	{
		USART_Transmit(*STR);
		++STR;
	}
}


int main(void)
{
	TWSR = 0x00;
	
	PORT_set();
	i2c_set_speed(400);
	USART_set();
	
	_delay_ms(50);
	
	Zyro_write(0x6b,0x00);
	Zyro_write(0x6c,0x00);
	Zyro_write(0x1b,0x10);
	Zyro_write(0x1c,0x10);
	
	while (1)
	{
		char Xgyro[15] = {'\0', }, Ygyro[15] = {'\0', }, Zgyro[15] = {'\0', }, Xacc[15] = {'\0', }, Yacc[15] = {'\0', }, Zacc[15] = {'\0', };
		
		// gytoscope data read
		Char_Raw_Data[0] = Zyro_read(0x43);
		Char_Raw_Data[1] = Zyro_read(0x44);
		Char_Raw_Data[2] = Zyro_read(0x45);
		Char_Raw_Data[3] = Zyro_read(0x46);
		Char_Raw_Data[4] = Zyro_read(0x47);
		Char_Raw_Data[5] = Zyro_read(0x48);
		
		// accelerometer data read
		Char_Raw_Data[6] = Zyro_read(0x3b);
		Char_Raw_Data[7] = Zyro_read(0x3c);
		Char_Raw_Data[8] = Zyro_read(0x3d);
		Char_Raw_Data[9] = Zyro_read(0x3e);
		Char_Raw_Data[10] = Zyro_read(0x3f);
		Char_Raw_Data[11] = Zyro_read(0x40);

		// comdine Hdata and Ldata
		MPU6050_Raw_Now_Data[0] = (Char_Raw_Data[0] << 8)|(Char_Raw_Data[1] << 0);	// gyroscope X raw data
		MPU6050_Raw_Now_Data[1] = (Char_Raw_Data[2] << 8)|(Char_Raw_Data[3] << 0);	// gyroscope Y raw data
		MPU6050_Raw_Now_Data[2] = (Char_Raw_Data[4] << 8)|(Char_Raw_Data[5] << 0);	// gyroscope Z raw data
		
		MPU6050_Raw_Now_Data[3] = (Char_Raw_Data[6] << 8)|(Char_Raw_Data[7] << 0);	// accelerometer X raw data
		MPU6050_Raw_Now_Data[4] = (Char_Raw_Data[8] << 8)|(Char_Raw_Data[9] << 0);	// accelerometer Y raw data
		MPU6050_Raw_Now_Data[5] = (Char_Raw_Data[10] << 8)|(Char_Raw_Data[11] << 0);// accelerometer Z raw data
		
		// correction data
		MPU6050_Now_Data[0] = T * MPU6050_Prior_Data[0] + T * (MPU6050_Raw_Now_Data[0] - MPU6050_Raw_Prior_Data[0]); // gyroscope X correction data
		MPU6050_Now_Data[1] = T * MPU6050_Prior_Data[1] + T * (MPU6050_Raw_Now_Data[1] - MPU6050_Raw_Prior_Data[1]); // gyroscope Y correction data
		MPU6050_Now_Data[2] = T * MPU6050_Prior_Data[2] + T * (MPU6050_Raw_Now_Data[2] - MPU6050_Raw_Prior_Data[2]); // gyroscope Z correction data
		
		MPU6050_Now_Data[3] = T * MPU6050_Raw_Now_Data[3] + (1 - T) * MPU6050_Prior_Data[3]; // accelerometer X correction data
		MPU6050_Now_Data[4] = T * MPU6050_Raw_Now_Data[4] + (1 - T) * MPU6050_Prior_Data[4]; // accelerometer Y correction data
		MPU6050_Now_Data[5] = T * MPU6050_Raw_Now_Data[5] + (1 - T) * MPU6050_Prior_Data[5]; // accelerometer Z correction data
		
		
		sprintf(Xgyro, "%d", MPU6050_Now_Data[0]);
		sprintf(Ygyro, "%d", MPU6050_Now_Data[1]);
		sprintf(Zgyro, "%d", MPU6050_Now_Data[2]);
		sprintf(Xacc, "%d", MPU6050_Now_Data[3]);
		sprintf(Yacc, "%d", MPU6050_Now_Data[4]);
		sprintf(Zacc, "%d", MPU6050_Now_Data[5]);
		
		USART_Transmit_STR("Gyro X : ");
		USART_Transmit_STR(Xgyro);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Gyro Y : ");
		USART_Transmit_STR(Ygyro);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Gyro Z : ");
		USART_Transmit_STR(Zgyro);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel X : ");
		USART_Transmit_STR(Xacc);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel Y : ");
		USART_Transmit_STR(Yacc);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel Z : ");
		USART_Transmit_STR(Zacc);
		USART_Transmit_STR("\r\n");
		
		
		for(int i = 0; i <= 5; i++)
		{
			MPU6050_Raw_Prior_Data[i] = MPU6050_Raw_Now_Data[i];
			MPU6050_Prior_Data[i] = MPU6050_Now_Data[i];
		}
	}
}

