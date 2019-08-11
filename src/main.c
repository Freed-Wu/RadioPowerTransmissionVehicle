#include <stdio.h>
#include <string.h>
#include <msp430f5529.h>

/**********************************************************************
*                                 避障                                 *
**********************************************************************/
#define S1 (P2IN&BIT0)
#define S2 (P2IN&BIT6)
#define S3 (P2IN&BIT7)
#define S4 (P2IN&BIT5)   //不亮 5V 亮 0V

/**********************************************************************
*                                PWM                                 *
**********************************************************************/
#define TIMER_PERIOD 511
/*#define PWM_LOGIC_INVERSE*/
#ifdef PWM_LOGIC_INVERSE
	#define PWM_LOGIC(num) ( TIMER_PERIOD - num )
#else
	#define PWM_LOGIC(num) ( num )
#endif
#define LAUNCH_TURN 0
/*#define FLOAT_FACTOR_USE*/
#ifdef FLOAT_FACTOR_USE
	#define FACTOR float
	#define ADC_USE
#else
	#define FACTOR int
#endif

/**********************************************************************
*                                 LED                                 *
**********************************************************************/
#define LED_DEBUG

/**********************************************************************
*                                UART                                *
**********************************************************************/
/*#define UART_DEBUG*/
#ifdef UART_DEBUG
	char str[];
#endif

/**********************************************************************
*                               电流传感器                                *
**********************************************************************/
/*#define ADC_USE*/
#ifdef ADC_USE
	#define VOLTAGE (3.3 * ADC12MEM0 / 0xfff )
	#define CURRENCY (( VOLTAGE - 2.5 ) / 0.185)
	#define POWER_THRESHOLD 0.05 // 模式1、2下通电断电
	#define CHARGE_THRESHOLD 0.5 // 模式3下靠近充电站远离充电站
#endif

/**********************************************************************
*                                看门狗                                 *
**********************************************************************/
unsigned int turn = 0;
unsigned int delayTurn = 0;
#define TURN_MAX 250 // 4000 / 16 = 250 
/*#define CHARGE_DEBUG*/
#ifdef CHARGE_DEBUG
	#define DELAY_TURN_MAX 0
#else
	#define DELAY_TURN_MAX 3750 // 60000 / 16 = 3750
#endif
unsigned int mode = 1; // 模式1、2、3
unsigned int isCharge = 1; // 默认充电

void go_straight ( FACTOR );
void turn_left ( FACTOR );
void turn_right ( FACTOR );
void turn_left_heavy ( FACTOR );
void turn_right_heavy ( FACTOR );
void SearchRun ( FACTOR );

/**
 *  @brief main
 */
void main ( void ) {
	/*********
	 *  看门狗  *
	 *********/
	WDTCTL = WDT_ADLY_16; // WDT 250ms, ACLK, interval timer
	SFRIE1 |= WDTIE; // Enable WDT interrupt
	/*********
	 *  PWM  *
	 *********/
	P1DIR |= BIT2 + BIT3; // P1.2 and P1.3 output as PWM
	P1SEL |= BIT2 + BIT3; // P1.2 and P1.3 options select
	TA0CCR0 = TIMER_PERIOD - 1; // PWM Period
	TA0CCR1 = PWM_LOGIC ( 0 ); // PWM Period
	TA0CCR2 = PWM_LOGIC ( 0 ); // PWM Period
	TA0CCTL1 = OUTMOD_7; // CCR1 reset/set
	TA0CCTL2 = OUTMOD_7; // CCR2 reset/set 512*75%=128
	TA0CTL = TASSEL__ACLK + MC__UP + TACLR; // ACLK, up mode, clear TAR
	#ifdef ADC_USE
	/***********
	 *  电流传感器  *
	 ***********/
	REFCTL0 &= ~REFMSTR; // 复位REFMSTR控制位，以使用ADC12_A参考电压控制寄存器
	ADC12CTL0 = ADC12SHT02 + ADC12ON; // 采样时间Tsample = 64个ADC12CLK, ADC12 on
	// ADC12MEMO0作为转换地址（缺省），脉冲采样模式（使用采样定时器）, 参考时钟源ACLK，单通道单次转换（缺省）
	ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHP + ADC12SSEL_1 + ADC12CONSEQ_0;
	ADC12MCTL0 = ADC12INCH_0 + ADC12SREF_0; // 输入通道0，参考电压（VR+ = AVcc, VR- = AVss）皆为缺省值，可注释掉
	ADC12IE |= BIT0; // Enable interrupt
	ADC12CTL0 |= ADC12ENC;  //ADC12ENC为低电平时相关控制寄存器才能被修改
	P6SEL |= BIT0; // P6.0 ADC option select
	#endif
	#ifdef UART_DEBUG
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
	#endif
	#ifdef FLOAT_FACTOR_USE
	P8DIR |= BIT1;
	P8OUT |= BIT1;
	#endif
	#ifdef LED_DEBUG
	/*********
	 *  LED  *
	 *********/
	P4DIR |= BIT7;
	P1DIR |= BIT0;
	#ifdef FLOAT_FACTOR_USE

	if ( P8IN & BIT2 ) {
		mode = 3;
		P1OUT |= BIT0;
		P4OUT |= BIT7;
		P8OUT &= ~BIT1;

	} else
	#endif
		if ( P6IN & BIT5 ) {
			mode = 1;
			P1OUT |= BIT0;
			P4OUT &= ~BIT7;

		} else {
			mode = 2;
			P4OUT |= BIT7;
			P1OUT &= ~BIT0;
		}

	#ifdef UART_DEBUG
	__bis_SR_register ( LPM0_bits + GIE ); // 进LPM0开总中断
	#else
	__bis_SR_register ( LPM3_bits + GIE ); // 进LPM3开总中断
	#endif
	#else
	#ifdef FLOAT_FACTOR_USE

	if ( P8IN & BIT2 )
		mode = 3;

	P8OUT &= ~BIT1;
	else
	#endif
		if ( P6IN & BIT5 )
			mode = 1;

		else
			mode = 2;

	__bis_SR_register ( LPM3_bits + GIE ); // 进LPM3开总中断
	#endif

	while ( 1 );
}
#ifdef ADC_USE
/**********************************************************************
*                               电流传感器                                *
**********************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
	/**
	*  @brief 电流传感器
	*/
	#pragma vector = ADC12_VECTOR
	__interrupt void ADC12_ISR ( void )
#elif defined(__GNUC__)
	void __attribute__ ( ( interrupt ( ADC12_VECTOR ) ) ) ADC12_ISR ( void )
#else
	#error Compiler not supported!
#endif
{
	switch ( __even_in_range ( ADC12IV, 34 ) ) {
		case 0:
			break; // Vector 0: No interrupt

		case 2:
			break; // Vector 2: ADC overflow

		case 4:
			break; // Vector 4: ADC timing overflow

		case 6: // Vector 6: ADC12IFG0
			#ifdef UART_DEBUG
			sprintf ( str, "%4d, %8f, %8f\r\n", ADC12MEM0, VOLTAGE, CURRENCY );
			int i;

			for ( i = 0; i < strlen ( str ); ++i ) {
				UCA0TXBUF = str[i];                  // TX -> RXed character
			}

			#endif

			switch ( mode ) {
				case 1:
				case 2:
					if ( CURRENCY < POWER_THRESHOLD ) // 断电
						isCharge = 0;

					break;
					#ifdef FLOAT_FACTOR_USE

				case 3:
					if ( CURRENCY < CHARGE_THRESHOLD ) // 远离充电站
						isCharge = 0;

					else   // 靠近充电站
						isCharge = 1;

					break;
					#endif
			}

			break;

		case 8:
			break; // Vector 8: ADC12IFG1

		default:
			break;
	}
}
#endif
/**********************************************************************
*                                看门狗                                 *
**********************************************************************/
// Watchdog Timer interrupt service routine
/**
  *  @brief 看门狗
  */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
	#pragma vector = WDT_VECTOR
	__interrupt void WDT_ISR ( void )
#elif defined(__GNUC__)
	void __attribute__ ( ( interrupt ( WDT_VECTOR ) ) ) WDT_ISR ( void )
#else
	#error Compiler not supported!
#endif
{
	switch ( mode ) {
		case 1:
			if ( isCharge ) {
				#ifdef ADC_USE
				ADC12CTL0 |= ADC12SC; // Start sampling/conversion
				#else

				if ( delayTurn < DELAY_TURN_MAX )
					delayTurn ++ ;

				else {
					isCharge = 0;
					#ifdef LED_DEBUG
					P1OUT &= ~BIT0;
					P4OUT &= ~BIT7;
					#endif
					TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( TIMER_PERIOD ) );
					TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( TIMER_PERIOD ) );
				}

				#endif

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
				#ifdef ADC_USE
				ADC12CTL0 |= ADC12SC; // Start sampling/conversion
				#else

				if ( delayTurn < DELAY_TURN_MAX )
					delayTurn ++ ;

				else {
					isCharge = 0;
					#ifdef LED_DEBUG
					P1OUT &= ~BIT0;
					P4OUT &= ~BIT7;
					#endif
					TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( TIMER_PERIOD ) );
					TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( TIMER_PERIOD ) );
				}

				#endif

			} else if ( delayTurn - DELAY_TURN_MAX < LAUNCH_TURN )
				delayTurn ++ ;

			else
				SearchRun ( 1 );

			break;
			#ifdef FLOAT_FACTOR_USE

		case 3:
			if ( isCharge )
				SearchRun ( 1 );

			else
				SearchRun ( 1.3 );

			ADC12CTL0 |= ADC12SC; // Start sampling/conversion
			SearchRun ( 1 );
			break;
			#endif
	}
}
/**********************************************************************
*                                 避障                                 *
**********************************************************************/
void go_straight ( FACTOR factor ) {
	TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
	TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
}
void turn_left ( FACTOR factor ) {
	TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( 400 * factor ) );
	TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
}
void turn_right ( FACTOR factor ) {
	TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
	TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( 400 * factor ) );
}
void turn_left_heavy ( FACTOR factor ) {
	TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( 420 * factor ) );
	TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
}
void turn_right_heavy ( FACTOR factor ) {
	TA0CCR1 = PWM_LOGIC ( ( unsigned int ) ( 511 * factor ) );
	TA0CCR2 = PWM_LOGIC ( ( unsigned int ) ( 420 * factor ) );
}
void SearchRun ( FACTOR factor ) {
	/*************
	*  左边碰到向左转  *
	*************/
	if ( S2 ) { //
		turn_left ( factor );
		return;
	}

	/*************
	*  右边碰到向右转  *
	*************/
	if ( S3 ) { //
		turn_right ( factor );
		return;
	}

	/***************
	*  最右边碰到重度右转  *
	***************/
	if ( S4 ) { //
		turn_right_heavy ( factor );
		return;
	}

	/***************
	*  最左边碰到重度左转  *
	***************/
	if ( S1 ) {
		turn_left_heavy ( factor );
		return;
	}

	/***********
	*  没碰到直走  *
	***********/
	if ( S2 == 0 && S3 == 0 ) {
		go_straight ( factor );
		return;
	}
}
