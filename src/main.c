#include <msp430f5529.h>

#define TIMER_PERIOD 511
#define DUTY_CYCLE 384 /*! TODO: ����
 *  \todo ����
 */
#define DUTY_CYCLE2 128 /*! TODO: ����
 *  \todo ����
 */
void main ( void ) {
	/*********
	 *  ���Ź�  *
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
	 *  ����  *
	 ********/
	P6DIR &= ~ ( BIT1 + BIT2 + BIT3 + BIT4 ); // P6.1, P6.2, P6.3, P6.4 input
	P6OUT |= ( BIT1 + BIT2 + BIT3 + BIT4 ); // Set P6.1, P6.2, P6.3, P6.4 as pull-Up resistance
	P6IES |= ( BIT1 + BIT2 + BIT3 + BIT4 ); // P6.1, P6.2, P6.3, P6.4 Hi/Lo edge
	/***********
	 *  ����������  *
	 ***********/
	/* TODO:�����Ǵ�����  <08-08-19, Freed-Wu> */
	/**********
	 *  ģ����  *
	 **********/
	P6DIR &= ~BIT5; // P6.5 input
	P6OUT |= BIT5; // Set P6.5 as pull-Up resistance
	P6IES |= BIT5; // P6.5 Hi/Lo edge

	switch ( P6IN & BIT5 ) {
		case 1: // ����Ҫ��
			break;

		default: // ���Ӳ���
	}

	/***********
	 *  �͹���ģʽ  *
	 ***********/
	__bis_SR_register ( LPM3_bits + GIE ); // ��LPM3�������ж�
	__no_operation(); // For debugger
}

/**********************************************************************
 *                                 ����                                 *
 **********************************************************************/
/* TODO:�ж� <08-08-19, Freed-Wu> */

/**********************************************************************
 *                               ����������                                *
 **********************************************************************/
/* TODO:�ж� <08-08-19, Freed-Wu> */
