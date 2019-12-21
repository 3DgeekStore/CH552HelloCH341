/* debugging */
/* Provide printf subroutines and delay functions */

#ifndef	__DEBUG_H__
#define __DEBUG_H__

//Define function return value
#ifndef  SUCCESS
#define  SUCCESS  0
#endif
#ifndef  FAIL
#define  FAIL    0xFF
#endif

//Define timer start
#ifndef  START
#define  START  1
#endif
#ifndef  STOP
#define  STOP    0
#endif

#ifndef  DE_PRINTF
#define  DE_PRINTF     1
#endif
#define	 FREQ_SYS	     32000000	         //System main frequency 12MHz
#ifndef  UART0_BUAD
#define  UART0_BUAD    57600
#define  UART1_BUAD    57600
#endif

void	CfgFsys( );                        //CH554 clock selection and configuration
void	mDelayuS( UINT16 n );              // Delay in uS
void	mDelaymS( UINT16 n );              // Delay in mS
void  CH554UART0Alter();                 //CH554 serial port 0 pin is mapped to P0.2 / P0.3
void	mInitSTDIO( );                      //T1 as a baud rate generator
UINT8 CH554UART0RcvByte( );              //CH554 UART0 query mode receives one byte
void  CH554UART0SendByte(UINT8 SendDat); //CH554UART0 sends a byte

void	UART1Setup( );                     //
UINT8 CH554UART1RcvByte( );              //CH554 UART1 query mode receives one byte
void  CH554UART1SendByte(UINT8 SendDat); //CH554UART1 sends a byte

void CH554WatchDog(UINT8 mode);          //CH554 watchdog mode setting
void CH554WatchDogFeed(UINT8 tim);       //CH554 watchdog feeds the dog
#endif
