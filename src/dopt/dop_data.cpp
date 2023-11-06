
  #define _DOPT_DEFINE_DATA
  #include "dopt.h"

  unsigned char *gwin_CHR_Mem;
  unsigned char *gwin_PAM_Mem;
  unsigned char *gwin_DAM_Mem;
  unsigned char *gwin_TAB_Mem;
  int gwin_Cur_Row;
  int gwin_Cur_Col;
  int gwin_Cur_Start;
  int gwin_Cur_End;
  int gwin_Cur_Control;
  int gwin_Format_Mode;
  int gwin_Protect_Mode;
  int gwin_Test_Mode;
  int gwin_Insert_Mode;
  int gwin_Lowcase_Mode;
  int gwin_Keylock_Mode;
  int gwin_Shflock_Mode;
  int gwin_Led_Stats[9];
  int gwin_Led[9];
  int gwin_MLU;
  int gwin_MLL;
  int gwin_SLU;
  int gwin_SLL;
  int gwin_SVW_Row;
  int gwin_f_crsr_start;
  int gwin_f_crsr_end;
  int gwin_ESC_u_Flag;
  int gwin_f_disp_lines;


 void WN_Init(void)
 {
  unsigned char *buffer;
  char tmp_str[1024];
  char *str = get_option("CONSOLE_color")->s,
       *ptr,
       *start,
       *end;
  int i,size = PDC_Get_Color_Table(NULL,0)+1;

  buffer = (unsigned char*)calloc(1,size);
  size = PDC_Get_Color_Table(buffer,size);

  gwin_Cur_Row       = 
  gwin_Cur_Col       = 
  gwin_Cur_Start     = 
  gwin_Cur_End       =
  gwin_Cur_Control   =
  gwin_Format_Mode   =
  gwin_Protect_Mode  =
  gwin_Test_Mode     =
  gwin_Insert_Mode   =
  gwin_Lowcase_Mode  =
  gwin_Keylock_Mode  =
  gwin_Shflock_Mode  =
  gwin_MLU           =
  gwin_MLL           =
  gwin_SLU           =
  gwin_SLL           =
  gwin_SVW_Row       =
  gwin_f_crsr_start  =
  gwin_f_crsr_end    =
  gwin_ESC_u_Flag    =
  gwin_f_disp_lines  = 0;

  memset(gwin_Led_Stats,0,sizeof(gwin_Led_Stats));
  memset(gwin_Led,0,sizeof(gwin_Led));

  if(str)
  {
   int min_size = min(strlen(str),sizeof(tmp_str));
   strncpy(tmp_str,str,min_size); tmp_str[min_size]=0;
   ptr = tmp_str;
   i=0;
   while(1)
   {
    start = ptr;
    while(*ptr&&*ptr!=',') ptr++;
    end   = ptr;
    *ptr = 0;
    if(i<size)
    {
     int attr=0;
     if(sscanf(start,"%X",&attr)==1)
     {
      buffer[i] = attr;
     }
    }
    i++;
    if(++ptr>=(tmp_str+min_size)) break;
   }
   PDC_Set_Color_Table(buffer,size);
   message(F_DEBUG,"Change color table(DOPT)... Ok\n");
  }
  free(buffer);
  WN_SW_Init();
 }

 void WN_SW_Init(void)
 {
   gwin_CHR_Mem = (unsigned char *)calloc(sizeof(char),DOPTROW*DOPTCOL);
   gwin_PAM_Mem = (unsigned char *)calloc(sizeof(char),DOPTROW*DOPTCOL);
   gwin_DAM_Mem = (unsigned char *)calloc(sizeof(char),DOPTROW*DOPTCOL);
   gwin_TAB_Mem = (unsigned char *)calloc(sizeof(char),DOPTCOL);

   memset(gwin_CHR_Mem,' ',DOPTROW*DOPTCOL);
   memset(gwin_PAM_Mem,1,DOPTROW*DOPTCOL);
   memset(gwin_DAM_Mem,color_table[8],DOPTROW*DOPTCOL);

   gwin_TAB_Mem[0]   = 1;
   gwin_Cur_Start    = 0xc;
   gwin_Cur_End      = 0xc;
   gwin_Cur_Control  = 1;
   gwin_Protect_Mode = 1;
   WN_Redraw_All();
   WN_Set_Cursor(0,0);
   WN_Set_Cursor_Shape('A','A');
 }

 void WN_Close(void)
 {
  if(gwin_CHR_Mem) free(gwin_CHR_Mem);
  if(gwin_PAM_Mem) free(gwin_PAM_Mem);
  if(gwin_DAM_Mem) free(gwin_DAM_Mem);
  if(gwin_TAB_Mem) free(gwin_TAB_Mem);
 }

 void WN_Putch_Abs(int y,int x,int c)
 {
  int offset = y*DOPTCOL+x;
  gwin_CHR_Mem[offset] = c;
  gwin_PAM_Mem[offset] = (char)gwin_Protect_Mode;
  WN_Redraw_Ch(y,x);
 }

 void WN_Puts_Abs_Fix(int y,int x,unsigned char*str,char attr)
 {
  ScreenDevice.OutputString(TRUE,x,y,attr,(char*)str);
 }

 void WN_Puts_Abs(int y,int x,unsigned char *str)
 {
  int offset,len;
  if(y<DOPTROW&&x<DOPTCOL)
  {
   offset = y*DOPTCOL+x;
   len = strlen((char*)str);
   if(x+len>DOPTCOL)
   {
    while(*str)
    {
     gwin_CHR_Mem[offset]   = *str++;
     gwin_PAM_Mem[offset++] = (char)gwin_Protect_Mode;
     if(++x>DOPTCOL)
     {
       p_wn_redraw_characters_fix(y,1);
       x=0;
       if(++y>DOPTROW) break;
     }//if
    }//while
   }else
   {
//     len = min(len,DOPTROW*DOPTCOL-offset);
     memcpy(&gwin_CHR_Mem[offset],str,len);
     memset(&gwin_PAM_Mem[offset],gwin_Protect_Mode,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 }

 void WN_Puts_Abs_Ext(int y,int x,unsigned char*s,int len)
 {
  int offset;
  offset = y*DOPTCOL+x;
  if(y<DOPTROW&&x<DOPTCOL)
  {
   if(x+len>DOPTCOL)
   {
    for(int i=0;i<len;i++)
    {
     gwin_CHR_Mem[offset]   = *s++;
     gwin_PAM_Mem[offset++] = (char)gwin_Protect_Mode;
     if(++x>DOPTCOL)
     {
      p_wn_redraw_characters_fix(y,1);
      x = 0;
      if(++y>DOPTROW) break;
     }
    }//for
   }else
   {
//     len = min(len,DOPTROW*DOPTCOL-offset);
     memcpy(&gwin_CHR_Mem[offset],s,len);
     memset(&gwin_PAM_Mem[offset],gwin_Protect_Mode,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 }
 
 void WN_Fill_Abs(int y,int x,int len,int s)
 {
  int offset,i;
  offset = y*DOPTCOL+x;
  if(y<DOPTROW&&x<DOPTCOL)
  {
   if(x+len>DOPTCOL)
   {
    for(i=0;i<len;i++)
    {
     gwin_CHR_Mem[offset] = s;
     gwin_PAM_Mem[offset++] = (char)gwin_Protect_Mode;
     if(++x>DOPTCOL)
     {
       p_wn_redraw_characters_fix(y,1);
       x = 0;
       if(++y>DOPTROW) return;
     }
    }//for
   }else
   {
//    len = min(len,DOPTROW*DOPTCOL-offset);
    memset(&gwin_CHR_Mem[offset],s,len);
    memset(&gwin_PAM_Mem[offset],gwin_Protect_Mode,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 }

 void WN_Fill_Abs_Enh(int y,int x,int len,int attr)
 {
  int offset,i;
  offset = y*DOPTCOL+x;
  if(y<DOPTROW&&x<DOPTCOL)
  {
   if(x+len>DOPTCOL)
   {
    for(i=0;i<len;i++)
    {
     gwin_DAM_Mem[offset++] = attr;
     if(++x>DOPTCOL)
     {
      p_wn_redraw_characters_fix(y,1);
      x = 0;
      if(++y>DOPTROW) return;
     }
    }//for
   }else
   {
//    len = min(len,DOPTROW*DOPTCOL-offset);
    memset(&gwin_DAM_Mem[offset],attr,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 }

 void WN_Puts_Abs_Enh(int y,int x,char *str)
 {
  int offset,len;
  offset = y*DOPTCOL+x;
  len = strlen(str);
  if(y<DOPTROW&&x<DOPTCOL)
  {
   if(x+len>DOPTCOL)
   {
    while(*str)
    {
     gwin_DAM_Mem[offset++] = *str++;
     if(++x>DOPTCOL)
     {
      p_wn_redraw_characters_fix(y,1);
      x = 0;
      if(++y>DOPTROW) return;
     }
    }
   }else
   {
//    len = min(len,DOPTROW*DOPTCOL-offset);
    memcpy(&gwin_DAM_Mem[offset],str,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 }

 void WN_Puts_Abs_Enh_Ext(int y,int x,unsigned char*str,int len)
 {
  int offset,i;
  offset = y*DOPTCOL+x;
  if(y<DOPTROW&&x<DOPTCOL)
  {
   if(x+len>DOPTCOL)
   {
    for(i=0;i<len;i++)
    {
     gwin_DAM_Mem[offset++] = *str++;
     if(++x>DOPTCOL)
     {
      p_wn_redraw_characters_fix(y,1);
      x=0;
      if(++y>DOPTROW) return;
     }
    }
   }else
   {
//    len = min(len,DOPTROW*DOPTCOL-offset);
    memcpy(&gwin_DAM_Mem[offset],str,len);
   }
   p_wn_redraw_characters_fix(y,1);
  }
 } 

 void WN_Set_Cursor_Shape(int y_s,int y_e)
 {
  int visual = CURSOR_ENABLE,size=CURSOR_NORMAL;
  switch(y_s)
  {
   case 'B': 
   case '@': visual = CURSOR_ENABLE;  size = CURSOR_NORMAL;   
   break;
   case 'A': visual = CURSOR_DISABLE; size = CURSOR_NORMAL;   
   break;
   case 'C': visual = CURSOR_ENABLE;  size = CURSOR_BLOCK;    
   break;
   default:
   case 'D': visual = CURSOR_DISABLE; size = CURSOR_HALF;     
   break;
  }
  ScreenDevice.CursorVisible(FALSE,visual);
  ScreenDevice.CursorSize(FALSE,size);
 }

 void WN_Get_Slock(int *slu,int*sll)
 {
  *slu = gwin_SLU;
  *sll = gwin_SLL;
 }

 void WN_Ctrl_SVW(int y1,int y2,int xx)
 {
   int a,b;
   WN_Get_Slock(&a,&b);
   if(y1<a||y1>a+DOPTROW-SCRROW) return;   // +50
   if(y2>=a)
   {
    if(y2>=y1)
    if(y1+SCRROW-1-a-b>=y2)
     goto L459;
    if(y2<=DOPTROW-1-b) return;
    if(y2>=DOPTROW)     return;
   }
L459:
   gwin_SVW_Row = y1;
   WN_Redraw_Client_Area();
   WN_Set_Cursor(y2,xx);
 }

// needfix
 void WN_Upd_Wndw(int y,int x,int c,int d)
 {
 }

 int WN_Calc_Dspl(int p1,int p2,int *p3,int *p4)
 {
  int a,b,c,z;

  a = gwin_SVW_Row; 
  b = gwin_SLU;         
  c = gwin_SLL;          
  if(p1<gwin_SLU)
  {
   *p3 = p1;
   *p4 = p2;
   return 1;
  }
  if(p1>DOPTROW-1-c)
  {
   *p3 = p1 - (DOPTROW-SCRROW);   // please fixed -50
   *p4 = p2;
   return 1;
  }
  if(p1>=a)
  {
   z = SCRROW-1+a-b-c;
   if(z<p1) return 0;
   *p3 = p1 - a+b;
   *p4 = p2;
   return 1;
  }
  return 0;
 }

 int WN_Calc_SVW(int y)
 {
  int a = gwin_SVW_Row, // bp-1
      b = gwin_SLU,     // bp-2
      c = gwin_SLL,     // bp-3
      d = 0,            // bp -4
      e;
  if(y<gwin_SVW_Row&&y>=gwin_SLU)
  {
   gwin_SVW_Row = y;
   d = 1;
  }
  e = a+SCRROW-1-b-c;
  if(e>=y||y>DOPTROW-1-c) return d;
  gwin_SVW_Row = (y+b+c)-(SCRROW-1);
  d = 1;
  return d;
 }

 void WN_Putc(int symb)
 {
  int c,d,e;
  int offset;

  c = gwin_Cur_Row; // bp - 3
  d = gwin_Cur_Col; // bp - 4
  e = gwin_Cur_Col; // bp - 8

  offset = c*DOPTCOL+e;  // bp - 2
 
  if(gwin_Format_Mode == 0 )
  {
   if(gwin_Insert_Mode == 0 )
   {
    gwin_CHR_Mem[offset] = symb;
    gwin_PAM_Mem[offset] = (char)gwin_Protect_Mode;
    WN_Redraw_Ch(c,d);
   }else
   {
    memmove(offset+gwin_CHR_Mem+1,offset+gwin_CHR_Mem,DOPTCOL-1-e);
    memmove(offset+gwin_PAM_Mem+1,offset+gwin_PAM_Mem,DOPTCOL-1-e);
    gwin_CHR_Mem[offset] = symb;
    gwin_PAM_Mem[offset] = (unsigned char)gwin_Protect_Mode; // hmm append
    WN_Redraw_Line(c);
   }
   if(d<DOPTCOL) 
    WN_Set_Cursor(c,d+1);
   else PDC_New_Line();
  }else
  {
    if(gwin_Insert_Mode == 0 )
    {
     if(gwin_PAM_Mem[offset]==0)
     {
       gwin_CHR_Mem[offset] = symb;
       WN_Redraw_Ch(c,d);
       if(d<DOPTCOL)
        WN_Set_Cursor(c,d+1);
       else 
        PDC_New_Line();
     }
    }else
    {
     if(gwin_PAM_Mem[offset]==0)
     {
      int g,i;
      i = g = offset+1;
      while(i<DOPTROW*DOPTCOL)
      {
       if(gwin_PAM_Mem[i]) break;
       ++i;
      }
      --i;
      memmove(gwin_CHR_Mem+g,gwin_CHR_Mem+offset,i-offset);
      gwin_CHR_Mem[offset] = symb;
      WN_Redraw_Line(c);
      if(d<DOPTCOL)
       WN_Set_Cursor(c,d+1);
      else PDC_New_Line();
     }// else bracket
    }
  }// last last if
  return;
 }

 
 void WN_Redraw_Ch(int y,int x)
 {
  int y1,x1,offset;
  if(WN_Calc_Dspl(y,x,&y1,&x1))
  {
   offset = y*DOPTCOL+x;
   ScreenDevice.OutputChr(TRUE,x1,y1,gwin_DAM_Mem[offset],gwin_CHR_Mem[offset]);
   return;
  }
 }

 void WN_Set_Cursor(int y,int x)
 {
  int a,b;
  if(y<DOPTROW && x<DOPTCOL)
  {
   int ret = WN_Calc_SVW(y);
   if(ret)
   {
    WN_Redraw_Partial_Line_Fix(gwin_SVW_Row,0,SCRROW-gwin_SLU-gwin_SLL);
   }
   gwin_Cur_Row = y;
   gwin_Cur_Col = x;
   WN_Calc_Dspl(y,x,&a,&b);
   ScreenDevice.CursorPos(FALSE,b,a);
  }
 }

 void WN_Redraw_All(void)
 {
  int y1,y2;
  y1 = gwin_SLU; // bp-1
  y2 = gwin_SLL; // bp-2
  if(gwin_SLU)
  {
   WN_Redraw_Partial_Line_Fix(0,0,gwin_SLU);
  }
  WN_Redraw_Partial_Line_Fix(gwin_SVW_Row,0,SCRROW-y1-y2);
  if(y2)
  {
   WN_Redraw_Partial_Line_Fix(DOPTROW-y2,0,y2);
  }
 }

 void WN_Redraw_Client_Area(void)
 {
  WN_Redraw_Partial_Line_Fix(gwin_SVW_Row,0,SCRROW-gwin_SLU-gwin_SLL);
 }
 
 void WN_Redraw_Line(int count)
 {
  WN_Redraw_Partial_Line_Fix(count,0,1);
 }

 void WN_Redraw_Partial_Line_Fix(int p1,int p2,int line)
 {
  int offset,a,b;
  if(WN_Calc_Dspl(p1,p2,&a,&b))
  {
   offset = p1*DOPTCOL;
   ScreenDevice.OutputLine(TRUE,a,(char*)&gwin_CHR_Mem[offset],(char*)&gwin_DAM_Mem[offset],line);
  }
 }

