#include "CH554.H"
#include "Debug.H"
#Include <string.h>
#include "USB_Serial.h"

main()
{
  CfgFsys( );                                   //Clock selection 
	mDelaymS(30);                                 //Power-on delay
	USBDeviceCfg();                               //Device mode configuration
  USBDeviceEndPointCfg();			  //Endpoint configuration	
	USBDeviceIntCfg();
  UEP0_T_LEN = 0;
  UEP1_T_LEN = 0;	                          //Pre-use send length must be cleared	
  UEP2_T_LEN = 0;	    	
  while(1)
    {
 	  SendData("Hello World \n\r");
		mDelaymS(10);                           //Analog microcontroller to do other things
  
    }
}