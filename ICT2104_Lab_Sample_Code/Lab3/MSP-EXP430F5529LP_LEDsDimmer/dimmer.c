//***************************************************************************************
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P2.4|--> PWM --> LED8.1
//            |             P2.5|--> PWM --> LED8..2
//
//***************************************************************************************

#include <msp430.h>				
#include "lcd16.h"

#define PWM_PERIOD 1000

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	P6DIR = 0xFF;			// configure LCD
	P6OUT = 0x00;
	lcdinit();

	prints("ICT2104 PWM LEDs");
	gotoXy(0,1);

	P1DIR |= BIT0;					// Set P1.0 LED output direction
	P4DIR |= BIT7;					// Set P4.7 LED output direction

	P2DIR |= BIT4 + BIT5;			// Set P2.4 and P2.5 to PWM LED output direction
	P8DIR |= BIT1 + BIT2;			// Set P8.1 and P8.2 to output direction
	P8OUT &= ~BIT1 & ~BIT2;			// off leds

	P2DIR |= BIT2;                	// SMCLK set out to P2.2
	P2SEL |= BIT2;
	P7DIR |= BIT7;                	// MCLK set out to P7.7
	P7SEL |= BIT7;

	//////////////////////// use XT1 32768 Hz ////////////////////////////////////////////
	P5SEL |= BIT4 + BIT5;                       // Select XT1
	UCSCTL6 &= ~XT1OFF;                         // Enable XT1
	UCSCTL6 |= XCAP_3;                          // Internal load cap

	//////////////////////// use XT2 4.0 MHz ////////////////////////////////////////////
	P5SEL |= BIT2 + BIT3;                       // Select XT2
	UCSCTL6 &= ~XT2OFF;                         // Enable XT2
	UCSCTL3 |= SELREF__XT1CLK;                  // FLLref = XT1

	do {                                        	// Loop until XT1 and XT2 stabilize
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2, XT1 fault flags
	    SFRIFG1 &= ~OFIFG;                      	// Clear oscillator fault flag
	} while (SFRIFG1 & OFIFG);                  	// Test oscillator fault flag
	UCSCTL6 &= ~(XT2DRIVE0 + XT2DRIVE1);        	// Decrease XT2 Drive as it is stabilized
	UCSCTL4 |= SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK;     	// ACLK = XT1, MCLK = SMCLK = XT2

	// setup Timer1
	TA1CCTL0 = CCIE;                        // CCR0 interrupt enabled
	TA1CCR0 = PWM_PERIOD/20;            	// polling approach
	TA1CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK, up mode, clear TAR

	// Setup Timer2 for P2.4 & P2.5 PWM
	TA2CCR0 = PWM_PERIOD;            		// PWM Period
	TA2CCTL1 = OUTMOD_7;                    // CCR1 reset/set
	TA2CCTL2 = OUTMOD_7;                    // CCR2 reset/set
	TA2CCR1 = 0;                         	// CCR1 PWM duty cycle
	TA2CCR2 = 0;                   			// CCR2 PWM duty cycle
	TA2CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK, up mode, clear TAR

    _enable_interrupt();

	for(;;)
	{
		if(TA2CCR1<PWM_PERIOD & TA2CCR2==0) {
			P2SEL |= BIT4;         	// P2.4 to TA2.1
			P2OUT = BIT4;			// LED1 bright up
			TA2CCR1++;

			gotoXy(0,1);
			prints("1=");
			gotoXy(2,1);
			integerToLcd(TA2CCR1);
		}
		else if(TA2CCR2<PWM_PERIOD & TA2CCR1==PWM_PERIOD) {
			P2SEL |= BIT5;         	// P2.5 to TA2.2
			P2OUT = BIT5;			// LED2 bright up
			TA2CCR2++;

			gotoXy(8,1);
			prints("2=");
			gotoXy(10,1);
			integerToLcd(TA2CCR2);
		}
		else if(TA2CCR1>0 & TA2CCR2==PWM_PERIOD) {
			P2SEL |= BIT4;         	// P2.4 to TA2.1
			P2OUT = BIT4;			// LED1 dim down
			TA2CCR1--;

			gotoXy(0,1);
			prints("1=");
			gotoXy(2,1);
			integerToLcd(TA2CCR1);
		}
		else if(TA2CCR1==0 & TA2CCR2>0) {
			P2SEL |= BIT5;         	// P2.5 to TA2.2
			P2OUT = BIT5;			// LED2 dim down
			TA2CCR2--;

			gotoXy(8,1);
			prints("2=");
			gotoXy(10,1);
			integerToLcd(TA2CCR2);
		}
	}
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	if((P2IN&BIT4)==BIT4)
		P8OUT|=BIT1;
	else if((P2IN&BIT4)==0)
		P8OUT&=~BIT1;

	if((P2IN&BIT5)==BIT5)
		P8OUT|=BIT2;
	else if((P2IN&BIT5)==0)
		P8OUT&=~BIT2;

//	P1OUT = P1OUT^BIT0;		// debugging
}

