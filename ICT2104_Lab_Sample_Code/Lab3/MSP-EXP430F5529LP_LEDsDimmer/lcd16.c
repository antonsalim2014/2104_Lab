#include "lcd16.h"

void lcdcmd(unsigned char cmd)
{
	P6OUT &= ~RS; 					// RS=0 to send command
	P6OUT &= ~EN;					// EN=0 to disable

	P6OUT &= 0xF0;					// Keep RS, RW and EN states and reset Data
	P6OUT |= (cmd >> 4);			// To send upper byte / 4-bits
	P6OUT |= EN;					// En=1 to enable
									// repeat to send lower byte
	P6OUT &= ~EN;					// EN=0 to disable
	P6OUT &= 0xF0;					// Keep RS, RW and EN states and reset Data
	P6OUT |= (cmd & 0x0F);			// To send lower byte / 4-bits
	P6OUT |= EN;					// EN=1 to enable

	P6OUT &= ~EN;					// EN=0 to disable
}

void lcdData(unsigned char data)
{
	P6OUT |= RS;  					// RS=1 to send data
	P6OUT &= ~EN;
	P6OUT &= 0xF0;
	P6OUT |= (data >> 4);			// To send upper byte / 4-bits
	P6OUT |= EN;					// EN=1 to enable

	P6OUT &= ~EN;
	P6OUT &=  0xF0;
	P6OUT |= (data & 0x0F);
	P6OUT |= EN;

	P6OUT &= ~EN;
}

void lcdinit(void)
{
	P6OUT &= ~RS;					// RS=0 to send cmd
	P6OUT &= ~EN;					// EN=0 to disable
	P6OUT |= 0x3;					// Wakeup (0x30)

	// 4-bits initialisation sequences
	P6OUT |=EN;						// enable
	P6OUT &=~EN;					// disable

	P6OUT |=EN;						// enable
	P6OUT &=~EN;					// disable

	P6OUT |=EN;						// enable
	P6OUT &=~EN;					// disable

	P6OUT &= 0xF2;					// clear (0x02) - Return Home (see page 18)

	P6OUT |=EN;
	P6OUT &=~EN;
	lcdcmd(0x28);   				// set data length 4 bit 2 line 5x8 dot (see page 19/20)

	lcdcmd(0x0C);   				// set Display On; Cursor On; (see page 19)

	lcdcmd(0x01); 					// Clear display (see page 18)

	lcdcmd(0x06);  					// Entry mode set - cursor shift direction to right (see page 18)

	lcdcmd(0x80);  					//set DDRAM address (see page 20)
	waitlcd(50);
}

void waitlcd(volatile unsigned int x)
{
	volatile unsigned int i;
	for (x;x>1;x--)
	{
		for (i=0;i<=100;i++);		// approximated software delay for 250ns
	}
}

void prints(char *str)
{
    while (*str)
    {
    	lcdData(*str);		// point to each character
    	str++;
    }
}

void gotoXy(unsigned char x, unsigned char y)
{
	if(x<40)
	{
		if(y)				// if not zero
			x |= 0x40;		// shift to 1st line (see page 19)
		x |=0x80;			// shift to left start position
		lcdcmd(x);
	}
}

void integerToLcd(unsigned int integer)
{
	unsigned char tenthousands,thousands,hundreds,tens,ones;

	tenthousands = integer / 10000;			// N---- or 0----
	lcdData(tenthousands + 0x30);

	thousands = integer % 10000 / 1000;		// xN--- or x0---
	lcdData(thousands + 0x30);

	hundreds = integer % 1000 / 100;		// xxN-- or xx0--
	lcdData(hundreds + 0x30);

	tens= integer % 100 / 10;				// xxxN- or xxx0-
	lcdData(tens + 0x30);

	ones=integer % 10;						// xxxxN or xxxx0
	lcdData(ones + 0x30);
}
