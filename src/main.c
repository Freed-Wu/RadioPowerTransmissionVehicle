#include <msp430f5529.h>

#define TIMER_PERIOD 511
#define DUTY_CYCLE 384 /*! TODO: 待调
 *  \todo 待调
 */
#define DUTY_CYCLE2 128 /*! TODO: 待调
 *  \todo 待调
 */
void main ( void ) {
	/*********
	 *  看门狗  *
	 *********/
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	/*********
	 *  PWM  *
	 *********/
	P1DIR |= BIT2 + BIT3; // P1.2 and P1.3 output
	P1SEL |= BIT2 + BIT3; // P1.2 and P1.3 options select
	TA0CCR0 = TIMER_PERIOD - 1; // PWM Period
	TA0CCTL1 = OUTMOD_7; // CCR1 reset/set
	TA0CCR1 = DUTY_CYCLE; // CCR1 PWM duty cycle 512*75%=384
	TA0CCTL2 = OUTMOD_7; // CCR2 reset/set 512*75%=128
	TA0CCR2 = DUTY_CYCLE2; // CCR2 PWM duty cycle
	TA0CTL = TASSEL__ACLK + MC__UP + TACLR; // ACLK, up mode, clear TAR
	/********
	 *  避障  *
	 ********/
	P6DIR &= ~ ( BIT1 + BIT2 + BIT3 + BIT4 ); // P6.1, P6.2, P6.3, P6.4 input
	P6OUT |= ( BIT1 + BIT2 + BIT3 + BIT4 ); // Set P6.1, P6.2, P6.3, P6.4 as pull-Up resistance
	P6IES |= ( BIT1 + BIT2 + BIT3 + BIT4 ); // P6.1, P6.2, P6.3, P6.4 Hi/Lo edge
	/***********
	 *  电流传感器  *
	 ***********/
	/* TODO:数据是串口吗？  <08-08-19, Freed-Wu> */
	/**********
	 *  模块检测  *
	 **********/
	P6DIR &= ~BIT5; // P6.5 input
	P6OUT |= BIT5; // Set P6.5 as pull-Up resistance
	P6IES |= BIT5; // P6.5 Hi/Lo edge

	switch ( P6IN & BIT5 ) {
		case 1: // 基本要求
			break;

		default: // 发挥部分
	}

	/***********
	 *  低功耗模式  *
	 ***********/
	__bis_SR_register ( LPM3_bits + GIE ); // 进LPM3并开总中断
	__no_operation(); // For debugger
}

/**********************************************************************
 *                                 避障                                 *
 **********************************************************************/
/* TODO:中断 <08-08-19, Freed-Wu> */

/**********************************************************************
 *                               电流传感器                                *
 **********************************************************************/
/* TODO:中断 <08-08-19, Freed-Wu> */
