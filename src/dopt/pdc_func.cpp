
 #include "dopt.h"


#define misdigit(a) ((a)>='0'&&(a)<='9')
 static int   LED_Flag;
 static int   _chr_hex[48];
 static int   v_video_on;
 static int   v_old_lowcase_mode;
 static int   PDC_Col;
 static int   PDC_Row;

 unsigned char color_table[]=
 {
  0x0F,0x8f,0x30,0xb0,0x1f,0x9f,0x2f,0xAf,0x07,0x87,
  0x70,0xf0,0x17,0x97,0x7F,0xFF,0xcf,0x8f,0xaF,0xae,
  0x9e,0x9f,0xfa,0xfe,0xcf,0x87,0xaf,0xae,0x9e,0x9f,
  0xfa,0xfe
 };

 int _table_char[48] = 
 {
  '\x0','A','B','C','D','E','F','G','H',
  'I','J','K','L','M','N','O','P','Q','R',
  'S','T','U','V','W','X','Y','Z','0','1',
  '2','3','4','5','6','7','8','9',':',';',
  '\\',',','.','+','-','/','!','?','\x0'
 };

 static void* _mmemset(void*dest,int c,size_t sz)
 {
  return memset(dest,c,sz);
  for(unsigned int i = 0;i<sz;i++)
  {
   ((char*)dest)[i] = c;
  }
  return dest;
 }

 void Word_to_Ascii(int p1,char*str,int p3)
 {
  str--;
  for(;p3>0;p3--)
  {
   str[p3] = p1%10+'0';
   p1 = p1/10;
  }
 }

 int Xlat_Nbr(unsigned char*ptr,int p2,int *p3)
 {
  int i,n;
  *p3=0;
  for(i=0;i<p2;i++)
  {
   register int pz = (int)ptr[i];
   if(!misdigit(pz)) return 0;
   n = ptr[i]-'0';
   *p3 = *p3*10+n;
  }
  return 1;
 }

 int Chr2_to_Hex(unsigned char *ptr,int *p1)
 {
   int a;
   a  =  (_chr_hex[*ptr]<<4)+_chr_hex[*(ptr+1)];
   if(a>255) return 0;
   *p1 = a;
   return 1;
 }

 int Chr_to_Hex(int p1,int*p2)
 {
  *p2 = _chr_hex[p1];
  if(*p2>=0x63) return 0;
  return 1;
 }

// 21022000 +
 void PDC_Backspace(void)
 {
  int a,b;
  int offset;        // bp-4
  a = gwin_Cur_Row; // bp-1
  b = gwin_Cur_Col; // bp-2
  if(a!=0||b!=0)
  {
   if(gwin_Format_Mode!=0||b!=0)
   {
    if(b==0)
    {
     b=DOPTCOL;
     a--;
    }
    b--;
    offset = a*DOPTCOL+b;
    if(gwin_Format_Mode!=0)
    {
     if(gwin_PAM_Mem[offset]) return;
    }else
     gwin_PAM_Mem[offset] = (char)gwin_Protect_Mode;

    gwin_CHR_Mem[offset] = ' ';
    WN_Redraw_Ch(a,b);
    WN_Set_Cursor(a,b);
   }
  }
 }

 int PDC_Xlat_22(int *p1,int *p2)
 {
  if(Xlat_Nbr(CPP_Waitbuf,2,p1))
  {
    if(Xlat_Nbr(CPP_Waitbuf+2,2,p2)) return 0;
  }
  return 1;
 }

 int PDC_Xlat_222(int *p1,int *p2,int *p3)
 {
  if(Xlat_Nbr(CPP_Waitbuf,2,p1))
  {
   if(Xlat_Nbr(CPP_Waitbuf+2,2,p2))
   {
    if(Xlat_Nbr(CPP_Waitbuf+4,2,p3)) return 0;
   }
  }
  return 1;
 }

 int PDC_Xlat_224(int*p1,int*p2,int *p3)
 {
  if(Xlat_Nbr(CPP_Waitbuf,2,p1))
  {
   if(Xlat_Nbr(CPP_Waitbuf+2,2,p2))
   {
    if(Xlat_Nbr(CPP_Waitbuf+4,4,p3)) return 0;
   }
  }
  return 1;
 }

 int PDC_Xlat_311(int *p1,int *p2,int *p3)
 {
  if(Xlat_Nbr(CPP_Waitbuf,3,p1))
  {
   if(Chr_to_Hex(*(CPP_Waitbuf+3),p2))
   {
    if(Chr_to_Hex(*(CPP_Waitbuf+4),p3)) return 0;
   }
  }
  return 1;
 }

// 21022000 +  function write
 int PDC_Xlat_11(int *p1,int*p2,int offset)
 {
  if(Chr_to_Hex(CPP_Waitbuf[offset],p1))
  {
   if(Chr_to_Hex(CPP_Waitbuf[offset+1],p2))
   {
    return 0;
   }
  }
  return 1;
 }

// 21022000 +
 void PDC_Dflt_Base_St(void)
 {
  if(CPP_ch<' ')
  {
   if(!Printable_arr[CPP_ch]) return;
  }
  WN_Putc(CPP_ch);
 }
// 21022000
 void PDC_Bell(void)
 {
 //  printf("\x7");
 }
// 21022000
 void PDC_Init(void)
 {
  int a,b;
  if(!PRN_Check_Print_Rdy()) PRN_Flush();
  PDC_Format_Off();
  PDC_Protect_On();
  PDC_Insert_Off();
  PDC_Keylock_Off();
  PDC_Low_Case_Off();
  WN_Get_Slock(&a,&b);
  WN_Ctrl_SVW(a,0,0);
  gwin_ESC_u_Flag = 0;
 }

 void PDC_Hard_Reset(void)
 {
  WN_Close();
  WN_SW_Init();
  WN_Puts_Abs(0,0,(unsigned char*)"0000\x20\x20\x20\x20\x20\x20\x20\x20\x20");
  Sleep(100); // 0.1 sec
  RS232_Put_Str((unsigned char*)"\x1b\x28");
 }

 void PDC_Software_Id(void)
 {
  RS232_Put_Str((unsigned char*)"\x1b\x3e");
  RS232_Put_Str((unsigned char*)"DOPT-07  -__-6-18Apr1994-_");
 }

 void PDC_Software_Cf(void)
 {
  RS232_Put_Str((unsigned char*)"\x1b\x3c");
  RS232_Put_Str((unsigned char*)"Nothing");
  message(F_DEBUG,"warning function %s is not supported!\n","PDC_Software_Cf");
 }

 void PDC_Insert_Off(void)  
 {
  gwin_Insert_Mode  = 0;
  WN_Puts_Abs_Fix(SCRROW,12,(unsigned char*)"   ",color_table[8]);
 }
 void PDC_Insert_On(void)
 {
  gwin_Insert_Mode  = 1;
  WN_Puts_Abs_Fix(SCRROW,12,(unsigned char*)"BCT",color_table[3]);
 }

 void PDC_Low_Case_Off(void){ gwin_f_crsr_end   = 0;}
 void PDC_Low_Case_On(void) { gwin_f_crsr_end   = 1;}
 void PDC_Protect_Off(void) { gwin_Protect_Mode = 0;}
 void PDC_Protect_On(void)  { gwin_Protect_Mode = 1;}
 void PDC_Keylock_Off(void) { gwin_Keylock_Mode = 0;}
 void PDC_Keylock_On(void)  { gwin_Keylock_Mode = 1;}
 void PDC_Format_Off(void)  { gwin_Format_Mode  = 0;}

 void PDC_Format_On(void)
 {
  int i;
  for(i=0;i<DOPTROW*DOPTCOL;i++)
   if(!gwin_PAM_Mem[i])
    break;

  if(i==DOPTROW*DOPTCOL)
  {
   RS232_Put_Char(0x7);
  }else
  {
   gwin_Format_Mode = 1;
   PDC_Cursor_Home();
  }
 }

 void PDC_Wrt_Enh_Mem(void)
 {
  int ptr; // bp-2
  if(!PDC_Xlat_224(&PDC_Row,&PDC_Col,&ptr))
  {
   if(ptr)
   {
    CPP_Waitlength = ptr;
    if(ptr>CPP_WAITBUFFER)
    {
     CPP_Cur_State = 8;
    }else
    {
     CPP_Waitbuf_Indx = 0;
     CPP_Cur_State = 5;
     CPP_Waitproc = PDC_Wrt_Enh_Body_2;
    }
   }
  }else CPP_Cur_State = 0;
 }

 void PDC_CWrt_Enh_Mem(void)
 {
  int ptr; //bp-2
  if(!PDC_Xlat_222(&PDC_Row,&PDC_Col,&ptr))
  {
   if(ptr)
   {
    CPP_Waitlength = ptr;
    if(ptr>CPP_WAITBUFFER)
    {
     CPP_Cur_State = 8;
    }else
    {
     CPP_Waitbuf_Indx = 0;
     CPP_Cur_State    = 5;
     CPP_Waitproc     = PDC_Wrt_Enh_Body_2;
    }
   }
  }else CPP_Cur_State = 0;
 }

 void PDC_Wrt_Enh_Body(void)
 {
  if(CPP_ch<'@'||CPP_ch>'_')
   CPP_ch = '@';
 
  CPP_ch = color_table[CPP_ch-'@'];
  WN_Fill_Abs_Enh(PDC_Row,PDC_Col,1,CPP_ch);
  if(++PDC_Col<DOPTCOL) 
   return;
  PDC_Col=0;
  PDC_Row++;
 }

 void PDC_Wrt_Enh_Body_2(void)
 {
  int i; // bp-2
  for(i=0;i<CPP_Waitlength;i++)
  {
   if(CPP_Waitbuf[i]<'@'||CPP_Waitbuf[i]>'_')   CPP_Waitbuf[i] = '@';
   CPP_Waitbuf[i] = color_table[CPP_Waitbuf[i]-'@'];
  }
  WN_Puts_Abs_Enh_Ext(PDC_Row,PDC_Col,CPP_Waitbuf,i);
 }

 void PDC_Rd_Char_Mem(void)
 {
  int a,//bp-2
      b,//bp-4
      c;//bp-6
  int offset; // bp-8
  if(!PDC_Xlat_224(&a,&b,&c))
  {
   if(a<DOPTROW&&b<DOPTCOL&&c)
   {
    offset = a*DOPTCOL+b;
    c+=offset;
    RS232_Put_Str((unsigned char*)"\x1b\x6c");
    int d = min(c,DOPTROW*DOPTCOL);
    RS232_Put_Str(&gwin_CHR_Mem[offset],d-offset);
   }
  }
 }

 void PDC_CRd_Char_Mem(void)
 {
  int a,//bp-2
      b,//bp-4
      c;//bp-6
  int offset;
  if(!PDC_Xlat_222(&a,&b,&c))
  {
   if(a<DOPTROW&&b<DOPTCOL&&c)
   {
    offset = a*DOPTCOL+b;
    c +=offset;
    RS232_Put_Str((unsigned char*)"\x1b\x6c");
    int d = min(c,DOPTROW*DOPTCOL);
    RS232_Put_Str(&gwin_CHR_Mem[offset],d-offset);
   }
  }
 }

 void PDC_Wrt_Chr_Mem(void)
 {
  int c;
  if(PDC_Xlat_224(&PDC_Row,&PDC_Col,&c))
  {
   CPP_Cur_State = 0;
   return;
  }
  if(c)
  {
   CPP_Waitlength = c;
   if(c>CPP_WAITBUFFER)
   {
    CPP_Cur_State = 9;
    return;
   }
   CPP_Waitbuf_Indx = 0;
   CPP_Cur_State = 5;
   CPP_Waitproc = PDC_Wrt_Chr_Body_2;
  }
 }

 void PDC_CWrt_Chr_Mem(void)
 {
  int c;
  if(PDC_Xlat_222(&PDC_Row,&PDC_Col,&c))
  {
   CPP_Cur_State = 0;
   return;
  }
  if(c)
  {
   CPP_Waitlength = c;
   if(c>CPP_WAITBUFFER)
   {
    CPP_Cur_State = 9;
    return;
   }
   CPP_Waitbuf_Indx = 0;
   CPP_Cur_State = 5;
   CPP_Waitproc = PDC_Wrt_Chr_Body_2;
  }
 }

 void PDC_Wrt_TChr_Mem(void)
 {
  if(CPP_Waitbuf[0]==0x66)
  {
   CPP_Waitlength   = 8;
   CPP_Cur_State    = 5;
   CPP_Waitbuf_Indx = 0;
   CPP_Waitproc     = PDC_Setup_Comb;
  }else
  {
   CPP_Waitlength   = 8;
   CPP_Cur_State    = 5;
   CPP_Waitbuf_Indx = 1;
   CPP_Waitproc     = PDC_Wrt_Chr_Mem;
  }
 }

 void PDC_Setup_Comb(void)
 {
  int c; // bp-2
  if(PDC_Xlat_224(&PDC_Row,&PDC_Col,&c))
  {
   CPP_Cur_State = 0;
  }else
  {
   if(c)
   {
    CPP_Waitlength = c+1;
    if(c>CPP_WAITBUFFER)
    {
     CPP_Cur_State = 0;
     return;
    }
    CPP_Waitbuf_Indx = 0x0;
    CPP_Cur_State    = 0x5;
    CPP_Waitproc     = PDC_Comb_Chr_Mem;
   }
  }
 }

 void PDC_Comb_Chr_Mem(void)
 {
  CPP_Waitbuf[0] = color_table[CPP_Waitbuf[0]];
  for(int i=1;i<CPP_Waitlength;i++)
  {
   if(CPP_Waitbuf[i]<' ') 
    CPP_Waitbuf[i]='@';
  }
  CPP_Waitlength--;
  WN_Fill_Abs_Enh(PDC_Row,PDC_Col,CPP_Waitlength,CPP_Waitbuf[0]);
  WN_Puts_Abs_Ext(PDC_Row,PDC_Col,(CPP_Waitbuf+1),CPP_Waitlength);
 }

 void PDC_Wrt_Chr_Body(void)
 {
  if(CPP_ch<' ')
  {
   CPP_ch = '@';
  }
  WN_Putch_Abs(PDC_Row,PDC_Col,CPP_ch);
  if(++PDC_Col>=DOPTCOL)
  {
   PDC_Col = 0;
   PDC_Row++;
  }
 }

 void PDC_Wrt_Chr_Body_2(void)
 {
  int i; // bp-2
  for(i=0;i<CPP_Waitlength;i++)
  {
   if(CPP_Waitbuf[i]<' ') CPP_Waitbuf[i] = '@';
  }
  WN_Puts_Abs_Ext(PDC_Row,PDC_Col,CPP_Waitbuf,i);
 }

 void PDC_Fill_Enh_Mem(void)
 {
  unsigned char *ptr = CPP_Waitbuf+8;
  int a,b,c;
  if(!PDC_Xlat_224(&a,&b,&c))
  {
   if(*ptr<'@'||*ptr>'_') *ptr='@';
   WN_Fill_Abs_Enh(a,b,c,color_table[*ptr-'@']);
  }
 }

 void PDC_CFill_Enh_Mem(void)
 {
  unsigned char *ptr = CPP_Waitbuf+6;
  int a,b,c;
  if(!PDC_Xlat_222(&a,&b,&c))
  {
   if(*ptr<'@'||*ptr>'_') *ptr='@';
   WN_Fill_Abs_Enh(a,b,c,color_table[*ptr-'@']);
  }
 }
 
 void PDC_Ctrl_Led(void)
 {
  int a,b;
// a  bp-2
// b  bp-4
  if(CPP_Waitbuf[0]<'0' || CPP_Waitbuf[0]>'5' || !Xlat_Nbr(CPP_Waitbuf+1,2,&a) || a>7)
   return;

  b = CPP_Waitbuf[0]-'0';
  if(!(CPP_Waitbuf[2]-'0'))
  {
   if(b)
   {
    WN_Puts_Abs_Fix(SCRROW,4,(unsigned char*)"BUSY",color_table[3]);
   }else
   {
    WN_Puts_Abs_Fix(SCRROW,4,(unsigned char*)"    ",color_table[8]);
   }
  }
  if(b==gwin_Led_Stats[a]) 
    return;
  gwin_Led_Stats[a] = b;
  if(gwin_Led_Stats[a]==4)
  {
   gwin_Led_Stats[a]=2;
  }else
  {
   if(gwin_Led_Stats[a]==5) 
    gwin_Led_Stats[a] = 2;
  }
  if(gwin_f_disp_lines == SCRROW)
  {
   LED_Flag = 1;
  }else
  {
   p_wn_ctrl_led(a);
  }
 }

 void PDC_Fill_Chr_Mem(void)
 {
  int a;
  if(!PDC_Xlat_224(&PDC_Row,&PDC_Col,&a))
  {
   if(CPP_Waitbuf[8]<' ') 
    CPP_Waitbuf[8] = '@';
   WN_Fill_Abs(PDC_Row,PDC_Col,a,CPP_Waitbuf[8]);
  }
 }

 void PDC_CFill_Chr_Mem(void)
 {
  int c;
  if(!PDC_Xlat_222(&PDC_Row,&PDC_Col,&c))
  {
   if(CPP_Waitbuf[6]<' ')
    CPP_Waitbuf[6] = '@';
   WN_Fill_Abs(PDC_Row,PDC_Col,c,CPP_Waitbuf[6]);
  }
 }

 void PDC_Long_Sync(void)
 {
  RS232_Put_Str((unsigned char*)"\x1b\x77");  // ESC w
 }
 int PDC_Test_Mode(void)
 {
  return gwin_Test_Mode;
 }
 void PDC_Keyswitches(void)
 {
  RS232_Put_Str((unsigned char*)"\x1b\x7b\x40"); // ESC {@
 }
 void PDC_Baudrate(void)
 {
  RS232_Set_Baudrate(CPP_Waitbuf);
 }
 void PDC_Swap_Window(void)
 {
  RS232_Put_Str((unsigned char*)"\x1b\x78\x46");
 }

 void PDC_Test_On(void)
 {
  v_old_lowcase_mode = gwin_Lowcase_Mode;
  gwin_Lowcase_Mode  = 1;
  gwin_Test_Mode     = 1;
 }

 void PDC_Test_Off(void)
 {
  gwin_Test_Mode    = 0;
  gwin_Lowcase_Mode = v_old_lowcase_mode;
 }

 void PDC_TDB_Features(void)
 {
  int a;
  a = CPP_Waitbuf[0];
  message(F_DEBUG,"Warning: function is building");

  if(a=='0')
  {
   RS232_Put_Str((unsigned char*)"\x1b\x01");
   RS232_Put_Char('0');
  }
  else
   if(a=='1'||a=='@')
   {
    CPP_Waitbuf_Indx = 0;
    CPP_Waitlength   = 1;
    CPP_Cur_State    = 5;
    if(CPP_Waitbuf[0]=='1')
     CPP_Waitproc = PDC_Baudrate;
    else
     CPP_Waitproc = PDC_Slct_Wndw;
   }
 }

 void PDC_Copy_Data(void)
 {
  int a,i;
  if(CPP_Waitbuf[1]=='1')
  {
   if(Xlat_Nbr(CPP_Waitbuf+6,4,&a))
   {
    for(i=0;i<a;i++) 
     RS232_Put_Char(' ');
   }
  }
 }

 void PDC_Compr_Char(void)
 {
  int a;
  CPP_Cur_State = 0x4;
  if(Chr2_to_Hex(CPP_Waitbuf,&a))
  {
   if(a<'0')
   {
    if(_table_char[a])
     WN_Putc(_table_char[a]);
   }
  } 
 }

 void PDC_Enh_Ctl_Prnt(void)
 {
  int a,b;
  if(!gwin_ESC_u_Flag)
  {
   if(Chr_to_Hex(CPP_Waitbuf[0],&a))
   {
    if(Chr_to_Hex(CPP_Waitbuf[1],&b))
     PRN_InBuf_Ch((a<<4)+b);
   }
  }
 }

 void PDC_Cpy_Dspl_Mem(void)
 {
  if(!gwin_ESC_u_Flag)
  {
   PDC_Row = 
   PDC_Col = 0;
   CPP_Cur_State = 0xB;
  }
 }
 void PDC_Set_Tab(void)
 {
  gwin_TAB_Mem[gwin_Cur_Col] = 1;
 }

 void PDC_Reset_Tab(void)
 {
  if(gwin_Cur_Col)  
   gwin_TAB_Mem[gwin_Cur_Col] = 0;
 }

 void PDC_Cursor_Up(void)
 {
  if(gwin_Cur_Row>0)
   WN_Set_Cursor(gwin_Cur_Row-1,gwin_Cur_Col);
 }

 void PDC_Cursor_Down(void)
 {
  if(gwin_Cur_Row!=(DOPTROW-1))
   WN_Set_Cursor(gwin_Cur_Row+1,gwin_Cur_Col);
 }
 
 void PDC_Cursor_Right(void)
 {
  if(gwin_Cur_Col!=(DOPTCOL-1))
   WN_Set_Cursor(gwin_Cur_Row,gwin_Cur_Col+1);
 }
 void PDC_Cursor_Left(void)
 {
  if(gwin_Cur_Col!=0)
   WN_Set_Cursor(gwin_Cur_Row,gwin_Cur_Col-1);
 }
 void PDC_Set_Cursor(void)
 {
  int a,b;
  if(!PDC_Xlat_22(&a,&b))
    WN_Set_Cursor(a,b);
 }

 void PDC_Csr_Antihome(void)
 {
  int a,      // bp-9
      b,      // bp-a
      c,      // bp-2
      offset, // bp-6
      len;    // bp-8
  unsigned char *ptr1;

  WN_Get_Slock(&a,&b);
  c = DOPTROW-1-b;
  if(!gwin_Format_Mode)
  {
   WN_Ctrl_SVW(a+DOPTROW-SCRROW,c,0);
  }else
  {
   offset = (c+1)*DOPTCOL;
   len = DOPTROW*DOPTCOL-1;
   for(;len>=offset;len--)
   {
    if(!gwin_PAM_Mem[len])
     break;
   }//for
   if(!gwin_PAM_Mem[len])
   {
     WN_Set_Cursor((len/DOPTCOL),len%DOPTCOL);
     return;
   }
   offset = gwin_SVW_Row*DOPTCOL+1;
   len = (gwin_SVW_Row+DOPTROW-b-a)*DOPTCOL;
   ptr1 = (len-1)+gwin_PAM_Mem; 
   for(;len>=offset;len--)
   {
    if(!*ptr1)
    {
     break;
    }
    ptr1--;
   }//for
   if(!*ptr1)
   {
     WN_Set_Cursor((len-1)/DOPTCOL,(len-1)%DOPTCOL); 
     return; 
   }
   if(a)
   {
    offset = 1;
    len = a*DOPTCOL;
    ptr1 = len-1+gwin_PAM_Mem;
    for(;len>=offset;len--)
    {
      if(!*ptr1)
      {
       break;
      }
      ptr1++;
    }
    if(!*ptr1)
    {
       WN_Set_Cursor((len-1)/DOPTCOL,(len-1)%DOPTCOL);
       return;
    }
   }
  }//else
 }

 void PDC_Cursor_Home(void)
 {
  int a = gwin_SLU;
  WN_Ctrl_SVW(a,a,0);
  if(gwin_Format_Mode==1)
    PDC_Next_Tab();
 }

 void PDC_Prev_Tab(void)
 {
  int a,   // bp-1
      b;   // bp-2
  int offset1;
  unsigned char *ptr;

  a = gwin_Cur_Row;
  b = gwin_Cur_Col;

  if(!gwin_Format_Mode)
  {
   if(!b)
   {
    if(a>0)
    {
     a--;
     b = DOPTCOL-1;
    }
   }else b--;
   for(;b>0;b--)
    if(gwin_TAB_Mem[b]==1)
     break;
   WN_Set_Cursor(a,b);
  }else
  {
   offset1 = a*DOPTCOL+b+1;
   ptr = offset1-1+gwin_PAM_Mem;

   for(ptr--;--offset1>0;ptr--)
   {
    if(!*ptr) break;
   }
   if(offset1<=0)
   {
    PDC_Cursor_Home();
    return;
   }
   unsigned char * offset2 = offset1-1+gwin_PAM_Mem;
   for(--offset2;--offset1>0;offset2--)
   {
    if(*offset2==1) break;
   }
   if(offset1<=0)
   {
    PDC_Cursor_Home();
    return;
   }
   WN_Set_Cursor(offset1/DOPTCOL,offset1%DOPTCOL);
  }
 }

 void PDC_Next_Tab(void)
 {
  int a,         // bp-7
      b,         // bp-8
      offset1,   // bp-4
      offset2;   // bp-6

  if(gwin_Format_Mode)
  {
   a  = gwin_Cur_Row; // bp-7
   b  = gwin_Cur_Col; // bp-8
   offset1 = a*DOPTCOL+b;
   offset2 = offset1;
   for(;offset1<DOPTROW*DOPTCOL;offset1++)
    if(gwin_PAM_Mem[offset1])
     break;
   if(offset1!=DOPTROW*DOPTCOL)
   {
    for(;offset1<DOPTROW*DOPTCOL;offset1++)
      if(!gwin_PAM_Mem[offset1])
        break;

    if(offset1<DOPTROW*DOPTCOL)
    {
     WN_Set_Cursor(offset1/DOPTCOL,offset1%DOPTCOL);
     return;
    }
    offset1=0;
    for(;offset1<offset2;offset1++)
     if(!gwin_PAM_Mem[offset1])
      break;
    if(offset1<=offset2)
    {
     WN_Set_Cursor(offset1/DOPTCOL,offset1%DOPTCOL);
     return;
    }
    PDC_Cursor_Home();
   }else
   {
    for(offset1=0;offset1<offset2;offset1++)
     if(gwin_PAM_Mem[offset1])
       break;
    if(offset1<=offset2)
    {
     for(;offset1<offset2;offset1++)
      if(!gwin_PAM_Mem[offset1])
       break;
     if(offset1<=offset2)
     {
      WN_Set_Cursor(offset1/DOPTCOL,offset1%DOPTCOL);
      return;
     }
     WN_Set_Cursor(a,b);
    }else
     WN_Set_Cursor(0,0);
   }
  }else
  {
   int h1;
   h1 = gwin_Cur_Col+1;
   for(;h1<DOPTCOL;h1++)
    if(gwin_TAB_Mem[h1])
     break;
   if(h1==DOPTCOL)
   {
    PDC_Carr_Return();
    PDC_Linefeed();
    return;
   }
   WN_Set_Cursor(gwin_Cur_Row,h1);
  }
 }

 void PDC_Carr_Return(void)
 {
  WN_Set_Cursor(gwin_Cur_Row,0);
 }


 void PDC_New_Line(void)
 {
  PDC_Carr_Return();
  PDC_Linefeed();
 }

 void PDC_Linefeed(void)
 {
  int a, //bp-2
      b, //bp-1
      d; //bp-6
  if(gwin_Format_Mode)
  {
   if(gwin_Cur_Row<DOPTROW-1)
    WN_Set_Cursor(gwin_Cur_Row,gwin_Cur_Col);
  }else
  {
   a  =  DOPTROW-1-gwin_MLL;
   if(gwin_Cur_Row!=a)
   {
    if(gwin_Cur_Row!=DOPTROW-1)
     b = gwin_Cur_Row+1;
    else
     b = DOPTROW-1;
   }else
   {
    d = gwin_MLU*DOPTCOL;
    int o1 = a*DOPTCOL,
        o2 = gwin_MLL*DOPTCOL;
    memmove(gwin_CHR_Mem+d,&gwin_CHR_Mem[DOPTCOL]+d,(DOPTROW*DOPTCOL)-d-o2);
//TRY
    _mmemset(gwin_CHR_Mem+o1,' ',DOPTCOL);
    _mmemset(gwin_PAM_Mem+o1,0x1,DOPTCOL);
    _mmemset(gwin_DAM_Mem+o1,color_table[8],DOPTCOL);

    WN_Redraw_Client_Area();
    b =a;
   }
   WN_Set_Cursor( b,gwin_Cur_Col);
  }
 }

 void PDC_Sense_Cursor(void)
 {
  char str[8];
  str[0] = 0x1b;
  str[1] = 0x46;
  Word_to_Ascii(gwin_Cur_Row,&str[2],2);
  Word_to_Ascii(gwin_Cur_Col,&str[4],2);
  str[6] = 0;
  RS232_Put_Str((unsigned char*)str);
 }

 void PDC_Ctrl_Cursor(void)
 {
  int a = CPP_Waitbuf[0];
  gwin_Cur_Control = a-'A';
  WN_Set_Cursor_Shape(a,a);
 }

 void PDC_Def_Chr_Gen(void)
 {
  PDC_Row = 0;
 }

 void PDC_Char_Gen(void)
 {
  if(CPP_ch=='F')
  {
   PDC_Row++;
   if(PDC_Row==2)
   {
    CPP_Cur_State = 0;
    PDC_Clr_All();
    PDC_Format_Off();
   }
  }else PDC_Row = 0;
 }

 void PDC_Def_VT220_Kbd(void)       {}
 void PDC_Def_Pri_Part(void)        {}
 void PDC_Def_Pri_All(void)         {}
 void PDC_Def_Shf_Part(void)        {}
 void PDC_Def_Shf_All(void)         {}
 void PDC_Def_Ctl_Part(void)        {}
 void PDC_Def_Ctl_All(void)         {}
 void PDC_Def_Typ_Part(void)        {}
 void PDC_Def_Typ_All(void)         {}
 void PDC_Def_Pre_Part(void)        {}
 void PDC_Def_Pre_All(void)         {}
 void PDC_Dflt_Def_St(void)         {}
 void PDC_Dflt_Compr_St(void)       {}
 void PDC_Dflt_Esc_St(void)         {}
 void PDC_Bootload(void)            {}
 void PDC_Dummy(void)               {}
 void PDC_Slct_Wndw(void)           {}
 void PDC_Page_Switch(void)         {}
 void PDC_E_D_Pref_Cd(void)         {}

 void PDC_Set_Mem_Lock(void)
 {
  int a,
      b;
  if(!PDC_Xlat_22(&a,&b))
  {
   if(a>DOPTROW-1|| b>DOPTROW-1 || a+b>DOPTROW+1 || gwin_SLU>a || gwin_SLL>b) 
    return;
   gwin_MLU = a;
   gwin_MLL = b;
  }
 }

 void PDC_Set_Scr_Lock(void)
 {
  int a, //bp-2
      b, //bp-4
      a1,//bp-9
      b1,//bp-A
      y1,//bp-8
      y2;//bp-6

  if(!PDC_Xlat_22(&a,&b))
  {
   if(a+b<=SCRROW-1)
   { 
    if(gwin_MLU<a) gwin_MLU = a;
    if(gwin_MLL<b) gwin_MLL = b;
    WN_Calc_Dspl(gwin_Cur_Row,gwin_Cur_Col,&a1,&b1);
    y1 = gwin_SLU;
    gwin_SLU = a;
    gwin_SLL = b; 
    y2 = gwin_SVW_Row+a-y1;

    if(a1<a)
    {
     gwin_Cur_Row = a1;
    }else
    {
     if(a1>=25-b)  gwin_Cur_Row = a1+0x32;
     else          gwin_Cur_Row = y2+a1-a;
    }
  
//    WN_Redraw_Partial_Line_Fix(0,int p1,int p2,int line)

//    WN_Redraw_All();
///*
    int i,j;
    for(i=1,j=0;i<=a;i++)
    {
     WN_Redraw_Line(j++);
    }
    for(i=1,j=DOPTROW-1;i<=b;i++)
    {
     WN_Redraw_Line(j--);
    }
//*/
    WN_Ctrl_SVW(y2,gwin_Cur_Row,gwin_Cur_Col);
   }
  }
 }

 void PDC_Ctrl_SVW(void)
 {
  int a,
      b,
      c;
  if(!Xlat_Nbr(CPP_Waitbuf,2,&a))   return;
  if(!Xlat_Nbr(CPP_Waitbuf+2,2,&b)) return;
  if(!Xlat_Nbr(CPP_Waitbuf+4,2,&c)) return;
  WN_Ctrl_SVW(a,b,c);
 }

 void PDC_Next_Page(void)
 {
  int a,
      b,
      c,
      d;
  WN_Get_Slock(&a,&b);
  c = SCRROW+gwin_SVW_Row-a-b;
  if(c>a+DOPTROW-SCRROW) c =a+DOPTROW-SCRROW;
  gwin_SVW_Row = c;
  WN_Redraw_Client_Area();
  d = gwin_Cur_Row;
  if(d<a)
   return;
  if(d>DOPTROW-1-b)
   return;
  if(d<c)
  {
   WN_Set_Cursor(c,gwin_Cur_Col);
   return;
  }
  WN_Set_Cursor(d,gwin_Cur_Col);
 }

 void PDC_Prev_Page(void)
 {
  int a,
      b,
      c;
  WN_Get_Slock(&a,&b);
  c = gwin_SVW_Row;
  if(c>=(SCRROW-b))
  {
   c = c+a+b-SCRROW;
  }else c = a;
  gwin_SVW_Row = c;
  WN_Redraw_Client_Area();
  if(gwin_Cur_Row<a || gwin_Cur_Row>(DOPTROW-1-b))
   return;
  if((c+SCRROW-1-a-b)<gwin_Cur_Row)
   WN_Set_Cursor(c+SCRROW-1-a-b,gwin_Cur_Col);
  else
   WN_Set_Cursor(gwin_Cur_Row,gwin_Cur_Col);
 }

 void PDC_Roll_Up(void)
 {
  int a;//, //bp-1
  if(gwin_SVW_Row<gwin_SLU+DOPTROW-SCRROW)
  {
   gwin_SVW_Row++;
   WN_Redraw_Client_Area();
   a = gwin_Cur_Row;
   if(a>=gwin_SLU)
   {
    if(a<=(DOPTROW-1-gwin_SLL))
    {
     if(a<gwin_SVW_Row) 
      WN_Set_Cursor(a+1,gwin_Cur_Col);
     else 
      WN_Set_Cursor(a,gwin_Cur_Col);
    }
   }
  }
 }

 void PDC_Roll_Down(void)
 {
  int a,  //bp-4
      b,  //bp-2
      c,  //bp-3
      m;  //bp-1

  if(gwin_SVW_Row>gwin_SLU)
  {
   gwin_SVW_Row--;
   WN_Redraw_Client_Area();
   a = gwin_Cur_Row;
   if(a<gwin_SLU || a>DOPTROW-1-gwin_SLL) return;
   WN_Get_Slock(&b,&c);
   m = gwin_SVW_Row+SCRROW-1-b-c;
   a = gwin_Cur_Row;
   if(a>m)
    WN_Set_Cursor(a-1,gwin_Cur_Col);
   else
    WN_Set_Cursor(a,gwin_Cur_Col);
  }
 }

 void PDC_Dir_Line_Ins(void)
 {
  int a,
      b,
      c,
      d,
      m,
      z;
  if(!PDC_Xlat_22(&a,&b))
  {
   c = gwin_MLL; 
   d = gwin_MLU;
   if(a<DOPTROW)
   {
    m = DOPTROW-c;
    if(m-d>=b)
    {
     z = m-b;     
     if(z>=a)
     {
      int o1 = a*DOPTCOL,
          o2 = b*DOPTCOL,
          o3 = z*DOPTCOL;
      memmove(gwin_CHR_Mem+o1+o2,gwin_CHR_Mem+o1,o3-o1);
      memmove(gwin_PAM_Mem+o1+o2,gwin_PAM_Mem+o1,o3-o1);
      memmove(gwin_DAM_Mem+o1+o2,gwin_DAM_Mem+o1,o3-o1);
//TRY
      _mmemset (gwin_CHR_Mem+o1,' ',o2);                                                    
      _mmemset (gwin_PAM_Mem+o1,0x1,o2);
      _mmemset (gwin_DAM_Mem+o1,color_table[8],o2);
      WN_Redraw_All();
     }else
     {
       if(m<=a)
       {
        int o1 = a*DOPTCOL,
           o2 = b*DOPTCOL,
           o3 = z*DOPTCOL,
           o4 = c*DOPTCOL;
        memmove(gwin_CHR_Mem+o3,gwin_CHR_Mem+DOPTROW*DOPTCOL-o4,o1+o4-DOPTROW*DOPTCOL);
        memmove(gwin_PAM_Mem+o3,gwin_PAM_Mem+DOPTROW*DOPTCOL-o4,o1+o4-DOPTROW*DOPTCOL);
        memmove(gwin_DAM_Mem+o3,gwin_DAM_Mem+DOPTROW*DOPTCOL-o4,o1+o4-DOPTROW*DOPTCOL);
//TRY
        _mmemset (gwin_CHR_Mem+o1-o2,' ',o2);
        _mmemset (gwin_PAM_Mem+o1-o2,0x1,o2);
        _mmemset (gwin_DAM_Mem+o1-o2,color_table[8],o2);
        WN_Redraw_All();
       }
     }
    }
   }
  }
 }

 void PDC_Dir_Line_Del(void)
 {
  int a,
      b,
      c,
      m,
      i;
  if(!PDC_Xlat_22(&a,&b))
  {
   if(a>=DOPTROW || b>DOPTROW-a) return;

   c = gwin_MLL;
   m = DOPTROW-c;
   if(m-b>=a)
   {
    int o1 = a*DOPTCOL,
        o2 = b*DOPTCOL,
        o4 = c*DOPTCOL;
    memmove(o1+gwin_CHR_Mem,o1+o2+gwin_CHR_Mem,DOPTROW*DOPTCOL-o1-o2-o4);
    memmove(o1+gwin_PAM_Mem,o1+o2+gwin_PAM_Mem,DOPTROW*DOPTCOL-o1-o2-o4);
    memmove(o1+gwin_DAM_Mem,o1+o2+gwin_DAM_Mem,DOPTROW*DOPTCOL-o1-o2-o4);
//TRY
    _mmemset(DOPTROW*DOPTCOL-o2-o4+gwin_CHR_Mem,' ',o2);
    _mmemset(DOPTROW*DOPTCOL-o2-o4+gwin_PAM_Mem,0x1,o2);
    _mmemset(DOPTROW*DOPTCOL-o2-o4+gwin_DAM_Mem,color_table[8],o2);
    WN_Redraw_All();
   }else
   {
    if(m>a) return;
    i = 0;
    while(i<b)
    {
     int o1 = a*DOPTCOL,
         o2 = b*DOPTCOL,
         o4 = c*DOPTCOL;
     memmove((DOPTROW+1)*DOPTCOL-o4+gwin_CHR_Mem,DOPTROW*DOPTCOL-o4+gwin_CHR_Mem,o1+o4-DOPTROW*DOPTCOL);
     memmove((DOPTROW+1)*DOPTCOL-o4+gwin_PAM_Mem,DOPTROW*DOPTCOL-o4+gwin_PAM_Mem,o1+o4-DOPTROW*DOPTCOL);
     memmove((DOPTROW+1)*DOPTCOL-o4+gwin_DAM_Mem,DOPTROW*DOPTCOL-o4+gwin_DAM_Mem,o1+o4-DOPTROW*DOPTCOL);
//TRY
     _mmemset(DOPTROW*DOPTCOL-o4+gwin_CHR_Mem,' ',DOPTCOL);
     _mmemset(DOPTROW*DOPTCOL-o4+gwin_PAM_Mem,0x1,DOPTCOL);
     _mmemset(DOPTROW*DOPTCOL-o4+gwin_DAM_Mem,color_table[8],DOPTCOL);
     i++;
    }
//    WN_Redraw_All();
///*
    for(i=DOPTROW-c;i<DOPTROW;i++)  
     WN_Redraw_Line(i);
//*/
   }
  }
 }

 void PDC_Ctrl_Print(void)
 {
  if(!gwin_ESC_u_Flag)
   PRN_InBuf_Ch(CPP_Waitbuf[0]-'@');
 }

 void PDC_Sense_Print(void)
 {
  if(!PRN_Cnt)
  {
   if(PRN_Check_Print_Rdy()==0x2) 
    RS232_Put_Str((unsigned char*)"\x1b\x71\x21");
   else 
    RS232_Put_Str((unsigned char*)"\x1b\x73\x20");
  }else
  {
   if(PRN_Check_Print_Rdy()==0x2) 
    RS232_Put_Str((unsigned char*)"\x1b\x73\x31");
   else 
    RS232_Put_Str((unsigned char*)"\x1b\x73\x30");
  }
 }

 void PDC_Ctl_Prnt_Int(void)
 {
  PRN_Init();
  if(!PRN_Check_Print_Rdy()) 
   PRN_Flush();
 }

 void PDC_FreePrntHead(void)
 {
  int a; // bp-2
  if(!Xlat_Nbr(CPP_Waitbuf,4,&a))
  {
   CPP_Cur_State = 0;
   return;
  }
  if(a!=0)
  {
   CPP_Cur_State = 0xa;
   CPP_Waitlength = a;
  }
 }
 
 void PDC_FreePrntBody(void)
 {
  if(!gwin_ESC_u_Flag) PRN_InBuf_Ch(CPP_ch);
 }
 void PDC_PrnBufEmpty(void)
 {
  gwin_ESC_u_Flag = 1;
 }

 void PDC_Video_On(void)
 {
  WN_Redraw_All();
  WN_Set_Cursor(gwin_Cur_Row,gwin_Cur_Col);
  v_video_on = 1;
 }

 void PDC_Video_Off(void)
 {
  v_video_on=0;
 }

 void PDC_Clr_All(void)
 {
  if(!gwin_Format_Mode)
  {
//TRY
   _mmemset(gwin_CHR_Mem,' ',DOPTROW*DOPTCOL);
   _mmemset(gwin_PAM_Mem,1,DOPTROW*DOPTCOL);
   _mmemset(gwin_DAM_Mem,color_table[8],DOPTROW*DOPTCOL);
  }else
  {
   for( int i=0;i<DOPTROW*DOPTCOL;i++)
   {
    if(!gwin_PAM_Mem[i])
     gwin_CHR_Mem[i]= ' ';
   }
  }
  PDC_Cursor_Home();
  WN_Redraw_All();
 }

 void PDC_Clr_Par(void)
 {
  int offset; //bp-2
  offset = gwin_Cur_Row*DOPTCOL+gwin_Cur_Col;
  if(!gwin_Format_Mode)
  {
//TRY
   _mmemset(gwin_CHR_Mem+offset,' ',DOPTROW*DOPTCOL-offset);
   _mmemset(gwin_PAM_Mem+offset,gwin_Protect_Mode,DOPTROW*DOPTCOL-offset);
  }else
  {
    for(;offset<DOPTROW*DOPTCOL;offset++)
    {
     if(!gwin_PAM_Mem[offset])
      gwin_CHR_Mem[offset]=' ';
    }
  }
  WN_Redraw_All();
 }

 void PDC_Clr_Line(void)
 {
  int offset;
  int offx = DOPTCOL-gwin_Cur_Col;

  offset = gwin_Cur_Row*DOPTCOL+gwin_Cur_Col;

  if(!gwin_Format_Mode)
  {
//TRY
   _mmemset(gwin_CHR_Mem+offset,' ',offx);
   _mmemset(gwin_PAM_Mem+offset,gwin_Protect_Mode,offx);
  }else
  {
   int end = (gwin_Cur_Row+1)*DOPTCOL;
   do
   {
    gwin_CHR_Mem[offset]=' ';
    if(offset==end) break;
    offset++;
   }while(!gwin_PAM_Mem[offset]);
  }
  p_wn_redraw_characters_fix(gwin_Cur_Row,1);
 }

 void PDC_Insert_Line(void)
 {
  int a,         // bp-1
      b,         // bp-4
      c,         // bp-6
      d;         // bp-8

  if(!gwin_Format_Mode)
  {
   a = gwin_Cur_Row;
   if(a<gwin_MLU)
    return;
   b = a;             
   c = DOPTROW-1-gwin_MLL;
   if(c>=b)
   {
    int o2 = b*DOPTCOL,
        o4 = c*DOPTCOL,
        o5 = gwin_Cur_Row*DOPTCOL;
    d = o2;
    memmove(gwin_CHR_Mem+DOPTCOL+d,gwin_CHR_Mem+d,o4-o2);
    memmove(gwin_PAM_Mem+DOPTCOL+d,gwin_PAM_Mem+d,o4-o2);
    memmove(gwin_DAM_Mem+DOPTCOL+d,gwin_DAM_Mem+d,o4-o2);
//TRY
    _mmemset(o5+gwin_CHR_Mem,' ',DOPTCOL);
    _mmemset(o5+gwin_PAM_Mem,0x1,DOPTCOL);
    _mmemset(o5+gwin_DAM_Mem,color_table[8],DOPTCOL);
    WN_Redraw_Client_Area();
    WN_Set_Cursor(gwin_Cur_Row,0);
   }
  }
 }

 void PDC_Del_Line(void)
 {
  int a, // bp-3
      b, // bp-6
      c, // bp-8
      d, // bp-a
      e; // bp-2
 
  if(!gwin_Format_Mode)
  {
   a = gwin_Cur_Row;
   if(a<gwin_MLU)
    return;
   b = a;
   c = DOPTROW-1-gwin_MLL;
   if(a<=c)
   {
    int o2 = b*DOPTCOL,
        o4 = c*DOPTCOL,
        o5 = gwin_Cur_Row*DOPTCOL;
    d = o2;
    memmove(gwin_CHR_Mem+d,gwin_CHR_Mem+DOPTCOL+d,o4-o2);
    memmove(gwin_PAM_Mem+d,gwin_PAM_Mem+DOPTCOL+d,o4-o2);
    memmove(gwin_DAM_Mem+d,gwin_DAM_Mem+DOPTCOL+d,o4-o2);
    e = (DOPTCOL*DOPTROW-DOPTCOL)-DOPTCOL*gwin_MLL;
//TRY
    _mmemset(gwin_CHR_Mem+e,' ',DOPTCOL);
    _mmemset(gwin_PAM_Mem+e,0x1,DOPTCOL);
    _mmemset(gwin_DAM_Mem+e,color_table[8],DOPTCOL);
    WN_Redraw_Client_Area();
    WN_Set_Cursor(gwin_Cur_Row,0);
   }
  }
 }

 void PDC_Del_Ch(void)
 {
  int a,         //bp-8
      offset,    //bp-2
      b,         //bp-6
      c;         //bp-4

   a = gwin_Cur_Col;
   offset = DOPTCOL*gwin_Cur_Row+a;
  if(!gwin_Format_Mode)
  {
   memmove(gwin_CHR_Mem+offset,gwin_CHR_Mem+1+offset,DOPTCOL-1-a);
   memmove(gwin_PAM_Mem+offset,gwin_PAM_Mem+1+offset,DOPTCOL-1-a);
   gwin_CHR_Mem[(gwin_Cur_Row+1)*DOPTCOL-1]=' ';
   gwin_PAM_Mem[(gwin_Cur_Row+1)*DOPTCOL-1]=0x1;
   WN_Redraw_Line(gwin_Cur_Row);
  }else
  {
   if(!gwin_PAM_Mem[offset])
   {
    for(b=offset;b<DOPTROW*DOPTCOL;b++)
    {
     if(gwin_PAM_Mem[b]==1)  break;
    }
    c = b-1; 
    memmove(gwin_CHR_Mem+offset,gwin_CHR_Mem+offset+1,c-offset);
    gwin_CHR_Mem[c]=' ';
    offset = offset/DOPTCOL;
    c = c/DOPTCOL;
    for(b=offset;b<=c;b++)
     WN_Redraw_Line(b);
   }
  }
 }

/*
 MyFunction
*/
 void PDC_Set_Color_Table(unsigned char*buffer,int size)
 {
  size = min(size,sizeof(color_table)/sizeof(char));
  memcpy(color_table,buffer,size);
 }

 int PDC_Get_Color_Table(unsigned char*buffer,int size)
 {
  if(!buffer) size = sizeof(color_table)/sizeof(char);
  else{
   if(!(size<sizeof(color_table)/sizeof(char))){
    size = min(size,sizeof(color_table)/sizeof(char));
    memcpy(buffer,color_table,size);
   }
  }
  return size;
 }


