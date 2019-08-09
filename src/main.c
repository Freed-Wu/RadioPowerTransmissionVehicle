#include <stdio.h>
#include <string.h>
#include <msp430f5529.h>

/**********************************************************************
*                                 ����                                 *
**********************************************************************/
#define S1 (P2IN&BIT0)
#define S2 (P2IN&BIT6)
#define S3 (P2IN&BIT7)
#define S4 (P2IN&BIT5)   //���� 5V �� 0V

/*#define DEBUG*/
#ifdef DEBUG
	char str[];
#endif
#define TIMER_PERIOD 511
#define VOLTAGE (3.3 * ADC12MEM0 / 0xfff )
#define CURRENCY (( VOLTAGE - 2.5 ) / 0.185)

unsigned int turn = 0;
#define TURN_MAX 150
int mode = 1; // ģʽ1��2��3
int isCharge = 1; // Ĭ�ϳ��
#define POWER_THRESHOLD 0.05 // ģʽ1��2��ͨ��ϵ� 
#define CHARGE_THRESHOLD 0.5 // ģʽ3�¿������վԶ����վ 

void go_straight ( float );
void turn_left ( float );
void turn_right ( float );
void turn_left_heavy ( float );
void turn_right_heavy ( float );
void SearchRun ( float );

/**
 *  @brief main
 */
void main ( void ) {
	/*********
	 *  ���Ź�  *
	 *********/
	WDTCTL = WDT_ADLY_16; // WDT 250ms, ACLK, interval timer
	SFRIE1 |= WDTIE; // Enable WDT interrupt
	/*********
	 *  PWM  *
	 *********/
	P1DIR |= BIT2 + BIT3; // P1.2 and P1.3 output as PWM
	P1SEL |= BIT2 + BIT3; // P1.2 and P1.3 options select
	TA0CCR0 = TIMER_PERIOD - 1; // PWM Period
	TA0CCTL1 = OUTMOD_7; // CCR1 reset/set
	TA0CCTL2 = OUTMOD_7; // CCR2 reset/set 512*75%=128
	TA0CTL = TASSEL__ACLK + MC__UP + TACLR; // ACLK, up mode, clear TAR
	/***********
	 *  ����������  *
	 ***********/
	REFCTL0 &= ~REFMSTR; // ��λREFMSTR����λ����ʹ��ADC12_A�ο���ѹ���ƼĴ���
	ADC12CTL0 = ADC12SHT02 + ADC12ON; // ����ʱ��Tsample = 64��ADC12CLK, ADC12 on
	// ADC12MEMO0��Ϊת����ַ��ȱʡ�����������ģʽ��ʹ�ò�����ʱ����, �ο�ʱ��ԴACLK����ͨ������ת����ȱʡ��
	ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHP + ADC12SSEL_1 + ADC12CONSEQ_0;
	ADC12MCTL0 = ADC12INCH_0 + ADC12SREF_0; // ����ͨ��0���ο���ѹ��VR+ = AVcc, VR- = AVss����Ϊȱʡֵ����ע�͵�
	ADC12IE |= BIT0; // Enable interrupt
	ADC12CTL0 |= ADC12ENC;  //ADC12ENCΪ�͵�ƽʱ��ؿ��ƼĴ������ܱ��޸�
	P6SEL |= BIT0; // P6.0 ADC option select
	#ifdef DEBUG
	/**********
	 *  UART  *
	 **********/
	P3SEL |= BIT3 + BIT4;                     // P3.3,4 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	UCA0BR1 = 0;                              // 1MHz 115200
	UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
	/*********
	 *  LED  *
	 *********/
	P4DIR |= BIT7;
	P1DIR |= BIT0;
	P8DIR |= BIT1;
	P8OUT |= BIT1;

	if ( P8IN & BIT2 ) {
		mode = 3;
		P1OUT |= BIT0;
		P4OUT |= BIT7;

	} else if ( P6IN & BIT5 ) {
		mode = 1;
		P1OUT |= BIT0;
		P4OUT &= ~BIT7;

	} else {
		mode = 2;
		P4OUT |= BIT7;
		P1OUT &= ~BIT0;
	}

	P8OUT &= ~BIT1;
	__bis_SR_register ( LPM0_bits + GIE ); // ��LPM0�����ж�
	#else
	P8DIR |= BIT1;
	P8OUT |= BIT1;

	if ( P8IN & BIT2 )
		mode = 3;

	else if ( P6IN & BIT5 )
		mode = 1;

	else
		mode = 2;

	P8OUT &= ~BIT1;
	__bis_SR_register ( LPM3_bits + GIE ); // ��LPM3�����ж�
	#endif

	while ( 1 );
}
/**********************************************************************
*                               ����������                                *
**********************************************************************/
#pragma vector = ADC12_VECTOR
/**
  *  @brief ����������
  */
__interrupt void ADC12_ISR ( void ) {
	switch ( __even_in_range ( ADC12IV, 34 ) ) {
		case 0:
			break; // Vector 0: No interrupt

		case 2:
			break; // Vector 2: ADC overflow

		case 4:
			break; // Vector 4: ADC timing overflow

		case 6: // Vector 6: ADC12IFG0
			#ifdef DEBUG
			sprintf ( str, "%4d, %8f, %8f\r\n", ADC12MEM0, VOLTAGE, CURRENCY );
			int i;

			for ( i = 0; i < strlen ( str ); ++i ) {
				UCA0TXBUF = str[i];                  // TX -> RXed character
			}

			#endif

			switch ( mode ) {
				case 3:
					if ( CURRENCY < CHARGE_THRESHOLD ) // Զ����վ
						isCharge = 0;

					else   // �������վ
						isCharge = 1;

					break;

				default:
					if ( CURRENCY < POWER_THRESHOLD ) // �ϵ�
						isCharge = 0;
			}

			break;

		case 8:
			break; // Vector 8: ADC12IFG1

		default:
			break;
	}
}
/**********************************************************************
*                                ���Ź�                                 *
**********************************************************************/
// Watchdog Timer interrupt service routine
#pragma vector = WDT_VECTOR
/**
  *  @brief ���Ź�
  */
__interrupt void WDT_ISR ( void ) {
	switch ( mode ) {
		case 1:
			if ( isCharge ) {
				ADC12CTL0 |= ADC12SC; // Start sampling/conversion

			} else {
				if ( turn < TURN_MAX ) {
					SearchRun ( 1 );
					turn ++ ;

				} else
					SearchRun ( 0 );
			}

			break;

		case 2:
			if ( isCharge ) {
				ADC12CTL0 |= ADC12SC; // Start sampling/conversion

			} else
				SearchRun ( 1 );

			break;

		default:
			if ( isCharge )
				SearchRun ( 1 );

			else
				SearchRun ( 1.3 );

			ADC12CTL0 |= ADC12SC; // Start sampling/conversion
	}
}
/**********************************************************************
*                                 ����                                 *
**********************************************************************/
void go_straight ( float factor ) {
	TA0CCR1 = ( unsigned int ) ( 100 * factor );
	TA0CCR2 = ( unsigned int ) ( 100 * factor );
}
void turn_left ( float factor ) {
	TA0CCR1 = ( unsigned int ) ( 100 * factor );
	TA0CCR2 = ( unsigned int ) ( 200 * factor );
}
void turn_right ( float factor ) {
	TA0CCR1 = ( unsigned int ) ( 200 * factor );
	TA0CCR2 = ( unsigned int ) ( 100 * factor );
}
void turn_left_heavy ( float factor ) {
	TA0CCR1 = ( unsigned int ) ( 20 * factor );
	TA0CCR2 = ( unsigned int ) ( 120 * factor );
}
void turn_right_heavy ( float factor ) {
	TA0CCR1 = ( unsigned int ) ( 120 * factor );
	TA0CCR2 = ( unsigned int ) ( 20 * factor );
}
void SearchRun ( float factor ) {
	/********�����������ת*******/
	if ( S2 ) { //
		turn_left ( factor );
		return;
	}

	/********�ұ���������ת*******/
	if ( S3 ) { //
		turn_right ( factor );
		return;
	}

	/********���ұ������ض���ת*******/
	if ( S4 ) { //
		turn_right_heavy ( factor );
		return;
	}

	/********����������ض���ת*******/
	if ( S1 ) {
		turn_left_heavy ( factor );
		return;
	}

	/********û����ֱ��*******/
	if ( S2 == 0 && S3 == 0 ) {
		go_straight ( factor );
		return;
	}
}
