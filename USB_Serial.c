#include"CH554.H"
#include"Debug.H"
#include<string.h>
#include "USB_Serial.h"

#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE

UINT8X	Ep0Buffer[THIS_ENDP0_SIZE] _at_ 0x0000;
UINT8X	Ep2Buffer[2*MAX_PACKET_SIZE] _at_ 0x0008;
UINT8X  Ep1Buffer[MAX_PACKET_SIZE] _at_ 0x00a0;

UINT8	  SetReqtp,SetupReq,SetupLen,UsbConfig,Flag;
PUINT8  pDescr;	                                                               
UINT8   num = 0;
UINT8   LEN = 0;
USB_SETUP_REQ	           SetupReqBuf;   

#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)															//cast


UINT8C DevDesc[18]={0x12,0x01,0x10,0x01,0xff,0x00,0x02,0x08,                   //Device descriptor
                    0x86,0x1a,0x23,0x55,0x04,0x03,0x00,0x00,
                    0x00,0x01};

UINT8C CfgDesc[39]={0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //Configuration descriptor, interface descriptor, endpoint descriptor
	            0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,           
                    0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //Bulk upload endpoint
		    0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //Batch downlink endpoint      
		    0x07,0x05,0x81,0x03,0x08,0x00,0x01};                       //Interrupt upload endpoint

UINT8C DataBuf[26]={0x30,0x00,0xc3,0x00,0xff,0xec,0x9f,0xec,0xff,0xec,0xdf,0xec,
                    0xdf,0xec,0xdf,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,
                    0xff,0xec};
UINT8 RecBuf[64];
										
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Function name	:USBDeviceCfg()
Description		:USB device mode configuration

DataSheet			:PP40-46
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void USBDeviceCfg()
{
		USB_CTRL = 0x00;
		USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY| bUC_DMA_EN;  //USB device and internal pull-up enable, automatically return to NAK before interrupt flag is cleared during interrupt
		USB_DEV_AD = 0x00;

//  USB_CTRL |= bUC_LOW_SPEED;    						//1.5Mbps
//  UDEV_CTRL |= bUD_LOW_SPEED;                                  

    USB_CTRL &= ~bUC_LOW_SPEED;								//12Mbps
    UDEV_CTRL &= ~bUD_LOW_SPEED;   
		UDEV_CTRL |= bUD_PD_DIS;										//
		UDEV_CTRL |= bUD_PORT_EN;   							//Enable physical port
}

/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description    : USB device mode interrupt initialization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceIntCfg()
{
    USB_INT_EN |= bUIE_SUSPEND;              //Enable device hang interrupt                                  
    USB_INT_EN |= bUIE_TRANSFER;             //Enable USB transfer completion interrupt                                  
    USB_INT_EN |= bUIE_BUS_RST;              //Enable device mode USB bus reset interrupt                                  
    USB_INT_FG |= 0x1F;                      //Clear interrupt flag                                  
    IE_USB = 1;                              //Enable USB interrupt                                  
    EA = 1; 																                                   
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Function name	:USBDeviceEndPointCfg()
Description		:USB device mode endpoint configuration

DataSheet			:PP40-46
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void USBDeviceEndPointCfg()
{

    UEP2_DMA = Ep2Buffer;                                       //Endpoint 2 data transfer address     																	                                         
    UEP2_3_MOD |= bUEP2_TX_EN;                                  //Endpoint 2 send enable          
    UEP2_3_MOD |= bUEP2_RX_EN;                                  //Endpoint 2 Receive Enable        
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;                               //Endpoint 2 single 64 byte send buffer, single 64 byte receive buffer, a total of 128 bytes           
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;	//Endpoint 2 automatically flips the sync flag, IN transaction returns NAK, OUT returns ACK
    
		UEP1_DMA = Ep1Buffer;
		UEP4_1_MOD |= bUEP1_TX_EN; 
		//  UEP4_1_MOD |= bUEP1_RX_EN;  
		UEP4_1_MOD &= ~bUEP1_BUF_MOD;
		UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
	
		UEP0_DMA = Ep0Buffer;																				//Endpoint 0 data transfer address
		UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);									//Endpoint 0 single 64 byte send and receive buffer
		UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;									//OUT transaction returns ACK, IN transaction returns NAK
}


/*******************************************************************************
* Function Name  : EcoSendData( PUINT8 SendBuf )
* Description    : Eco Send data to the host serial port
* Input          : PUINT8 SendBuf
* Output         : None
* Return         : None
*******************************************************************************/
/*void EcoSendData( PUINT8 SendBuf)
{
	
	 if(Flag==1)                             
	 {
     while(LEN > 32){		 
     memcpy(&Ep2Buffer[MAX_PACKET_SIZE],SendBuf,32);
	   UEP2_T_LEN = 32;
	   UEP2_CTRL &= ~(bUEP_T_RES1 | bUEP_T_RES0);								// means reply ACK or ready
     while(( UEP2_CTRL & MASK_UEP_T_RES ) == UEP_T_RES_ACK);  //
     LEN -= 32;
     }
     memcpy(&Ep2Buffer[MAX_PACKET_SIZE],SendBuf,LEN);
	   UEP2_T_LEN = LEN;
	   UEP2_CTRL &= ~(bUEP_T_RES1 | bUEP_T_RES0);		 		
     Flag = 0;		 
   }
}
*/

/*******************************************************************************
* Function Name  : SendData( PUINT8 SendBuf )
* Description    : Send data to the host serial port
* Input          : PUINT8 SendBuf
* Output         : None
* Return         : None
*******************************************************************************/
void SendData(PUINT8 SendBuf)
{
		UINT8 SendLEN = 0;
		
		for(SendLEN=0; SendBuf[SendLEN]!='\0'; ++SendLEN);
	
     while(SendLEN > 32){		 
     memcpy(&Ep2Buffer[MAX_PACKET_SIZE],SendBuf,32);
	   UEP2_T_LEN = 32;
	   UEP2_CTRL &= ~(bUEP_T_RES1 | bUEP_T_RES0);								// means reply ACK or ready
     while(( UEP2_CTRL & MASK_UEP_T_RES ) == UEP_T_RES_ACK);  //
     SendLEN -= 32;
     }
     memcpy(&Ep2Buffer[MAX_PACKET_SIZE],SendBuf,SendLEN);
	   UEP2_T_LEN = SendLEN;
	   UEP2_CTRL &= ~(bUEP_T_RES1 | bUEP_T_RES0);		 		
     Flag = 0;		 
}

/*******************************************************************************
* Function Name  : RecieveData()
* Description    : USB device mode endpoint configuration
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RecieveData()
{
	  memcpy(RecBuf,Ep2Buffer,USB_RX_LEN); 
	  UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK; //Default response ACK
	  Flag = 1;
}

/**********************************************************************************
* Function Name  : u16str()
* Description    : Convert Uint64 type to char type
* Input          : UINT16 i_data, char *str_t
* Output         : None
* Return         : None
***********************************************************************************/
void u16str(UINT16 i_data, char *str_t)
{
	int n,i;
	if(str_t)
	{
		str_t[0] = '\0';
		n = 0;
		
		do
		{
			i = n;
			do
			{
				str_t[i+1] = str_t[i];
			} while (i--);
			str_t[0] = 0x30+(i_data % 10);
			i_data /= 10;
			n++;
		} while (i_data);
	}
}



/*******************************************************************************
* Function Name  : USBInterrupt()
* Description    : USB interrupt handler
*******************************************************************************/



void	USBInterrupt( void ) interrupt INT_NO_USB using 1                       
{   
	UINT8 len; 
	if(UIF_TRANSFER)                                                              {
    switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
    {
			 case UIS_TOKEN_OUT | 2:                                                				 
						LEN = USB_RX_LEN; 
			      RecieveData();
            //EcoSendData(RecBuf);			 
						break;
		   case UIS_TOKEN_IN | 2:                                                 
            UEP2_T_LEN = 0;	                                                  						 
	          UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;         					 
    			  UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;            
						break;
    	 case UIS_TOKEN_SETUP | 0:                                                 
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {   
							 SetReqtp = UsbSetupBuf->bRequestType;
               SetupLen = UsbSetupBuf->wLengthL;
							
               len = 0;                                                                                                                        
               SetupReq = UsbSetupBuf->bRequest;
               if(SetReqtp == 0xc0)
						   {
								  Ep0Buffer[0] = DataBuf[num];
								  Ep0Buffer[1] = DataBuf[num+1];
								  len = 2;
								  if(num<24)
								  {	
								    num += 2;
									}
									else
									{
										num = 24;
									}
						   }
					     else if(SetReqtp == 0x40)
						   {
							    len = 9;                                                        
						   }
						   else
						   { 
							    switch(SetupReq)                                                
							    {
								     case USB_GET_DESCRIPTOR:
											    switch(UsbSetupBuf->wValueH)
											    {
													   case 1:	                                            
																 pDescr = DevDesc;                                
																 len = sizeof(DevDesc);								       
													   break;	 
													   case 2:									                            
																 pDescr = CfgDesc;                                
																 len = sizeof(CfgDesc);
													   break;	
													   default:
																 len = 0xff;                                      
													   break;
											     }
									         if ( SetupLen > len ) SetupLen = len;                  
									         len = SetupLen >= 8 ? 8 : SetupLen;                    
									         memcpy(Ep0Buffer,pDescr,len);                          
									         SetupLen -= len;
									         pDescr += len;
										       break;						 
							        case USB_SET_ADDRESS:
										       SetupLen = UsbSetupBuf->wValueL;                       
										       break;
							        case USB_GET_CONFIGURATION:
									         Ep0Buffer[0] = UsbConfig;
									         if ( SetupLen >= 1 ) len = 1;
									         break;
							        case USB_SET_CONFIGURATION:
									         UsbConfig = UsbSetupBuf->wValueL;
									         break;
							        default:
										       len = 0xff;                                            
										       break;    
							       }
					        }
				      }
					    else
					    {
							    len = 0xff;                                                     
					    }

						  if(len == 0xff)
						  {
								  SetupReq = 0xFF;
								  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL				     
						  }
						  else if(len <= 8)                                                   
						  {
								  UEP0_T_LEN = len;
								  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;  
						  }
						  else
						  {
								  UEP0_T_LEN = 0;                                                       
								  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;  				     
						  }
					    break;
				 case UIS_TOKEN_IN | 0:                                                         //endpoint0 IN
						  switch(SetupReq)
						  {
							   case USB_GET_DESCRIPTOR:
								      len = SetupLen >= 8 ? 8 : SetupLen;                               
											memcpy( Ep0Buffer, pDescr, len );                                 
											SetupLen -= len;
											pDescr += len;
											UEP0_T_LEN = len;
											UEP0_CTRL ^= bUEP_T_TOG;                                          
								      break;
							   case USB_SET_ADDRESS:
											USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
											UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
								      break;
							   default:
								      UEP0_T_LEN = 0;                                                
								      UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
								      break;
						  }
						  break;
				 case UIS_TOKEN_OUT | 0:                                                 // endpoint0 OUT
							len = USB_RX_LEN;
							UEP0_T_LEN = 0;                                                    
							UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_ACK;                         									
						  break;
					default:
						  break;
				}
				UIF_TRANSFER = 0;                                         
    }
    if(UIF_BUS_RST)                                             
    {
			USB_DEV_AD = 0x00;
			UIF_SUSPEND = 0;
			UIF_TRANSFER = 0;
			UIF_BUS_RST = 0;                                     
    }
	  if (UIF_SUSPEND) 
		{                                                      
			UIF_SUSPEND = 0;
			if ( USB_MIS_ST & bUMS_SUSPEND ) 
			{                                                                          
				while ( XBUS_AUX & bUART0_TX );                                          
				SAFE_MOD = 0x55;
				SAFE_MOD = 0xAA;
				WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;                                  
				PCON |= PD;                                                              
				SAFE_MOD = 0x55;
				SAFE_MOD = 0xAA;
				WAKE_CTRL = 0x00;
			}
    } 
	  else 
	  {                                                                            
		  USB_INT_FG = 0x00;                                                         
	  }      
}
