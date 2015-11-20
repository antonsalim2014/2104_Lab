/*
 * lcd16.h
 *
 *  Created on: Dec 1, 2011
 *      Author: Gaurav www.circuitvalley.com
 */

#ifndef MSP_EXP430F5529LP_LCD16X2_LCD16_H_
#define MSP_EXP430F5529LP_LCD16X2_LCD16_H_


//#include <msp430g2211.h>
#include <msp430f5529.h>

#define  RS BIT4
#define  RW BIT5
#define  EN BIT6

void waitlcd(unsigned int x);

void lcdinit(void);
void integerToLcd(unsigned int integer );
void lcdData(unsigned char l);
void prints(char *s);
void gotoXy(unsigned char  x,unsigned char y);

#endif /* MSP_EXP430F5529LP_LCD16X2_LCD16_H_ */
