
 #include "dopt.h"

 int PRN_Cnt=1;
 int Printable_arr[32];

 int PRN_Check_Print_Rdy(void)
 {
  long status = get_option("PRINT_status")->i;

  return status; // if not busy

/*
//  return 2; // if simple busy  ?????
//  return 0; // busy
   and ax,0x38
   cmp ax,0x10
   je label_1
   return 1;
label_1:
   test ax,0x80
   jne label_2
   return 0;
label_2:
   return 2;
   */
   return 1;
 }
 void PRN_Flush(void)
 {
 }
 void PRN_InBuf_Ch(int)
 {
 }
 void PRN_Init(void)
 {
 }
 void PDC_Cpy_Dspl_Prt(void)
 {
 }
