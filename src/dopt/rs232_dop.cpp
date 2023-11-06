
 #include "dopt.h"

 int RS232_Put_Str(unsigned char*str,int len)
 {
  return IODevice.Write(str,len);
 }
 int RS232_Put_Str(unsigned char*str)
 {
  return IODevice.Write(str,strlen((char*)str));
 }
 int RS232_Put_Char(int ch)
 {
  return IODevice.Write(&ch,1);
 }
 int RS232_Set_Baudrate(unsigned char*)
 {
  return 1;
 }
