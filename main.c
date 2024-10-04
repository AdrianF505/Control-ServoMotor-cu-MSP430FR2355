#include <msp430.h>
volatile char st[9] ={'S','T','A','N','G','A','\n','\r'};
volatile char dr[10]={'D','R','E','A','P','T','A','\n','\r'};
int i=0;
int j=0;
int Dir=0;
int rtc=1;
volatile char Control;
#define DTC_delta 84
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                          // Stop watchdog timer
    FRCTL0 = FRCTLPW | NWAITS_2;

    __bis_SR_register(SCG0);                           // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source
    CSCTL0 = 0;                                        // clear DCO and MOD registers
    CSCTL1 |= DCORSEL_5;                               // Set DCO = 16MHz
    CSCTL2 = FLLD_0 + 487;                             // DCOCLKDIV = 16MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                           // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;        // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
     CSCTL5= DIVM_2 | DIVS_0;
    // MCLK=4Mhz   SMCLK=1Mhz
   // MCLK -> 3.0
    P3DIR |=BIT0;
    P3SEL0 |=BIT0;
    P3SEL1 &=~BIT0;

    // SMCLK ->3.4
    P3DIR |=BIT4;
    P3SEL0 |=BIT4;
    P3SEL1 &=~BIT4;

    //P2.3 buton
    P2OUT |= BIT3;                          // Configure P1.3 as pulled-up
    P2DIR &=~BIT3;
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES |= BIT3;                          // P2.3 Hi/Low edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    //P2.3 buton
    P4OUT |= BIT1;                          // Configure P1.3 as pulled-up
    P4DIR &=~BIT1;
    P4REN |= BIT1;                          // P2.3 pull-up register enable
    P4IES |= BIT1;                          // P2.3 Hi/Low edge
    P4IE |= BIT1;                           // P2.3 interrupt enabled
    //P1.0 si P6.6 led directie
    P1DIR |= BIT0;
    P1OUT |= BIT0;// 0 logic
    P6DIR |= BIT6;
    P6OUT |=BIT6;// 0 logic

    P4SEL0 |= BIT2 | BIT3; // set 2-UART pin as second function
    // Baud Rate calculation. Referred to UG 17.3.10
    // (1) N=1000000/9600=104.16 //17.36
    // (2) OS16=1, UCBRx=INT(N/16)=6 //1
    //UCBRFx = INT([(N/16)– INT(N/16)] × 16)=6 //
           // Configure UART

           UCA1CTLW0 |= UCSWRST;
           UCA1CTLW0 |= UCSSEL_2; // set SMCLK as BRCLK
           UCA1BR0 = 26; // UCBRx = 26
           UCA1BR1 = 0; // UCBRx = 0
           UCA1MCTLW = 0xB601; // UCBRSx = 0xb6, UCBRFx = 0, UCOS16 = 1

           UCA1CTLW0 &= ~UCSWRST; // Initialize eUSCI
           UCA1IE |= UCRXIE; // Enable USCI_A1 RX interrupt

    PMMCTL0_H = PMMPW_H;                      // Unlock the PMM registers
    PMMCTL2 = INTREFEN | REFVSEL_0;           // Enable internal 2V reference
    while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles


    // P6.4 iesire TimerB 3 -> CCR5 -> TB3.5
        P6DIR |= BIT4;
        P6SEL1 &=~BIT4;
        P6SEL0 |=BIT4;

        // CCR0 = f_clk/f_pwm = 1*10^6Hz/0.05*10^3Hz = 20000
        // CCR0 = 20000 -> perioade de SMCLK= 1MHz -> f_pwm=50Hz
        //// f_pwm =50Hz -> factor umplere DTC = 5-10%
        // ideal
        // CCR0 = 20000
        // CCR5 = 1000
        // real
        // CCR0 = 16800
        // CCR5 = 840

            // Setup Timer3_B
            TB3CCR0 = 16800-1;                                  // PWM Period
            TB3CCTL5 = OUTMOD_7;                              // CCR5 reset/set
            TB3CCR5 = 840;                                     // CCR5 PWM duty cycle
            TB3CTL = TBSSEL_2 | MC_1 | TBCLR;                 //SMCLK, up mode, clear TBR

            // 1000/1.000.000 * 1us = 1 sec.

            RTCMOD = 1000-1;

            SYSCFG2 |= RTCCKSEL_0;                    // Select ACLK as RTC clock

            RTCCTL = RTCSS_1 | RTCSR | RTCPS__1000  | RTCIE;



    PM5CTL0 &= ~LOCKLPM5;                              // Disable the GPIO power-on default high-impedance mode
                                                       // to activate previously configured port settings

    P2IFG &= ~BIT3;
    P4IFG &= ~BIT1;


    __bis_SR_register(LPM3_bits | GIE);                     // Enter LPM3
}
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        while(!(UCA1IFG&UCTXIFG));
        UCA1TXBUF = UCA1RXBUF;
        Control=UCA1RXBUF;
        j = Control - '0';
        if(j==4){
            rtc=1;
            Dir=0;
        }
        switch(Dir){
        case 1:if(TB3CCR5>=840 && TB3CCR5<=2436 && j<=3){
            for(i=1;i<=j;i++){
            TB3CCR5 += i*DTC_delta;}}
            else{TB3CCR5=840;}break;
        case 2: if(TB3CCR5>=840 && TB3CCR5<=2436 && j<=3){
            for(i=1;i<=j;i++){
            TB3CCR5 -= i*DTC_delta;}}
            else{TB3CCR5=2436;}break;
        default: Dir=0;break;
        }

        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}
// Port 2 interrupt service routintere
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
      rtc=0;
        for(i=0;i<sizeof(st);i++){
            while(!(UCA1IFG&UCTXIFG));
            UCA1TXBUF=st[i];
        }
        Dir=2;

        P1OUT &= ~BIT0;
        P6OUT |= BIT6;

    P2IFG &=~BIT3;
    }
// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
        rtc=0;
        for(i=0;i<sizeof(dr);i++){
            while(!(UCA1IFG&UCTXIFG));
            UCA1TXBUF=dr[i];
        }
        Dir=1;

        P6OUT &= ~BIT6;
        P1OUT |= BIT0;
    P4IFG &=~BIT1;
    }
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)

{
    switch(__even_in_range(RTCIV,RTCIV_RTCIF))
    {
        case  RTCIV_NONE:   break;          // No interrupt
        case  RTCIV_RTCIF:
                if(TB3CCR5 >=840 && TB3CCR5<=2436 && rtc==1){
                    TB3CCR5 += DTC_delta;
                }
                else if(rtc ==1){
                    TB3CCR5 = 840;}



        default: break;
    }
}
