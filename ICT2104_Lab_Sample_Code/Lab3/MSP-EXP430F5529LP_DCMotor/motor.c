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

#define MCLK_FREQUENCY 4000000
#define PWM_PERIOD (MCLK_FREQUENCY/5000)-1

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
int dir = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	P6DIR = 0xFF;			// configure LCD
	P6OUT = 0x00;
	lcdinit();

	prints("ICT2104 PWM DEMO");
	gotoXy(0,1);

	P1DIR |= BIT4 + BIT5;			// Set P1.4 and P1.5 to output direction
	P1DS |= BIT4 + BIT5;			// Drive Strength

	P2DIR |= BIT4;					// IR LED
	P2OUT = BIT4;					// ON IR
	P1DIR &= ~(BIT2|BIT3); 			// P1.2 and P1.3 as input

    //setup interrupts for quadrature encoder
    P1IE |=  BIT2 + BIT3;        	//enable interrupts for IR Encoder

    //active-high configuration
    //P1REN |= BIT2 + BIT3;         //enable pullups on respective ports
    //P1OUT |= BIT2 + BIT3;

    P1IFG &= ~(BIT2 + BIT3);         //clear interrupt flag
    P1IES &= ~(BIT2 + BIT3);         //Interrupt on Hi->Lo transition

	TA0CCR0 = PWM_PERIOD;                   // PWM Period
	TA0CCTL3 = OUTMOD_7;                    // CCR3 reset/set
	TA0CCTL4 = OUTMOD_7;                    // CCR4 reset/set
	TA0CCR3 = 0;                         	// CCR3 PWM duty cycle
	TA0CCR4 = 0;                         	// CCR4 PWM duty cycle
	TA0CTL = TASSEL_1 + MC_1 + TACLR;       // ACLK, up mode, clear TAR

    _enable_interrupt();

	for(;;) {
		volatile unsigned int i;	// volatile to prevent optimization

		if(TA0CCR3<PWM_PERIOD & TA0CCR4==0) {
			P1SEL |= BIT4;         	// P1.4 to TA0.3
			P1OUT = BIT4 | ~BIT5;	//	FORWARD and speed up
			TA0CCR3++;
			gotoXy(0,1);
			prints("F=");
			gotoXy(2,1);
			integerToLcd(TA0CCR3);
		}
		else if(TA0CCR4<PWM_PERIOD & TA0CCR3==PWM_PERIOD) {
			P1SEL |= BIT5;         	// P1.5 to TA0.4
			P1OUT = BIT4 | BIT5;	//	FORWARD and slow down
			TA0CCR4++;
			gotoXy(8,1);
			prints("R=");
			gotoXy(10,1);
			integerToLcd(TA0CCR4);
		}
		else if(TA0CCR3>0 & TA0CCR4==PWM_PERIOD) {
			P1SEL |= BIT4;         	// P1.5 to TA0.4
			P1OUT = BIT4 | BIT5;	//	REVERSE and speed up
			TA0CCR3--;
			gotoXy(0,1);
			prints("F=");
			gotoXy(2,1);
			integerToLcd(TA0CCR3);
		}
		else if(TA0CCR3==0 & TA0CCR4>0) {
			P1SEL |= BIT5;         	// P1.5 to TA0.4
			P1OUT = BIT4 | BIT5;	//	REVERSE and slow down
			TA0CCR4--;
			gotoXy(8,1);
			prints("R=");
			gotoXy(10,1);
			integerToLcd(TA0CCR4);
		}
		gotoXy(0,0);

		if(tr_state==0)
		{
		    if (dir > 0)
		    	prints("CW DIR ");
		    else if (dir < 0)
		    	prints("CCW DIR");
		    else
		    	prints("=STOP?=");
		}
	}
}
// for 5-holes wheel
void pattern_match_5(){
 /* Transition sequence is as follows (Active High Config)
  *         --Clockwise--
  * CH_A    ->  H L L H H
  * CH_B    ->  H H L L H
  * 			3 2 0 1 3
  *     ---Counter-Clockwise---
  * CH_A    ->  H H L L H
  * CH_B    ->  H L L H H
  * 			3 1 0 2 3
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
                dir = 1;	// cw
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
                dir = -1;	// ccw
            }

        }
    }
}

// for 2-holes wheel
void pattern_match_2(){
 /* Transition sequence is as follows (Active High Config)
  *         --Clockwise--
  * CH_A    ->  H L L L H
  * CH_B    ->  H H L L L
  *				3 2 0 0 1
  *     ---Counter-Clockwise---
  * CH_A    ->  H H L L L
  * CH_B    ->  H L L L H
  * 		    3 1 0 0 2
  *
  * If the above pattern is stored in a 2 bit array with CH_A as Bit 2 and CH_B as Bit 3
  * The value shall be as follows (integer)
  * CW      -> 3-3-0-1-3
  * CCW     -> 3-1-0-2-3
  */
    int cw[]={3,2,0,0,1};
    int ccw[]={3,1,0,0,2};
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
                dir = 1;	// cw
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
                dir = -1;	// ccw
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
        if ((P1IN & BIT2)==BIT2) {a=1;} else {a=0;}  //read pin state CH A
        if ((P1IN & BIT3)==BIT3) {b=1;} else {b=0;}  //read pin state CH B
//        cval = (a << 0) | (b <<1);
        cval = (a<<0) + (b<<1);			  // HH=3, HL=1, LH=2
        P1IES ^= BIT2;                    //toggle interrupt edge
    }

    //Check for BIT3 interrupt
    if ((P1IFG & BIT3)==BIT3)
    {
        if ((P1IN & BIT2)==BIT2) {a=1;} else {a=0;}  	//read pin state CH A
        if ((P1IN & BIT3)==BIT3) {b=1;} else {b=0;}     //read pin state CH B
//        cval = (a << 0) | (b <<1);
        cval = (a<<0) + (b<<1);			// HH=3, HL=1, LH=2
        P1IES ^= BIT3;    				//toggle interrupt edge
    }

//    pattern_match_5();	// for 5 holes
    pattern_match_2();		// for 2 holes

    //clear all interrupts
    P1IFG &= ~(BIT2 + BIT3);

    //enable all interrupts
    P1IE |= (BIT2 + BIT3);
}

