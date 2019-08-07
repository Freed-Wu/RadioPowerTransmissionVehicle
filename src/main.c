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
			P2DIR &= ~ BIT1; // P2.1 input // LED1
			P2OUT |= BIT1; // Set P2.1 as pull-Up resistance
			P2IES |= BIT1; // P2.1 Hi/Lo edge
			break;

		default: // ���Ӳ���
			P1DIR &= ~ BIT1; // P1.1 input // LED2
			P1OUT |= BIT1; // Set P1.1 as pull-Up resistance
			P1IES |= BIT1; // P1.1 Hi/Lo edge
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
// PORT6 interrupt service routine
#pragma vector=PORT6_VECTOR
__interrupt void Port_6 ( void ) {
	if ( P6IFG & BIT1 ) { // ��P6.1�жϣ�
		/* TODO:  <08-08-19, Freed-Wu> */
		P6IFG &= ~BIT1; // ��P2.6�жϱ�־
	}

	if ( P6IFG & BIT2 ) { // ��P6.2�жϣ�
		/* TODO:  <08-08-19, Freed-Wu> */
		P6IFG &= ~BIT2; // ��P2.7�жϱ�־
	}

	if ( P6IFG & BIT3 ) { // ��P6.3�жϣ�
		/* TODO:  <08-08-19, Freed-Wu> */
		P6IFG &= ~BIT3; // ��P2.7�жϱ�־
	}

	if ( P6IFG & BIT4 ) { // ��P6.4�жϣ�
		/* TODO:  <08-08-19, Freed-Wu> */
		P6IFG &= ~BIT4; // ��P2.7�жϱ�־
	}
}
/**********************************************************************
 *                               ����������                                *
 **********************************************************************/
// PORT6 interrupt service routine
#pragma vector=PORT6_VECTOR
/* TODO:�ж� <08-08-19, Freed-Wu> */
__interrupt void Port_6 ( void ) {
	if ( P6IFG & BIT1 ) { // ��P6.1�жϣ�
		/* TODO:  <08-08-19, Freed-Wu> */
		P6IFG &= ~BIT1; // ��P2.6�жϱ�־
	}

	switch ( P6IN & BIT5 ) {
		case 1: // ����Ҫ��
			P1OUT |= BIT1; // ����P1.1��LED
			break;

		default: // ���Ӳ���
			P2OUT |= BIT1; // ����P2.1��LED
	}
}
