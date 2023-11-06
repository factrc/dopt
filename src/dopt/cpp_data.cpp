
 #include "dopt.h"

 unsigned char CPP_Waitbuf[CPP_WAITBUFFER];
 unsigned char  CPP_ch;

 void (*CPP_Waitproc)(void);
 int   CPP_Waitlength;
 int   CPP_Cur_State;
 int   CPP_Waitbuf_Indx;


 void CPP_Init(void)
 {
   CPP_Cur_State = 0;
 }

 void CPP_Parser(unsigned char*Buffer,int szBuffer)// unsigned char ch)
 {
  unsigned int ch;
 for(int i=0;i<szBuffer;i++)
 {
  ch = Buffer[i];
  if(!ch) continue;

  if(gwin_Test_Mode==1)
  {
    switch(CPP_Cur_State)
    {
     case 0:  
      WN_Putc(ch);
	  if(ch==0x1B)
       CPP_Cur_State = 7;
	 break;
     case 7:
	  if(ch==0x7A)
       PDC_Test_Off();
      else
       WN_Putc(ch);
	  if(ch==0x1B)
       CPP_Cur_State = 7;
      else 
       CPP_Cur_State = 0;
     break;
    }
  }else
  {
   CPP_ch = ch;
   switch(ch)
   {
    case 0x18:
     if(CPP_Cur_State==0xB)
     {
       PRN_Init();
	   if(!PRN_Check_Print_Rdy())
        PRN_Flush();
     }
     CPP_Cur_State=0;
	 PDC_Init();
    break;
    case 0x5:
     if(CPP_Cur_State!=0xB)
      RS232_Put_Char(6);
    break;
    default:
     if(CPP_Cur_State<=0xC)
     {
	  switch(CPP_Cur_State)
      {
	   case 0:
	   case 1:
	   case 2:
	   case 3:   //E0
         CPP_Waitproc   = Table[CPP_Cur_State][ch].Proc;
		 CPP_Waitlength = Table[CPP_Cur_State][ch].Len;	   
		 CPP_Cur_State  = Table[CPP_Cur_State][ch].State;

          if(CPP_Cur_State==0x5)   CPP_Waitbuf_Indx = 0;
          else                     (*CPP_Waitproc)();
       break;
       case 4:  //0132
	     if(ch==0x2F)
         {
           CPP_Cur_State = 0;
           continue;
         }
		 CPP_Waitbuf[0]   = ch;
		 CPP_Waitlength   = 1;
		 CPP_Waitbuf_Indx = 1;
		 CPP_Waitproc     = PDC_Compr_Char;
		 CPP_Cur_State    = 5;
       break;
	   case 5: // 0166
	    if(ch == 0x1B)
        {
         CPP_Cur_State = 1;
         continue;
        }
		CPP_Waitbuf[CPP_Waitbuf_Indx++] = ch;
		if(CPP_Waitbuf_Indx==CPP_Waitlength)
        {
		 CPP_Cur_State = 0;
		 (*CPP_Waitproc)();
        }
	   break;
	   case 6:  // 019B
	    if(ch==0x1B)
        {
         CPP_Cur_State = 1;
         continue;
        }
	    if(!isxdigit(ch))
         CPP_Cur_State = 0;
	   break;
	   case 7: 
       break;
	   case 8:
	   case 9:
	   case 0xA:
	    if(ch==0x1B)
        {
         CPP_Cur_State = 1;
         continue;
        }
		switch(CPP_Cur_State)
        {
		 case 8:  PDC_Wrt_Enh_Body(); break;
		 case 9:  PDC_Wrt_Chr_Body(); break;
		 default: PDC_FreePrntBody(); break;
        }
        CPP_Waitlength--;
	    if(!CPP_Waitlength)
         CPP_Cur_State=0;
	   break;			// 01C9
	   case 0xB:	          // 020b
	    PDC_Cpy_Dspl_Prt();
	   break;
	   case 0xC:
	    if(ch==0x1B)
        {
         CPP_Cur_State = 1;
         continue;
        }
	    PDC_Char_Gen();
	   break;
      }
     }
   }
  }//last if
 }
}

