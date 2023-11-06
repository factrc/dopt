
  #include "dopt.h"

  #define M_A_X            3
  #define M_A_Y            5
  #define M_B_X            44
  #define M_B_Y            5
  #define M_PHONE_SIZE     25
  #define M_DELAY_MSG      5000
  #define M_DELAY_MSG2     5000
  #define M_DELAY_ASYNCMSG 3000
  #define M_ATTR           0x1E
  #define M_ATTR1          0x20
  #define M_ATTR2          0x4E
  #define M_POS_STR        "43 3"

  #define INITPROC    long (__cdecl *)(const char *)
  #define DONEPROC    long (__cdecl *)(void)
  #define PROCESSPROC long (__cdecl *)(void ** ,const char *)
  #define GETPROC     long (__cdecl *)(long)
  #define SETPROC     long (__cdecl *)(long,void*,long)


  struct PARSECMD
  {
   CValue *m_phoneA;
   CValue *m_phoneB;
   CValue *m_DPTCol;
   CValue *m_DPTRow;
   CValue *m_DPTBuffer;
   CValue *m_DPTViewRowTop;
   CValue *m_DPTViewRowBottom;
  };

  extern unsigned long str_key_to_scan(const char*);

//extern unsigned long get_key_from_scan( unsigned long scan );

  static void* EM_malloc(int size)
  {
   return malloc(size);
  }
  static void* EM_calloc(int num,int size)
  {
   return calloc(num,size);
  }
  static void* EM_realloc(void *ptr,int size)
  {
   return realloc(ptr,size);
  }
  static void  EM_free(void *ptr)
  {
   try
   {
    if(ptr)
     free(ptr);
   }catch(...)
   {

   }
  }

  static unsigned char* bin_string(char*str,int *sz)
   {
    int buflen = 0,len=0;
    unsigned char *pbin = NULL;
    
//        "\\10\a\\D\\AF"
    while( *str )
     {
      if ( len+4 > buflen )
       {
        buflen +=1024;
        pbin = (unsigned char *)realloc(pbin,buflen);
       }
      if( str[1] && str[0] == '\\' && (str[1] =='x'||str[1] =='X') ) 
       {
        int a;
        char tmp[20];
        char *e = &str[2];
        while( *e && isxdigit(*e))e++;
        strncpy(tmp,&str[2],e-(&str[2]));
        tmp[e-(&str[1])-1] = 0;
        sscanf(tmp,"%X",&a);
        pbin[len++] = a&0xFF;
        str = e;
       }else 
        pbin[len++] = *str++;
     }
    *sz = len;
    return pbin;
   }


/*
  static unsigned char* bin_string(char*str,int *sz)
  {
   char tmp[255];
   char*st = NULL;
   unsigned char *pbin = NULL;
   int a;
   *sz=0;
   while(str[0])
   {
    while(str[0]&&str[0]!='\\') str++;
    if(st)
    {
     a = 0;
     memset(tmp,0,sizeof(tmp));
     strncpy(tmp,st,str-st);
     if(sscanf(tmp,"%X",&a)!=1)
     {
      if(pbin) free(pbin);
      *sz = 0;
      return pbin = NULL;
     }
     pbin = (unsigned char*)realloc(pbin,(*sz)+1);
     pbin[*sz] = a;
     *sz = *sz+1;
     st = NULL;
    }
    if(!str[0]) break;
    str++;              // skip '\'
    if(!str[0])
    {
     if(pbin) free(pbin);
     *sz = 0;
     return pbin = NULL;
    }else
     str++;
    st = str;
   }
   return pbin;
  }

*/


  CModule::CModule(char *name,char *fname)
  {
   m_fOpen            = FALSE;
   m_hModule          = NULL;
   m_Key              = 0;
   FileName           = NULL;
   ModuleName         = NULL;
   opt.Key            = NULL;
   opt.Msg            = NULL;
   opt.swap           = 0;
   opt.pha.x          = M_A_X;
   opt.pha.y          = M_A_Y;
   opt.pha.sz         = M_PHONE_SIZE;
   opt.phb.x          = M_B_X;
   opt.phb.y          = M_B_Y;
   opt.phb.sz         = M_PHONE_SIZE;
   m_hModule = LoadLibrary(fname);
   if(m_hModule)
   {
    p_init      = (INITPROC)   GetProcAddress(m_hModule,"_InitPlugin");
    p_done      = (DONEPROC)   GetProcAddress(m_hModule,"_DonePlugin");
    p_process   = (PROCESSPROC)GetProcAddress(m_hModule,"_ProcessPlugin");
    p_getstate  = (GETPROC)    GetProcAddress(m_hModule,"_GetPluginState");
    p_setstate  = (SETPROC)    GetProcAddress(m_hModule,"_SetPluginState");
    if(!p_init) 
     p_init     = (INITPROC)   GetProcAddress(m_hModule,"InitPlugin");
    if(!p_done)
     p_done     = (DONEPROC)   GetProcAddress(m_hModule,"DonePlugin");
    if(!p_process)
     p_process  = (PROCESSPROC)GetProcAddress(m_hModule,"ProcessPlugin");
    if(!p_getstate)
     p_getstate = (GETPROC)    GetProcAddress(m_hModule,"GetPluginState");
    if(!p_setstate)
     p_setstate = (SETPROC)    GetProcAddress(m_hModule,"SetPluginState");
   }
   FileName           = strdup(fname);
   ModuleName         = strdup(name);
  }

  CModule::~CModule()
  {
   if(m_hModule)
    FreeLibrary(m_hModule);
   if(opt.Key)
    free(opt.Key);
   if(opt.Msg)
    free(opt.Msg);
   if(FileName)
    free(FileName);
   if(ModuleName)
    free(ModuleName);
  }

  long CModule::Open()
  {
   char str[2048];
   CValue m_hwnd    ("HWND"    );
   CValue m_Free    ("FREE"    );
   CValue m_Malloc  ("MALLOC"  );
   CValue m_Realloc ("REALLOC" );

   if(opt.Key)
   {
    m_Key = str_key_to_scan(opt.Key);
    free(opt.Key);
    opt.Key = 0;
   }

   m_hwnd    = (void*)ScreenDevice.GetWindowHandle();
   m_Malloc  = (void*)EM_malloc;
   m_Realloc = (void*)EM_realloc;
   m_Free    = (void*)EM_free;
   strcpy(str,(char*)m_hwnd);
   strcat(str,(char*)m_Malloc);
   strcat(str,(char*)m_Realloc);
   strcat(str,(char*)m_Free);
   if(d_init((const char*)str)<0)
   {
    m_fOpen = 0;
    return -1;
   }
   return 1;
  }

  long CModule::Close()
  {
   return 1;
  }
  void onlynumber(char *str)
  {
   char *p;
   trim(str);
   p = str;
   while(*str && !isdigit(*str)) str++;
   if(p!=str)
    memmove(p,str,strlen(str)+1);
   str = p;
   while(*str)
   {
    if(!isdigit(*str))
     memmove(str,str+1,strlen(str+1)+1);
    else str++;
   }
  }

  long CModule::SetInputCmd(SCMD *p)
  {
   char *str;
   int a[3],b[3],sz;
   SCMD     cmd;
   PARSECMD *pp;


   cmd.type     = CMD_BIN;
   cmd.bin.size = sizeof(PARSECMD);
   cmd.data     = (unsigned char*)calloc(1,cmd.bin.size);
   pp           = (struct PARSECMD *)cmd.data;

   pp->m_phoneA           = new CValue("PHONEA"        );
   pp->m_phoneB           = new CValue("PHONEB"        ), 
   pp->m_DPTCol           = new CValue("DOPTCOL"       ), 
   pp->m_DPTRow           = new CValue("DOPTROW"       ), 
   pp->m_DPTBuffer        = new CValue("DOPTBUFFER"    ), 
   pp->m_DPTViewRowTop    = new CValue("DOPTVROWTOP"   ), 
   pp->m_DPTViewRowBottom = new CValue("DOPTVROWBOTTOM"); 

   // fill data
   *(pp->m_DPTCol)           = (int)DOPTCOL;
   *(pp->m_DPTRow)           = (int)DOPTROW;
   *(pp->m_DPTBuffer)        = (void*)gwin_CHR_Mem;
   *(pp->m_DPTViewRowTop)    = (int)gwin_MLU;
   *(pp->m_DPTViewRowBottom) = (int)gwin_MLL;

   if(opt.swap)
   {
    a[0] = opt.phb.x;
    a[1] = opt.phb.y;
    a[2] = opt.phb.sz;
    b[0] = opt.pha.x;
    b[1] = opt.pha.y;
    b[2] = opt.pha.sz;     
   }else
   {
    a[0] = opt.pha.x;
    a[1] = opt.pha.y;
    a[2] = opt.pha.sz;
    b[0] = opt.phb.x;
    b[1] = opt.phb.y;
    b[2] = opt.phb.sz;
   }
   sz  = max(a[2],b[2])+1;

   str = (char*)calloc(1,sz);
   {
    int i;
    for(i = 0;i<a[2];i++)  str[i] = gwin_CHR_Mem[a[0]+a[1]*DOPTCOL+i];
    str[i] = 0;
    if(opt.num)onlynumber(str);
    *pp->m_phoneA = (char*)str;
    for(i = 0;i<b[2];i++)  str[i] = gwin_CHR_Mem[b[0]+b[1]*DOPTCOL+i];
    str[i] = 0;
    if(opt.num)onlynumber(str);
    *pp->m_phoneB = (char*)str;
   }
   free(str);
   m_InCmd.Push(&cmd);
   free(cmd.data);
   return 1;
  }


  long  CModule::Execute(void)
  {
   char    *str = NULL,*result;
   long     ret;
   PARSECMD *pp;
   SCMD     cmd;

   if(m_InCmd.Pop(&cmd)<0)
    return -1;

   if(cmd.bin.size != sizeof(PARSECMD))
   {
    free(cmd.data);
    return -1;
   }

   pp = (PARSECMD*)cmd.data;
   {
    int sz;
    sz = pp->m_phoneA->sizeA()    + pp->m_phoneB->sizeA()        +
         pp->m_DPTCol->sizeA()    + pp->m_DPTRow->sizeA()        +
         pp->m_DPTBuffer->sizeA() + pp->m_DPTViewRowTop->sizeA() +
         pp->m_DPTViewRowBottom->sizeA();
    str  = (char*)realloc(str,sz+1);
    strcpy(str,(char*)*pp->m_phoneA);
    strcat(str,(char*)*pp->m_phoneB);
    strcat(str,(char*)*pp->m_DPTCol);
    strcat(str,(char*)*pp->m_DPTRow);
    strcat(str,(char*)*pp->m_DPTBuffer);
    strcat(str,(char*)*pp->m_DPTViewRowTop);
    strcat(str,(char*)*pp->m_DPTViewRowBottom);
    delete pp->m_phoneA;
    delete pp->m_phoneB;
    delete pp->m_DPTCol;
    delete pp->m_DPTRow;
    delete pp->m_DPTBuffer;
    delete pp->m_DPTViewRowTop;
    delete pp->m_DPTViewRowBottom;
   }
   free(cmd.data);
//   m_OutCmd.Clear();
   cmd.type     = CMD_DEBUG;
   cmd.data     = (unsigned char*)"Enter_Module: Ok";
   m_OutCmd.Push(&cmd);
   ret = d_process((void**)&result,(const char*)str);
   free(str);
   if(ret < 0)
    {
     sscanf(M_POS_STR,"%d%d",&cmd.msg.p.x,&cmd.msg.p.y);
     cmd.msg.attr = M_ATTR2;
     cmd.msg.ttl  = M_DELAY_ASYNCMSG/2;
     cmd.type     = CMD_MSG;
     char buf[255];
     strcpy(buf,"База данных недоступна");
     int size = 80 - (cmd.msg.p.x+strlen(buf));
     if(size<0) size = strlen(buf);
     else       size+=strlen(buf);
     for ( int i = strlen(buf);i<size&&i<253;i++ )   buf[i] = ' ';
     buf [ size ] = 0;
     cmd.data     = (unsigned char*)buf;
     m_OutCmd.Push(&cmd);
     cmd.type     = CMD_DEBUG;
     cmd.data     = (unsigned char*)"Leave_Module: Error";
     m_OutCmd.Push(&cmd);
     return -2;
    }
   parse_return_value((char*)result);
   EM_free(result);
   cmd.type     = CMD_DEBUG;
   cmd.data     = (unsigned char*)"Leave_Module: Ok";
   m_OutCmd.Push(&cmd);
   return 1;
  }

  long CModule::parse_return_value(char*rstr)
  {
   int     size;
   char    *str = NULL,pstr[40];
   SCMD     cmd;
   CValue m_ttl      ("TTL"     );
   CValue m_pos      ("POS"     );
   CValue m_ttl2     ("TTL2"    );
   CValue m_attr     ("ATTR"    );
   CValue m_send     ("SEND"    );
   CValue m_attr2    ("ATTR2"   );
   CValue m_message  ("MESSAGE" );
   CValue m_message2 ("MESSAGE2");

   m_send     = "";
   m_message  = "";
   m_message2 = "";
   m_ttl      = (int)M_DELAY_MSG;   
   m_ttl2     = (int)M_DELAY_MSG2;
   m_attr     = (void*)M_ATTR;
   m_attr2    = (void*)M_ATTR2;
   m_pos      = (char*)M_POS_STR;

   m_pos.from(rstr);
   m_ttl.from(rstr);
   m_ttl2.from(rstr);
   m_send.from(rstr);
   m_attr.from(rstr); 
   m_attr2.from(rstr);
   m_message.from(rstr);
   m_message2.from(rstr);

   size   = m_message.size();
   str    = (char*)realloc(str,size+1);
   str[0] = 0;
   m_message.get((char*)str);
   if(str[0])
   {
    m_attr.get((void**)&cmd.msg.attr);
    m_ttl.get((void**)&cmd.msg.ttl);
    m_pos.get((char*)pstr);
    sscanf(pstr,"%d%d",&cmd.msg.p.x,&cmd.msg.p.y);
    cmd.type     = CMD_MSG;
    cmd.data     = (unsigned char*)str;
    m_OutCmd.Push(&cmd);
   }
   size   = m_message2.size();
   str    = (char*)realloc(str,size+1);
   str[0] = 0;
   m_message2.get((char*)str);
   if(str[0])
   {
    m_attr2.get((void**)&cmd.msg.attr);
    m_ttl2.get((void**)&cmd.msg.ttl);
    cmd.type     = CMD_MSG;
    cmd.data     = (unsigned char*)str;
    cmd.msg.p.x  = 0;
    cmd.msg.p.y  = SCRROW+1;
    m_OutCmd.Push(&cmd);
   }
   size   = m_send.size();
   str    = (char*)realloc(str,size+1);
   str[0] = 0;
   m_send.get((char*)str);
   if(str[0])
   {
    int szbin;
    unsigned char *pbin = bin_string(str,&szbin);
    if(pbin)
    {
     cmd.type     = CMD_BIN;
     cmd.data     = pbin;
     cmd.bin.size = szbin;
     m_OutCmd.Push(&cmd);
     free(pbin);
    }
   }
   if(str) free(str);
   return 1;
  }


  long CModule::d_init(const char*str)
  {
   long ret = -1;
   try
   {
    ret = p_init(str);
   }catch(...)
   {
   }
   return ret;
  }
  long CModule::d_done(void)
  {
   long ret = -1;
   try
   {
    ret = p_done();
   }catch(...)
   {
   }
   return ret;
  }
  long CModule::d_process(void**res,const char*param)
  {
   long ret = -1;
   try
   {
    ret = p_process(res,param);
   }catch(...)
   {
   }
   return ret;
  }
  long CModule::d_getstate(long state)
  {
   long ret = -1;
   try
   {
    ret = p_getstate(state);
   }catch(...)
   {
   }
   return ret;
  }
  long CModule::d_setstate(long state,void*buf,long size_buf)
  {
   long ret = -1;
   try
   {
    ret = p_setstate(state,buf,size_buf);
   }catch(...)
   {
   }
   return ret;
  }

  int CModule::SetSwap(int swap)
  {
   opt.swap = swap;
   return 1;
  }
  int CModule::SetMessage(char *str)
  {
   opt.Msg = strdup(str);
   return 1;
  }
  int CModule::SetKey(char *str)
  {
   opt.Key = strdup(str);
   return 1;
  }

  long CModule::GetKeyName ( char *buf, int size )
   {
    if ( !buf || !opt.Key )
     return 0;
    if ( (int)strlen( opt.Key ) > size )
     return 0;
    strncpy(buf,opt.Key,size);
    size = strlen(opt.Key);
    buf[size] = 0;
    return size;
   }
  int CModule::SetPhoneB(char *str)
  {
   return sscanf(str,"%d/%d/%d",&opt.phb.x,&opt.phb.y,&opt.phb.sz) == 3;
  }

  int CModule::SetPhoneA(char *str)
  {
   return sscanf(str,"%d/%d/%d",&opt.pha.x,&opt.pha.y,&opt.pha.sz) == 3;
  }

  long CModule::PutAsyncMessage(void)
  {
   SCMD     cmd;
   sscanf(M_POS_STR,"%d%d",&cmd.msg.p.x,&cmd.msg.p.y);
   cmd.msg.attr = M_ATTR1;
   cmd.msg.ttl  = M_DELAY_ASYNCMSG;
   cmd.type     = CMD_MSG;
   cmd.data     = (unsigned char*)opt.Msg;
   m_OutCmd.Push(&cmd);
   return 1;
  }