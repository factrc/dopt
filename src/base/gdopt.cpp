#define _GCONSOLE_DEF

 #include "dopt.h"

 CMessages   *pIOMessage;
 GConsole     ScreenDevice;
 Comm         IODevice;
 CModuleBase *pModuleDevice;

 static long WaitingMessages(void);
/* static long TestingFont(HWND wnd);*/

 struct _d_font
 {
  struct _d_font *next;
  ENUMLOGFONTEX enum_logfont;
  NEWTEXTMETRICEX enum_textmetric;
  int font_type;
 };

 DWORD        __EventTime;
 long         __EventCode;

 int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPInstance,LPSTR lpCmdLine,int nCmdShow)
 {
  DWORD rcode,x,y,mode;
#ifdef _DEBUG_LEAK
  long tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
       tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
       tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;

  _CrtSetDbgFlag(tmpDbgFlag);

  OFSTRUCT ddd;
  HFILE hStdout = OpenFile("c:/1.log",&ddd,OF_CREATE|OF_WRITE);
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, (void*)hStdout);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ERROR, (void*)hStdout);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, (void*)hStdout);
#endif

  pIOMessage    = new CMessages(-1);
  pModuleDevice = new CModuleBase();

  if(parse_argument(lpCmdLine)<0) 
  {
   return -1;
  }
  InitDebugSection();

  mode  = get_option("WINDOW_mode")->i?1:0;
  x     = get_option("CONSOLE_col")->i;
  y     = get_option("CONSOLE_row")->i;
  if(!ScreenDevice.Init(mode,x,y))
  {
   message(F_DEBUG,"Не найден подходящий для консоли фонт");
   MessageBox(NULL,"Не найден подходящий для консоли фонт","Ошибка",MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
   return -1;
  }
/*  TestingFont(ScreenDevice.GetWindowHandle());*/
  if(!init_dopt_emulation(ScreenDevice.GetWindowHandle()))
  {
   MessageBox(NULL,"Инициализация DOPT терминала","Ошибка",MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
   ScreenDevice.Close();
   return -1;
  }
  ScreenDevice.CursorVisible(TRUE,CURSOR_ENABLE);
  ScreenDevice.CursorSize(TRUE,CURSOR_BLOCK);
#ifdef __SLOW_USE
  SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
#endif
  try
  {
   rcode = WaitingMessages();
  }catch(...)
  {
   message(F_DEBUG,"Try,Catch(...)WaitMessages\n");
  }
  try
  {
   ScreenDevice.Close();
   IODevice.Close();
   pModuleDevice->Clear();
   delete pIOMessage;
   delete pModuleDevice;
  }catch(...)
  {
   message(F_DEBUG,"Try,Catch(...)Closed\n");
  }
  pIOMessage    = NULL;
  pModuleDevice = NULL;
#ifdef _DEBUG_LEAK
  _CrtDumpMemoryLeaks();
#endif
  return rcode;
 }

#define EVENT_MSG       0
#define EVENT_SCREEN    1
#define EVENT_READ      2
#define EVENT_WRITE     3

#define FPS_MSEC        50

 long WaitingMessages(void)
 {
  long size,result,BufferIOSize = 0;
  unsigned char buf[MAX_READ_ONE_SHOT+10],BufferIO[MAX_READ_ONE_SHOT*3];
  MSG    msg;
  DWORD  stopping = 0;
  HANDLE events_array[4];
  DWORD  dwTimes = INFINITE;
  DWORD  dwTimeStart;

  events_array[EVENT_WRITE]  = IODevice.WEvent();  
  events_array[EVENT_READ]   = IODevice.REvent();
  events_array[EVENT_SCREEN] = ScreenDevice.GetUpdateEvent();
  events_array[EVENT_MSG]    = pModuleDevice->GetEvent();

  IODevice.Select();
  __EventCode = -1;
  DWORD bUpdateComplete = TRUE;

  dwTimeStart = GetTickCount();

  while(!stopping)
  {
   result = MsgWaitForMultipleObjects(4,events_array,FALSE,dwTimes,QS_ALLINPUT);
   switch(result)
   {
    case (WAIT_OBJECT_0+EVENT_WRITE):  // WRITECOMM
     IODevice.WriteFlush();
    break;                   
    case (WAIT_OBJECT_0+EVENT_READ):  // READCOMM
     if((size = IODevice.Read(buf,MAX_READ_ONE_SHOT))>0)
     {
      if(BufferIOSize+size>=sizeof(BufferIO))
      {
       CPP_Parser(BufferIO,BufferIOSize);
       ScreenDevice.Flush();
       BufferIOSize = 0;
      }
      memcpy(BufferIO+BufferIOSize,buf,size);
      BufferIOSize += size;
     }
     if(bUpdateComplete == TRUE && BufferIOSize)
     {
      CPP_Parser(BufferIO,BufferIOSize);
      ScreenDevice.Flush();
      BufferIOSize = 0;
     }
     IODevice.Select();
    break;
    case (WAIT_OBJECT_0+EVENT_SCREEN): // SCREEN
#ifdef __SLOW_USE
     if(bUpdateComplete == TRUE)
     {
      if(dwTimes == INFINITE)
      {
       dwTimeStart = GetTickCount();
       dwTimes     = FPS_MSEC;
       continue;
      }else
      {
timeout_label:
       dwTimeEnd = GetTickCount();
       if(dwTimeEnd<dwTimeStart) dwTimeStart = dwTimeEnd;
       if(dwTimeEnd<(dwTimeStart+FPS_MSEC))
       {
        continue;
       }
       dwTimes = INFINITE;
      }
     }
#endif
     if(ScreenDevice.Update() == 1)
     {
       bUpdateComplete = TRUE;
       if(BufferIOSize)
       {
        CPP_Parser(BufferIO,BufferIOSize);
        ScreenDevice.Flush();
        BufferIOSize = 0;
       }
     }else   bUpdateComplete = FALSE;
    break;
    case (WAIT_OBJECT_0+EVENT_MSG): // WINDOW
     pModuleDevice->ExecuteModule();
    break;
    case WAIT_TIMEOUT:
#ifdef __SLOW_USE
     goto timeout_label;
#endif
    break;
   }
   while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
   { 
    if(msg.message == WM_QUIT)
    {
     stopping = 1;
     break;
    }
    DispatchMessage(&msg); 
   }
  }
  return msg.wParam;
 }

/*
 static int CALLBACK FFontProc(ENUMLOGFONTEX *lpelfe,NEWTEXTMETRICEX*lpntme,unsigned long FontType,LPARAM lParam)
 {
  struct _d_font **list = (struct _d_font **)lParam;
  struct _d_font *ptr;
  ptr = (struct _d_font *)calloc(1,sizeof(struct _d_font));
  if(ptr)
  {
   if(lpelfe) memcpy(&ptr->enum_logfont,lpelfe,sizeof(ENUMLOGFONTEX));
   if(lpntme) memcpy(&ptr->enum_textmetric,lpntme,sizeof(NEWTEXTMETRICEX));
   ptr->font_type = FontType;
   if(*list) ptr->next = *list;
   *list = ptr;
  }
  return 1;
 }
 static long TestingFont(HWND wnd)
 {
  struct _d_font *list=NULL,*tmp,*ptr;
  HDC dc = GetDC(wnd);
  LOGFONT lfnt;
  memset(&lfnt,0,sizeof(LOGFONT));
  lfnt.lfPitchAndFamily = FIXED_PITCH;
  message(F_DEBUG," ------ Checking compatibles fonts ------\n");
  EnumFontFamiliesEx(dc,&lfnt,(FONTENUMPROC)FFontProc,(long)&list,0);
  ReleaseDC(wnd,dc);
  ptr = list;
  while(ptr)
  {
   message(F_DEBUG,"Name: %-20s                   Style:%-10s\n",
           ptr->enum_logfont.elfFullName,ptr->enum_logfont.elfStyle);
   ptr= ptr->next;
  }
  ptr = list;
  while(ptr)
  {
   tmp = ptr->next;
   free(ptr);
   ptr = tmp;
  }
  message(F_DEBUG," ------ Stoping scan fonts ------\n");
  return 1;
 }
*/
