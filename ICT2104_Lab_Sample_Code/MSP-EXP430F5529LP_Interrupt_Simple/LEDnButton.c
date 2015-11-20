#include "msp430f5529.h"
#include "lcd16.h"

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; 	// Stop watchdog timer

	P8DIR |= BIT1 + BIT2; 		// Set P8.1 and P8.2 to output direction

	P6DIR = 0xFF;				// configure LCD
	P6OUT = 0x00;				// initialise all pins to low

	lcdinit();
	prints("Switch Interrupt");

///////////////////////////////////////////////////////////////////////////////
	// Set Master Clock to XT2
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

	do                                          // Loop until XT1 and XT2 stabilize
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);     // Clear XT2, XT1 fault flags
		SFRIFG1 &= ~OFIFG;                      // Clear oscillator fault flag
	} while (SFRIFG1 & OFIFG);                  // Test oscillator fault flag

	UCSCTL6 &= ~(XT2DRIVE0 + XT2DRIVE1);        // Decrease XT2 Drive as it is stabilized
	UCSCTL6 &= ~(XT1DRIVE0 + XT1DRIVE1);        // Decrease XT1 Drive as it is stabilized

	UCSCTL4 |= SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; 	// ACLK = XT1, MCLK = SMCLK = XT2
//	UCSCTL4 |= SELA__XT1CLK + SELM__XT2CLK;         		// ACLK = XT1, MCLK = XT2

	P1DIR |= BIT0;								// Set P1.0 to output direction

	// Timer interrupt
//	TA0CCTL0 = CCIE;							// capture & compare interrupt
//	TA0CCR0 = 0xFFFF;
//	TA0CTL = TASSEL__SMCLK + MC_1 + ID__4;		//	SMCLK, Up to CCR0, /4
//	TA0CTL = TASSEL_1 + MC_1;					//	ACLK

///////////////////////////////////////////////////////////////////////////////
	// interrupt approach
	P2IES |= BIT0 + BIT2; 		// P2.0 and P2.2 Hi/lo edge
//	P2IES &= ~BIT0 & ~BIT2; 	// P2.0 and P2.2 lo/Hi edge
	P2IFG &= ~BIT0 & ~BIT2; 	// P2.0 and P2.2 IFG cleared
	P2IE |= BIT0 + BIT2; 		// P2.0 and P2.2 interrupt enabled

//	while(1)
//	{
//		//_BIS_SR(LPM4_bits + GIE); // Enter LPM4 w/interrupt
//		_BIS_SR(GIE); // Enable Ginterrupt
//		__no_operation();
//	}

//	_enable_interrupt(); //global interrupt
	_BIS_SR(GIE); // Enable Ginterrupt
	for(;;)
	{//
		P1OUT ^= BIT0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// nested interrupts
// TimerA0 ISR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void led_ISR (void)
{
	P1OUT ^= BIT0; 		// P1.0 = toggle
}
// nested interrupts
///////////////////////////////////////////////////////////////////////////////
// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void pushbutton_ISR (void)
{
	switch(__even_in_range( P2IV, 10 ))
	{
	case 0x00: break; 					// None
	case P2IV_P2IFG0: 					// Pin 2.0 => 0x02
		P8OUT ^= BIT1; 					// P8.1 = toggle
//		gotoXy(0,1);
//		if ((BIT1&P8IN)==BIT1)			// read P8.1 Output
//			prints("LED 8.1 is ON ");
//		else
//			prints("LED 8.1 is OFF");
		break;
	case 0x04: break; 					// Pin 2.1
	case P2IV_P2IFG2: 					// Pin 2.2 => 0x06
		P8OUT ^= BIT2; 					// P8.1 = toggle
		gotoXy(0,1);
		if ((BIT2&P8IN)==BIT2)			// read P8.2 Output
			prints("LED 8.2 is ON ");
		else
			prints("LED 8.2 is OFF");
		P2IFG &= ~BIT0 & ~BIT2; 		// P2.0 and P2.2 IFG cleared
		P2IES ^= BIT2;					// P2.2 lo/Hi edge
		break;
	case 0x08: break; 					// Pin 2.3
	case 0x0A: break; 					// Pin 2.4
	case 0x0C: break; 					// Pin 2.5
	case 0x0E: break; 					// Pin 2.6
	case 0x10: break; 					// Pin 2.7
	default: break;
	}

    //disable all interrupts
//    P2IE &= ~(BIT0 + BIT2);
//
//	if((P2IFG & BIT0)==BIT0)
//	{
//	    //button P2.0 has been pressed
//		P8OUT ^= BIT1; 					// P8.1 = toggle
//		gotoXy(0,1);
//		if ((BIT1&P8IN)==BIT1)			// read P8.1 Output
//			prints("LED 8.1 is ON ");
//		else
//			prints("LED 8.1 is OFF");
//	}
//	else if((P2IFG & BIT2)==BIT2)
//	{
//	    //button P2.2 has been pressed
//		P8OUT ^= BIT2; 					// P8.2 = toggle
//		gotoXy(0,1);
//		if ((BIT2&P8IN)==BIT2)			// read P8.2 Output
//			prints("LED 8.2 is ON ");
//		else
//			prints("LED 8.2 is OFF");
//	}
//	P2IFG &= ~BIT0 & ~BIT2; 	// P2.0 and P2.2 IFG cleared
}

