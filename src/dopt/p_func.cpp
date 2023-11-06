
 #include "dopt.h"


 void p_sv_recover_low_area(void)
 {
 }
 void p_error_dump(void)
 {
  message(F_DEBUG,"p_error_dump: unsupport function");
 }

 void p_wn_redraw_cursor(void)
 {
  WN_Set_Cursor_Shape(gwin_f_crsr_start,gwin_f_crsr_end);
 }

 void p_wn_redraw_characters(int y,int x,int count)
 {
  int a,b;//,i;
  char *ptr,*ptr1;
  if(WN_Calc_Dspl(y,x,&a,&b))
  { 
   int offset = y*DOPTCOL+x;
   ptr    = (char*)(gwin_CHR_Mem+offset);
   ptr1   = (char*)(gwin_DAM_Mem+offset);
   ScreenDevice.OutputString(TRUE,x,a,ptr,ptr1,count);
  }
 }

 void p_wn_redraw_characters_fix(int y,int count)
 {
  int a,b;
  if(WN_Calc_Dspl(y,0,&a,&b))
  { 
   int offset = y*DOPTCOL;
   ScreenDevice.OutputLine(TRUE,a,(char*)&gwin_CHR_Mem[offset],(char*)&gwin_DAM_Mem[offset],count);
  }
 }

 void p_wn_redraw_enhancement(int y,int x,int count)
 {
  int a,b;
  char *ptr,*ptr1;
  if(WN_Calc_Dspl(y,x,&a,&b))
  {
   int offset = y*DOPTCOL+x;
   ptr    = (char*)(gwin_DAM_Mem+offset);
   ptr1    = (char*)(gwin_CHR_Mem+offset);
   ScreenDevice.OutputString(TRUE,x,a,ptr1,ptr,count);
  }
 }

 void p_wn_redraw_del_line(int y,int p2,int p3)
 {
  int a,b;
  if(WN_Calc_Dspl(y,0,&a,&b))
  {
   ScreenDevice.OutputChr(TRUE,a,DOPTCOL,color_table[8],' ');
  }
  return;
 }

 void p_wn_ctrl_led(int a)
 {
  return;
 }

 void p_wn_reset_leds(void)
 {
  return;
 }
 void p_sv_clear_low_area(void)
 {
 }
 void p_clear_low_area(int p1)
 {
 }
 void p_sv_recover_low(void)
 {
 }






