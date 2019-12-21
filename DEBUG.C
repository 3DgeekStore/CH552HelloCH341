/********************************** (C) COPYRIGHT *******************************
* File Name          : DEBUG.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 DEBUG Interface
                     (1)¡¢Serial port 0 prints information, baud rate is variable;              				   
*******************************************************************************/

#include "CH554.H"
#include "DEBUG.H"

/*******************************************************************************
* Function Name  : CfgFsys( )
* Description    : CH554 Clock selection and configuration function. By default, the internal
* crystal oscillator is 12MHz. If you define FREQ_SYS, you can
                   PLL_CFG and CLOCK_CFG configuration, the formula is as follows:
                   Fsys = (Fosc * ( PLL_CFG & MASK_PLL_MULT ))/(CLOCK_CFG & MASK_SYS_CK_DIV);
                   Specific clock needs to be configured by yourself
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void	CfgFsys( )  
{
// 		SAFE_MOD = 0x55;
// 		SAFE_MOD = 0xAA;
//     CLOCK_CFG |= bOSC_EN_XT;                          //Enable external crystal
//     CLOCK_CFG &= ~bOSC_EN_INT;                        //Turn off the internal crystal   
		SAFE_MOD = 0x55;
		SAFE_MOD = 0xAA;
 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x07;  // 32MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;  // 24MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x05;  // 16MHz	
//		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x04;  // 12MHz
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x03;  // 6MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x02;  // 3MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x01;  // 750KHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x00;  // 187.5MHz		
		SAFE_MOD = 0x00;
}

/*******************************************************************************
* Function Name  : mDelayus(UNIT16 n)
* Description    : us Delay function
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/ 
void	mDelayuS( UINT16 n )  // Delay in uS
{
#ifdef	FREQ_SYS
#if		FREQ_SYS <= 6000000
		n >>= 2;
#endif
#if		FREQ_SYS <= 3000000
		n >>= 2;
#endif
#if		FREQ_SYS <= 750000
		n >>= 4;
#endif
#endif
	while ( n ) {  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
		++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef	FREQ_SYS
#if		FREQ_SYS >= 14000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 16000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 18000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 20000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 22000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 24000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 26000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 28000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 30000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 32000000
		++ SAFE_MOD;
#endif
#endif
		-- n;
	}
}

/*******************************************************************************
* Function Name  : mDelayms(UNIT16 n)
* Description    : ms Delay function
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/
void	mDelaymS( UINT16 n )                                                  // Delay in mS
{
	while ( n ) {
#ifdef	DELAY_MS_HW
		while ( ( TKEY_CTRL & bTKC_IF ) == 0 );
		while ( TKEY_CTRL & bTKC_IF );
#else
		mDelayuS( 1000 );
#endif
		-- n;
	}
}                                         

/*******************************************************************************
* Function Name  : CH554UART0Alter()
* Description    : CH554 Serial port 0 pin mapping,
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART0Alter()
{
    PIN_FUNC |= bUART0_PIN_X;                                                  //Serial port mapping to P1.2 and P1.3
}

/*******************************************************************************
* Function Name  : mInitSTDIO()
* Description    : CH554Serial port 0 is initialized, T1 is used as the baud rate 
* generator for UART0 by default, and T2 can also be used as a baud rate generator
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	mInitSTDIO( )
{
    UINT32 x;
    UINT8 x2; 

    SM0 = 0;
    SM1 = 1;
    SM2 = 0;                                                                   //Serial port 0 use mode 1
                                                                               //Using Timer1 as a baud rate generator
    RCLK = 0;                                                                  //UART0 receive clock
    TCLK = 0;                                                                  //UART0 transmit clock
    PCON |= SMOD;
    x = 10 * FREQ_SYS / UART0_BUAD / 16; //If you change main frequency, be careful not to overflow the value of x                           
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                       //rounding

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;              //0X20, Timer1 as 8-bit auto reload timer
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;                                        //Timer1 clock selection
    TH1 = 0-x;                                                                 //12MHz crystal, buad / 12 is the actual baud rate
    TR1 = 1;                                                                   //Start timer 1
    TI = 1;
    REN = 1;                                                                   //Serial port 0 receive enable
}

/*******************************************************************************
* Function Name  : CH554UART0RcvByte()
* Description    : CH554UART0 Receive a byte
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8  CH554UART0RcvByte( )
{
    while(RI == 0);                                                            //Query reception, no interruption method required
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : CH554UART0SendByte(UINT8 SendDat)
* Description    : CH554UART0 Send a byte
* Input          : UINT8 SendDat£» Data to send
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART0SendByte(UINT8 SendDat)
{
	SBUF = SendDat; //Inquiry transmission, the interruption method can not use the following two statements, but TI = 0 before sending
	while(TI ==0);
	TI = 0;
}

/*******************************************************************************
* Function Name  : UART1Setup()
* Description    : CH554 Serial port 1 initialization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	UART1Setup( )
{
  U1SM0 = 0;                                                                   //UART1 selects 8 data bits
  U1SMOD = 1;                                                                  //Fast mode
  U1REN = 1;                                                                   //Enable receive
  SBAUD1 = 0 - FREQ_SYS/16/UART1_BUAD;
  IE_UART1 = 1;
  EA = 1;
}

/*******************************************************************************
* Function Name  : CH554UART1RcvByte()
* Description    : CH554UART1 Receive a byte
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8  CH554UART1RcvByte( )
{
    while(U1RI == 0);                 //Query reception, no interruption method required
    U1RI = 0;
    return SBUF1;
}

/*******************************************************************************
* Function Name  : CH554UART1SendByte(UINT8 SendDat)
* Description    : CH554UART1 Send a byte
* Input          : UINT8 SendDat£»Data to send
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART1SendByte(UINT8 SendDat)
{
	SBUF1 = SendDat;      //Inquiry transmission, the interruption method can not use the following two statements, but TI = 0 before sending
	while(U1TI ==0);
	U1TI = 0;
}

/*******************************************************************************
* Function Name  : CH554WatchDog(UINT8 mode)
* Description    : CH554 Watchdog mode setting
* Input          : UINT8 mode 
                   0  timer
                   1  watchDog
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WatchDog(UINT8 mode)
{
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xaa;                                                             //Enter safe mode
  if(mode){
    GLOBAL_CFG |= bWDOG_EN;
  }
  else GLOBAL_CFG &= ~bWDOG_EN;	
  SAFE_MOD = 0x00;                                                             //exit safe Mode
  WDOG_COUNT = 0;                                                              //Watchdog assigned initial value
}

/*******************************************************************************
* Function Name  : CH554WatchDogFeed(UINT8 tim)
* Description    : CH554Watchdog feeding dog
* Input          : UINT8 tim Watchdog reset time setting
                   00H(6MHz)=2.8s
                   80H(6MHz)=1.4s
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WatchDogFeed(UINT8 tim)
{
  WDOG_COUNT = tim;                                                             //Watchdog assigned initial value	
}