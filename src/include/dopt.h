 #include <windows.h>
 #include <stdio.h>
 #include <stdarg.h>
 #include <stdlib.h>
 #include <string.h>
 #include <conio.h>
 #include <ctype.h>
 #include <io.h>
 #include <process.h>
 #include <crtdbg.h>

 #include "def.h"
 #include "bitarray.h"
 #include "command.h"
 #include "message.h"
 #include "gcon.h"
 #include "comm.h"
 #include "ckbd.h"
 #include "value.h"
 #include "module.h"
 #include "mcore.h"

 extern GConsole      ScreenDevice;
 extern Comm          IODevice;
 extern CMessages    *pIOMessage;
 extern CModuleBase  *pModuleDevice;
 extern PDCPROC       Table[4][256];
 extern void          (*CPP_Waitproc)(void);
 extern int           Printable_arr[32];
 extern int           PRN_Cnt;
 extern unsigned char CPP_Waitbuf[],
                      CPP_ch;
 extern int           CPP_Waitlength,
                      CPP_Cur_State,
                      CPP_Waitbuf_Indx;
 extern unsigned char color_table[];


 void WN_Init(void);
 void WN_Close(void);
 void WN_Putch_Abs(int y,int x,int c);
 void WN_Putc(int );
 void WN_Puts_Abs_Fix(int y,int x,unsigned char*str,char attr);
 void WN_Puts_Abs(int y,int x,unsigned char*str);
 void WN_Puts_Abs_Ext(int y,int x,unsigned char*s,int len);
 void WN_Puts_Abs_Enh(int y,int x,unsigned char *str);
 void WN_Puts_Abs_Enh_Ext(int y,int x,unsigned char*str,int len);
 void WN_Fill_Abs(int y,int x,int len,int s);
 void WN_Fill_Abs_Enh(int y,int x,int len,int s);
 void WN_Set_Cursor(int y,int x);
 void WN_Set_Cursor_Shape(int a,int b);
 void WN_SW_Init(void);
 void WN_Get_Slock( int *,int *);
 void WN_Upd_Wndw(int y,int x,int c,int d);
 void WN_Redraw_Ch(int y,int x);
/*
 No need anymore
---------------------------------------------------
 void WN_Redraw_Partial_Line(int p1,int p2,int p3);
*/
 void WN_Redraw_Line(int count);
 void WN_Redraw_Client_Area(void);
 void WN_Redraw_All(void);
 void WN_Ctrl_SVW(int,int,int);
 int  WN_Calc_Dspl(int p1,int p2,int *p3,int *p4);
 int  WN_Calc_SVW(int p1);
 void WN_Redraw_Partial_Line_Fix(int p1,int p2,int line);

 void p_wn_ctrl_led(int a);                     //+
 void p_error_dump(void);
 void p_wn_redraw_characters_fix(int y,int count);
 void p_wn_redraw_characters(int,int,int);      //+
 void p_wn_redraw_cursor(void);                 //+
 void p_sv_clear_low_area(void);                //+


 void PDC_Backspace(void);     //+
 int  PDC_Xlat_22(int*,int*);       //+
 int  PDC_Xlat_222(int*,int*,int*); //+
 int  PDC_Xlat_224(int*,int*,int*); //+
 int  PDC_Xlat_311(int*,int*,int*); //+
 int  PDC_Test_Mode(void);     //+
 void PDC_Dummy(void);         //+
 void PDC_Dflt_Base_St(void);  //+
 void PDC_Bell(void);          //+
 void PDC_Dflt_Esc_St(void);   //+
 void PDC_Init(void);          //+
 void PDC_Hard_Reset(void);    //+
 void PDC_Software_Id(void);   //+
 void PDC_Software_Cf(void);   //+
 void PDC_Low_Case_Off(void);  //+
 void PDC_Low_Case_On(void);   //+
 void PDC_Insert_Off(void);    //+
 void PDC_Insert_On(void);     //+
 void PDC_Protect_Off(void);   //+
 void PDC_Protect_On(void);    //+
 void PDC_Keylock_Off(void);   //+
 void PDC_Keylock_On(void);    //+
 void PDC_Format_Off(void);    //+
 void PDC_Format_On(void);     //+
 void PDC_Wrt_Enh_Mem(void);   //+
 void PDC_CWrt_Enh_Mem(void);  //+
 void PDC_Wrt_Enh_Body(void);  //+
 void PDC_Wrt_Enh_Body_2(void);//+
 void PDC_Rd_Char_Mem(void);   //+
 void PDC_CRd_Char_Mem(void);  //+
 void PDC_Wrt_Chr_Mem(void);   //+
 void PDC_CWrt_Chr_Mem(void);  //+
 void PDC_Wrt_TChr_Mem(void);  //+
 void PDC_Setup_Comb(void);    //+
 void PDC_Comb_Chr_Mem(void);  //+
 void PDC_Wrt_Chr_Body(void);  //+
 void PDC_Wrt_Chr_Body_2(void);//+
 void PDC_Fill_Enh_Mem(void);  //+
 void PDC_CFill_Enh_Mem(void); //+
 void PDC_Ctrl_Led(void);      //+
 void PDC_Fill_Chr_Mem(void);  //+
 void PDC_CFill_Chr_Mem(void); //+
 void PDC_Long_Sync(void);     //+
 void PDC_Bootload(void);      //+
 void PDC_Test_On(void);       //+
 void PDC_Test_Off(void);      //+
 void PDC_Keyswitches(void);   //+
 void PDC_TDB_Features(void);  //+
 void PDC_Baudrate(void);      //+
 void PDC_Slct_Wndw(void);     //+
 void PDC_Page_Switch(void);   //+
 void PDC_Copy_Data(void);     //+
 void PDC_Def_VT220_Kbd(void); //+
 void PDC_Swap_Window(void);   //+
 void PDC_Dflt_Compr_St(void); //+
 void PDC_Compr_Char(void);    //+
 void PDC_Enh_Ctl_Prnt(void);  //+
 void PDC_Cpy_Dspl_Mem(void);  //+
 void PDC_Set_Tab(void);       //+
 void PDC_Reset_Tab(void);     //+
 void PDC_Cursor_Up(void);     //+
 void PDC_Cursor_Down(void);   //+
 void PDC_Cursor_Right(void);  //+
 void PDC_Cursor_Left(void);   //+
 void PDC_Set_Cursor(void);    //+
 void PDC_Csr_Antihome(void);  //+
 void PDC_Cursor_Home(void);   //+
 void PDC_Prev_Tab(void);      //+
 void PDC_Sense_Cursor(void);  //+
 void PDC_Ctrl_Cursor(void);   //+
 void PDC_Dflt_Def_St(void);   //+
 void PDC_Def_Chr_Gen(void);   //+
 void PDC_Char_Gen(void);      //+
 void PDC_Def_Pri_Part(void);  //+
 void PDC_Def_Pri_All(void);   //+
 void PDC_Def_Shf_Part(void);  //+
 void PDC_Def_Shf_All(void);   //+
 void PDC_Def_Ctl_Part(void);  //+
 void PDC_Def_Ctl_All(void);   //+
 void PDC_Def_Typ_Part(void);  //+
 void PDC_Def_Typ_All(void);   //+
 void PDC_Def_Pre_Part(void);  //+
 void PDC_Def_Pre_All(void);   //+
 void PDC_E_D_Pref_Cd(void);   //+
 void PDC_Carr_Return(void);   //+
 void PDC_Next_Tab(void);      //+
 void PDC_New_Line(void);      //+
 void PDC_Linefeed(void);      //+
 void PDC_New_Line(void);      //+
 void PDC_Cpy_Dspl_Prt(void);  //+
 void PDC_FreePrntBody(void);  //-
 void PDC_Ctl_Prnt_Int(void);  //+
 void PDC_Clr_All(void);       //+
 void PDC_Clr_Par(void);       //+
 void PDC_Clr_Line(void);      //+
 void PDC_Insert_Line(void);   //+
 void PDC_Del_Line(void);      //+
 void PDC_Del_Ch(void);        //+
 void PDC_Roll_Up(void);       //+
 void PDC_Roll_Down(void);     //+
 void PDC_Dir_Line_Ins(void);  //+
 void PDC_Dir_Line_Del(void);  //+
 void PDC_Next_Page(void);     //+
 void PDC_Prev_Page(void);     //+
 void PDC_Ctrl_SVW(void);      //+
 void PDC_Set_Scr_Lock(void);  //+
 void PDC_Set_Mem_Lock(void);  //+
 void PDC_FreePrntHead(void);  //+
 void PDC_FreePrntBody(void);  //+
 void PDC_Ctrl_Print(void);    //+
 void PDC_Sense_Print(void);   //+
 void PDC_PrnBufEmpty(void);   //+
 void PDC_Video_On(void);      //+
 void PDC_Video_Off(void);     //+
 int  Xlat_Nbr(unsigned char*,int,int *); //+
 void PDC_Set_Color_Table(unsigned char*buffer,int size);
 int  PDC_Get_Color_Table(unsigned char*buffer,int size);

 void CPP_Parser(unsigned char*,int);//+
 void CPP_Init(void);               // +

 int  PRN_Check_Print_Rdy(void);
 void PRN_Flush(void);
 void PRN_InBuf_Ch(int);
 void PRN_Init(void);
 void PDC_Cpy_Dspl_Prt(void);

 int RS232_Put_Str(unsigned char*);     //+
 int RS232_Put_Str(unsigned char*,int); //+
 int RS232_Put_Char(int);               //+
 int RS232_Set_Baudrate(unsigned char*);//+



 int   init_dopt_emulation(HWND wnd);

 char *trim(char*str);

/* Config function */
 Option*  get_option(char*name);
 int      parse_argument(char *argv);

/* Messages function */
 void InitDebugSection(void);
 int  message(int flag,char*frm,...);
// long sys_debug(int level,char*frm,...);
 int  message_trace(int flag,unsigned char*buf,int len);
/* Keyboard function */ 
 int           keyboard_input(int,int,int,int);
 int           keyboard_init(void);
 int           keyboard_lang_switch(void);
 int           keyboard_lang_set(int);
 char*         keyboard_error(void);
 int           keyboard_clear_flags(void);

 
