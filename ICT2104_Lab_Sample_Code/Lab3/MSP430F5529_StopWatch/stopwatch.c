#include <msp430.h>
#include "lcd16.h"

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	P6DIR = 0xFF;
	P6OUT = 0x00;

	lcdinit();
	prints("ICT2104 SIT 2015");
	gotoXy(0,1);
	prints("0min:00sec:000ms");

	P1DIR |= 0x01;              // P1.0 - Outputs
	P8DIR |= BIT1 + BIT2; 		// Set P8.1 and P8.2 to output direction

	// switch interrupt
	P2IES |= BIT0 + BIT2; 		// P2.0 and P2.2 Hi/lo edge
	P2IFG &= ~BIT0 & ~BIT2; 	// P2.0 and P2.2 IFG cleared
	P2IE |= BIT0 + BIT2; 		// P2.0 and P2.2 interrupt enabled

	TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
	TA0CCR0 = 32;
	TA0CTL = TASSEL_1 + MC_1 + TACLR;         // ACLK, upmode, clear TAR

	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM3, enable interrupts
	__no_operation();                         // For debugger
}

unsigned short m=0;
unsigned short s=0;
unsigned short ms=0;

// Timer0 A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	ms += 1;					// increment at 2ms interval
	P1OUT ^= 0x01;                            // Toggle P1.0
	gotoXy(11,1);
	lcdData((ms/100) + 0x30);
	lcdData(((ms%100)/10) + 0x30);
	lcdData((ms%10) + 0x30);
	if(ms>=1000) {
		ms=0;
		s++;
		gotoXy(5,1);
		lcdData((s/10) + 0x30);
		lcdData((s%10) + 0x30);
//		P1OUT ^= 0x01;                            // Toggle P1.0
		if(s>=60) {
			s=0;
			m++;
			gotoXy(0,1);
			lcdData((m%10) + 0x30);
			if(m>=60)
				m=0;
		}
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void pushbutton_ISR (void)
{
	switch(__even_in_range( P2IV, 10 ))
	{
	case 0x00: break; 					// None
	case P2IV_P2IFG0: 					// Pin 2.0 => 0x02
		P8OUT ^= BIT1; 					// P8.1 = toggle
//		TA0CCTL0 ^= CCIE;         		// toggle CCR0 interrupt approach
		if ((BIT1&P8IN)==BIT1)			// read P8.1 Output
			TA0CCTL0 &= ~CCIE;         		// disable CCR0 interrupt
		else
			TA0CCTL0 |= CCIE;         		// enable CCR0 interrupt
		break;
	case 0x04: break; 					// Pin 2.1
	case P2IV_P2IFG2: 					// Pin 2.2 => 0x06
		P8OUT ^= BIT2; 					// P8.1 = toggle
		if ((BIT1&P8IN)==BIT1)			// timer stop?
		{
			gotoXy(0,1);
			prints("0min:00sec:000ms");
			m=0;s=0;ms=0;
		}
		break;
	case 0x08: break; 					// Pin 2.3
	case 0x0A: break; 					// Pin 2.4
	case 0x0C: break; 					// Pin 2.5
	case 0x0E: break; 					// Pin 2.6
	case 0x10: break; 					// Pin 2.7
	default: break;
	}
}

