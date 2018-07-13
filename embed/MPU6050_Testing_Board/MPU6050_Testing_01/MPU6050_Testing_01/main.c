/*
 * MPU6050_Testing_01.c
 *
 * Created: 2018-07-13 오후 5:31:21
 * Author : juwon
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


unsigned int g_X = 0;
unsigned int g_Y = 0;
unsigned int g_Z = 0;
unsigned int a_X = 0;
unsigned int a_Y = 0;
unsigned int a_Z = 0;

char c_g_X[], c_g_Y[], c_g_Z[], c_a_X[], c_a_Y[], c_a_Z[];

void i2c_set_speed (unsigned int khz)
{
	if((khz < 50)||(khz > 400))
	khz = 100;
	TWSR = 0x00;
	TWBR = (F_CPU/1000-16*khz)/(2*khz);
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
	UCSRB = 0x18;
	UCSRC = 0x86;
}

void puta(char daa)
{
	while (!(UCSRA & (1 << UDRE)));
	UDR = daa;
}

void USART_Transmit_STR(unsigned char *STR)
{
	while (*STR != 0)
	{
		puta(*STR);
		++STR;
	}
}


int main(void)
{
	char dat0, dat1,dat2, dat3,dat4, dat5,dat6, dat7,dat8, dat9,dat10, dat11;


	PORT_set();
	i2c_set_speed(400);
	USART_set();

	USART_Transmit_STR("AT+BAUD7\r\n");

	Zyro_write(0x6b,0x00);
	Zyro_write(0x6c,0x00);
	Zyro_write(0x1b,0x10);
	Zyro_write(0x1c,0x10);

	while (1)
	{

		dat0=Zyro_read(0x43);
		dat1=Zyro_read(0x44);
		dat2=Zyro_read(0x45);
		dat3=Zyro_read(0x46);
		dat4=Zyro_read(0x47);
		dat5=Zyro_read(0x48);
		
		
		dat6=Zyro_read(0x3b);
		dat7=Zyro_read(0x3c);
		dat8=Zyro_read(0x3d);
		dat9=Zyro_read(0x3e);
		dat10=Zyro_read(0x3f);
		dat11=Zyro_read(0x40);


		g_X = (dat0 << 8)|(dat1 << 0);
		g_Y = (dat2 << 8)|(dat3 << 0);
		g_Z = (dat4 << 8)|(dat5 << 0);
		a_X = (dat6 << 8)|(dat7 << 0);
		a_Y = (dat8 << 8)|(dat9 << 0);
		a_Z = (dat10 << 8)|(dat11 << 0);
		
		sprintf(c_g_X, "%d", g_X);
		sprintf(c_g_Y, "%d", g_Y);
		sprintf(c_g_Z, "%d", g_Z);
		sprintf(c_a_X, "%d", a_X);
		sprintf(c_a_Y, "%d", a_Y);
		sprintf(c_a_Z, "%d", a_Z);
		
		USART_Transmit_STR("Gyro X : ");
		USART_Transmit_STR(c_g_X);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Gyro Y : ");
		USART_Transmit_STR(c_g_Y);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Gyro Z : ");
		USART_Transmit_STR(c_g_Z);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel X : ");
		USART_Transmit_STR(c_a_X);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel Y : ");
		USART_Transmit_STR(c_a_Y);
		USART_Transmit_STR("\r\n");
		
		USART_Transmit_STR("Accel Z : ");
		USART_Transmit_STR(c_a_Z);
		USART_Transmit_STR("\r\n");
		
	}
}


