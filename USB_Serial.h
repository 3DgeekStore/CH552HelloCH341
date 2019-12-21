#ifndef USB_SERIAL_H_
#define USB_SERIAL_H_

void USBDeviceCfg();
void USBDeviceIntCfg();
void USBDeviceEndPointCfg();
void EcoSendData(PUINT8 SendBuf);
void SendData(PUINT8 SendBuf);
void RecieveData();
void	USBInterrupt();
void u16str(UINT16 i_data, char *str_t);

#endif
