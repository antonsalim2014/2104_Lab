#include "lcd16.h"

void lcdcmd(unsigned char cmd)
{
	P6OUT &= ~RS; 	// RS=0 to send command
	P6OUT &= ~EN;

	P6OUT &= 0xF0;
	P6OUT |= ((cmd >> 4) & 0x0F);
	P6OUT |= EN;
	waitlcd(2);

	P6OUT &= ~EN;
	P6OUT &= 0xF0;
	P6OUT |= (cmd & 0x0F);
	P6OUT |= EN;
	waitlcd(2);

	P6OUT &= ~EN;
}

void lcdData(unsigned char data)
{
	P6OUT |= RS;  	// RS=1 to send data
	P6OUT &= ~EN;
	P6OUT &= 0xF0;
	P6OUT |= ((data >> 4) & 0x0F);
	P6OUT |= EN;	// EN=1
	waitlcd(2);

	P6OUT &= ~EN;
	P6OUT &=  0xF0;
	P6OUT |= (data & 0x0F);
	P6OUT |= EN;
	waitlcd(2);

	P6OUT &= ~EN;
}

void lcdinit(void)
{
	P6OUT &= ~RS;	// RS=0 to send cmd
	P6OUT &= ~EN;
	P6OUT |= 0x3;
	waitlcd(40);

	P6OUT  |=EN;
	P6OUT  &=~EN;
	waitlcd(5);

	P6OUT  |=EN;
	P6OUT  &=~EN;
	waitlcd(5);

	P6OUT  |=EN;
	P6OUT  &=~EN;
	waitlcd(2);

	P6OUT &= 0xF2;
	P6OUT  |=EN;
	P6OUT  &=~EN;
	lcdcmd(0x28);   //set data length 4 bit 2 line
	waitlcd(250);

	lcdcmd(0x0E);   // set display on cursor on blink on
	waitlcd(250);

	lcdcmd(0x01); // clear lcd
	waitlcd(250);

	lcdcmd(0x06);  // cursor shift direction
	waitlcd(250);

	lcdcmd(0x80);  //set ram address
	waitlcd(250);
}

void waitlcd(volatile unsigned int x)
{
	volatile unsigned int i;
	for (x;x>1;x--)
	{
		for (i=0;i<=110;i++);
	}
}

void prints(char *str)
{
    while (*str)
    {
    	lcdData(*str);
    	str++;
    }
}

void gotoXy(unsigned char x, unsigned char y)
{
	if(x<40)
	{
		if(y) x |= 0x40;
		x |=0x80;
		lcdcmd(x);
	}
}

void integerToLcd(unsigned int integer)
{
	unsigned char tenthousands,thousands,hundreds,tens,ones;

	tenthousands = integer / 10000;
	lcdData(tenthousands + 0x30);

	thousands = integer % 10000 / 1000;
	lcdData(thousands + 0x30);

	hundreds = integer % 1000 / 100;
	lcdData(hundreds + 0x30);

	tens= integer % 100 / 10;
	lcdData(tens + 0x30);

	ones=integer % 10;
	lcdData(ones + 0x30);
}
