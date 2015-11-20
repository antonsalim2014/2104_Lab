//***************************************************************************************
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.4|--> TA0.3
//            |             P1.5|--> TA0.4
//
//***************************************************************************************

#include <msp430.h>				
#include "lcd16.h"

#define PWM_PERIOD 4095

//variable declaration
char buffer[5];
int a=0;        	//variable stores interrupt edge
int b=0;        	//variable stores interrupt edge
int cval=3;     	//store current value to determine rotation
int pval=3;     	//stores previous value to determine rotation direction
int tr_state=0;     //variable to store transition state
int num=0;      	//arbitary number. will increment or decrement iff tr_state=4
int tr;         	//holds trigger state for variable tr_state
int inc_fact=1;     //this variable holds the increment factor

unsigned short power;		// power value from ADC

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	ADC12CTL0 = ADC12SHT02 + ADC12ON;         	// Sampling time, ADC12 on
	ADC12CTL1 = ADC12SHP;                     	// Use sampling timer
	ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_12;	// Vr+=AVcc and Vr-=AVss; Select Channel A12
	ADC12CTL0 |= ADC12ENC;						// Enable conversion
	P7SEL |= BIT0;                            	// P7.0 ADC option select
	ADC12IE = BIT0;                           	// Enable interrupt

	P6DIR = 0xFF;			// configure LCD
	P6OUT = 0x00;
	lcdinit();

	prints("ICT2104 PWM DEMO");
	gotoXy(0,1);

	gotoXy(0,1);
	prints("D=");

	gotoXy(8,1);
	prints("P=");

	P1DIR |= BIT0;					// Set P1.0 output direction for debugging display
	P4DIR |= BIT7;					// Set P4.7 output direction for debugging display
	P1DIR |= BIT4 + BIT5;			// Set P1.4 and P1.5 to output direction
	P1DS |= BIT4 + BIT5;			// Drive Strength

	P2DIR |= BIT4;					// IR LED
	P2OUT = BIT4;					// ON IR
	P1DIR &= ~(BIT2|BIT3); 			// P1.2 and P1.3 as input

    //setup interrupts for quadrature encoder
    P1IES |= BIT2 + BIT3;         //Interrupt on Hi->Lo transition
//    P1IES &= ~(BIT2 + BIT3);       //Interrupt on Lo->Hi transition
    P1IFG &= ~(BIT2 + BIT3);         //clear interrupt flag
    P1IE |=  BIT2 + BIT3;        	//enable interrupts for IR Encoder

	TA0CCR0 = PWM_PERIOD;                   // PWM Period
	TA0CCTL3 = OUTMOD_7;                    // CCR3 reset/set
	TA0CCTL4 = OUTMOD_7;                    // CCR4 reset/set
	TA0CCR3 = 0;                         	// CCR3 PWM duty cycle
	TA0CCR4 = 0;                         	// CCR4 PWM duty cycle
	TA0CTL = TASSEL_1 + MC_1 + TACLR;       // ACLK, up mode, clear TAR

    _enable_interrupt();

	P1SEL |= BIT4;         	// P1.4 to TA0.3
	P1OUT = BIT4;			// Drive FORWARD

	for(;;)
	{
		volatile unsigned int i;	// volatile to prevent optimization

		ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

		if(TA0CCR3 < (power-5))			// tol of +/-5
			TA0CCR3+=3;
		else if (TA0CCR3 > (power+5))
			TA0CCR3-=3;

		gotoXy(2,1);
		integerToLcd(TA0CCR3);		// display duty cycle value

		// display direction
		gotoXy(0,0);
	    if (cval!=pval && tr==1)
	    	prints("CW DIR ");
	    else if (cval!=pval && tr==0)
	    	prints("CCW DIR");
	}
}

void pattern_match(){
 /* Transition sequence is as follows (Active High Config)
  *         --Clockwise--
  * CH_A    ->  H L L H H
  * CH_B    ->  H H L L H
  *     ---Counter-Clockwise---
  * CH_A    ->  H H L L H
  * CH_B    ->  H L L H H
  *
  * If the above pattern is stored in a 2 bit array with CH_A as Bit 2 and CH_B as Bit 3
  * The value shall be as follows (integer)
  * CW      -> 3-2-0-1-3
  * CCW     -> 3-1-0-2-3
  */
    int cw[]={3,2,0,1,3};
    int ccw[]={3,1,0,2,3};
    int i;
    for (i=1;i<5;i++)
    {
        if (pval==cw[i-1] && cval==cw[i])
        {
            if (tr){tr_state++;} else {(tr=1);};
            if (tr_state>=5)
            {
                num=num+inc_fact;
                tr_state=0;
            }
        }
    }

    for (i=1;i<5;i++)
    {
        if (pval==ccw[i-1] && cval==ccw[i])
        {
            if (!tr){tr_state++;} else {(tr=0);};
            if (tr_state>=5)
            {
                num=num-inc_fact;
                tr_state=0;
            }

        }
    }
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    pval=cval;      //store current_value

    //disable all interrupts
    P1IE &= ~(BIT2 + BIT3);

    //check interrupt flags to find which interrupt was triggred
    //Check for BIT2 interrupt
    if ((P1IFG & BIT2)==BIT2)
    {
        if ((P1IN & BIT2)==BIT2) {a=1;} else {a=0;}  //read pin state
        if ((P1IN & BIT3)==BIT3) {b=1;} else {b=0;}  //read pin state
        cval = (a<<0) + (b<<1);
        P1IES ^= BIT2;                    //toggle interrupt edge
    }

    //Check for BIT3 interrupt
    if ((P1IFG & BIT3)==BIT3)
    {
        if ((P1IN & BIT2)==BIT2) {a=1;} else {a=0;}  	//read pin state
        if ((P1IN & BIT3)==BIT3) {b=1;} else {b=0;}     //read pin state
        cval = (a<<0) + (b<<1);
        P1IES ^= BIT3;    //toggle interrupt edge
    }

    pattern_match();

    //clear all interrupts
    P1IFG &= ~(BIT2 + BIT3);

    //enable all interrupts
    P1IE |= (BIT2 + BIT3);
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
	switch(__even_in_range(ADC12IV,34))
	{
	case  0: break;                           	// Vector  0:  No interrupt
	case  2: break;                           	// Vector  2:  ADC overflow
	case  4: break;                           	// Vector  4:  ADC timing overflow
	case  6:                                  	// Vector  6:  ADC12IFG0
		power = ADC12MEM0;
//		power *= (PWM_PERIOD/4095);				// assume linear scale factor and ignore losses
		if (ADC12MEM0 >= 0x7ff) {             	// mid range
			P4OUT |= BIT7;                    	// P4.7 = 1
			printf("%d => %f\n", ADC12MEM0, power);
		}
		else {
			P4OUT &= ~BIT7;                  	// P4.7 = 0
			printf("%d => %f\n", ADC12MEM0, power);
		}
		gotoXy(10,1);
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
