#include <msp430.h>

#include <msp430x552x.h>
#include <stdio.h>
#include "lcd16.h"

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 	// Stop WDT

	ADC12CTL0 = ADC12SHT02 + ADC12ON;         	// Sampling time, ADC12 on
	ADC12CTL1 = ADC12SHP;                     	// Use sampling timer
	ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_12;	// Vr+=AVcc and Vr-=AVss; Select Channel A12

	ADC12CTL0 |= ADC12ENC;						// Enable conversion
	P7SEL |= 0x01;                            	// P7.0 ADC option select

	P6DIR = 0xFF;
	P6OUT = 0x00;
	lcdinit();
	prints("ICT2104 ADC DEMO");

/*
	// polling approach
	while (1) {
	  	  while ((P2IN & BIT0) == 1);
	  	  while ((P2IN & BIT0) == 0);
	  	  ADC12CTL0 |= ADC12SC;
	  	  while (!(ADC12IFG & BIT0));
	 	 printf("%d\n",ADC12MEM0);
	}
*/
	// interrupt approach
	P1DIR |= 0x01;                            // P1.0 output
	ADC12IE = 0x01;                           // Enable interrupt

	while (1)
	{
		ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

		__bis_SR_register(LPM0_bits + GIE);     // LPM0, ADC12_ISR will force exit
		__no_operation();                       // For debugger
	}
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
	float voltage;

	switch(__even_in_range(ADC12IV,34))
	{
	case  0: break;                           	// Vector  0:  No interrupt
	case  2: break;                           	// Vector  2:  ADC overflow
	case  4: break;                           	// Vector  4:  ADC timing overflow
	case  6:                                  	// Vector  6:  ADC12IFG0
		voltage = ADC12MEM0;
		voltage *= (3.3/4095);					// assume linear scale factor and ignore losses
		if (ADC12MEM0 >= 0x7ff) {             	// ADC12MEM = A0 > 0.5AVcc?
			P1OUT |= BIT0;                    	// P1.0 = 1
			printf("%d => %f\n", ADC12MEM0, voltage);
		}
		else {
			P1OUT &= ~BIT0;                  	// P1.0 = 0
			printf("%d => %f\n", ADC12MEM0, voltage);
		}
		gotoXy(0,1);
		prints("RAW = ");
		gotoXy(6,1);
		integerToLcd(ADC12MEM0);
		__bic_SR_register_on_exit(LPM0_bits);   // Exit active CPU

	// not used and for references
  	case  8: break;                           	// Vector  8:  ADC12IFG1
  	case 10: break;                           	// Vector 10:  ADC12IFG2
  	case 12: break;                           	// Vector 12:  ADC12IFG3
  	case 14: break;                           	// Vector 14:  ADC12IFG4
  	case 16: break;                           	// Vector 16:  ADC12IFG5
  	case 18: break;                           	// Vector 18:  ADC12IFG6
  	case 20: break;                           	// Vector 20:  ADC12IFG7
  	case 22: break;                           	// Vector 22:  ADC12IFG8
  	case 24: break;                           	// Vector 24:  ADC12IFG9
  	case 26: break;                           	// Vector 26:  ADC12IFG10
  	case 28: break;                           	// Vector 28:  ADC12IFG11
  	case 30: break;                           	// Vector 30:  ADC12IFG12
  	case 32: break;                           	// Vector 32:  ADC12IFG13
  	case 34: break;                           	// Vector 34:  ADC12IFG14
  	default: break;
	}
}
