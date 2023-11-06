
 #include "dopt.h"

 static unsigned _stdcall ThreadRoutine(void*context);

 static void SendMessageToConsole(struct SCMD *pCmd);

 CModuleBase::CModuleBase(DWORD Count)
 {
  m_fOpen = TRUE;
  m_RootList.currentModuleCount = 0;
  m_RootList.maxModuleCount     = Count;
  m_RootList.next = &m_RootList;
  m_RootList.prev = &m_RootList;
  m_hWriteEvent   = CreateEvent(NULL,FALSE,0,NULL);
  m_ModuleShared  = CreateEvent(NULL,FALSE,TRUE,NULL);
 }
 CModuleBase::~CModuleBase()
 {
  _ASSERT(m_RootList.next == &m_RootList);
  _ASSERT(m_RootList.prev == &m_RootList);
  _ASSERT(m_RootList.currentModuleCount == 0);
  CloseHandle(m_ModuleShared);
  CloseHandle(m_hWriteEvent);
 }

 int CModuleBase::Lock(int bWait)
 {
  DWORD time    = bWait?INFINITE:0;
  DWORD bStatus = WaitForSingleObject(m_ModuleShared,time);
  if(bStatus == WAIT_OBJECT_0)
   return 1;
  return -1;
 }
 void CModuleBase::Unlock(void)
 {
   SetEvent(m_ModuleShared);
 }

 int CModuleBase::Insert(CModule *mod)
 {
  struct s_modEntry *p;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return -1;
  }
  INC_MODULE_COUNT();
  if(m_RootList.currentModuleCount+1>m_RootList.maxModuleCount)
  {
   DEC_MODULE_COUNT();
   message(F_DEBUG,"CModuleBase::Insert() : currentModuleCount > maxModuleCount \n");
   Unlock();
   return -1;
  }
  p = alloc_item(mod);
  if(p)
  {
   p->prev               = m_RootList.prev;
   p->next               = m_RootList.prev->next;
   m_RootList.prev->next = p;
   m_RootList.prev       = p;
  }
  Unlock();
  return p?1:-1;
 }
 
 int CModuleBase::Clear()
 {
  int bStatus = -1;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return -1;
  }
  struct s_modEntry *p = m_RootList.next,*tmp;
  while(p!=&m_RootList)
  {
   tmp = p->next;
   DEC_MODULE_COUNT();
   p->prev->next = p->next;
   p->next->prev = p->prev;
   free_item(p);
   bStatus = 1;
   p = tmp;
  }
  Unlock();
  return bStatus;
 }

 struct s_modEntry *CModuleBase::alloc_item(CModule *pmod)
 {
  struct s_modEntry *p = (struct s_modEntry*)calloc(1,sizeof(struct s_modEntry));
  if(p)
  {
   p->entryspot  = pmod;
   p->hStart     = CreateEvent(NULL,TRUE,FALSE,NULL);
   p->hExit      = CreateEvent(NULL,TRUE,FALSE,NULL);
   p->hRead      = CreateEvent(NULL,TRUE,FALSE,NULL);
   p->hWrite     = &m_hWriteEvent;
   p->hThread    = (HANDLE)_beginthreadex(NULL,0,ThreadRoutine,p,0,(unsigned *)&p->idThread);
   p->owner      = this;
   p->errorCount = 0;
  }
  message(F_DEBUG,"CModuleBase::Insert() : insert Ok\n");
  return p;
 }

 void CModuleBase::free_item(struct s_modEntry *p)
 {
  DWORD bThreadStatus;
  if(!p) return;
  SetEvent(p->hExit);
  bThreadStatus = WaitForSingleObject(p->hThread,1000);
  
  if( bThreadStatus != WAIT_OBJECT_0 )
  {
   message(F_DEBUG,"CModuleBase::Remove() : bThreadStatus = 0x%X\n",bThreadStatus);
   TerminateThread(p->hThread,0);
  }
  CloseHandle(p->hExit);
  CloseHandle(p->hThread);
  CloseHandle(p->hStart);
  if(p->entryspot)
  {
   CModule *mm = (CModule*)p->entryspot;
   delete mm;
  }
  free(p);
 }

 int CModuleBase::Remove(CModule *mod)
 {
  int bStatus = -1;
  Lock();
  if(m_fOpen == FALSE || mod == NULL )
  {
   Unlock();
   return -1;
  }
  struct s_modEntry *p = m_RootList.next;
  while(p!=&m_RootList)
  {
   if(p->entryspot == mod) 
    break;
   p = p->next;
  }
  if(p != &m_RootList)
  {
   DEC_MODULE_COUNT();
   p->prev->next = p->next;
   p->next->prev = p->prev;
   free_item(p);
   bStatus = 1;
  }
  Unlock();
  return bStatus;
 }

 int CModuleBase::ExecuteCommandIfFound(unsigned long key)
 {
  DWORD mkey;
  int bStatus = FALSE;
  if(Lock(FALSE)<0) return bStatus;
   struct s_modEntry *p = m_RootList.next;
   while(p!=&m_RootList)
   {
    if(p->entryspot)
    {
     CModule *pmod = (CModule*)p->entryspot;
     if((mkey = pmod->GetKey()) != 0 && mkey == key)
     {
      message(F_DEBUG,"Info: ES(%s)\n",(pmod->GetModuleName())?pmod->GetModuleName():"Ooops");
      if(WaitForSingleObject(p->hRead,0) == WAIT_OBJECT_0)
      {
       if(++p->errorCount>10)
       {
        message(F_DEBUG,"Warning: Module not actived very long times\n");
        p->errorCount = 0;
       }
       message(F_DEBUG,"Info: EB(%s)\n",(pmod->GetModuleName())?pmod->GetModuleName():"Ooops");
       break;
      }
      p->errorCount = 0;
      if(p->entryspot->isHaveMessage())
      {
       p->entryspot->PutAsyncMessage();
       SetEvent(*p->hWrite);
      }
      p->entryspot->SetInputCmd(NULL);
      Execute(p);
      bStatus = TRUE;
      message(F_DEBUG,"Info: EE(%s)\n",(pmod->GetModuleName())?pmod->GetModuleName():"Ooops");
     }
    }
    p = p->next;
   }
  Unlock();
  return bStatus;
 }

 int CModuleBase::ExecuteCommandIfFound(void)
 {
  int bStatus = FALSE;
  if(Lock(FALSE)<0) return bStatus;
   struct s_modEntry *p = m_RootList.next;
   while(p!=&m_RootList)
   {
    if(p->entryspot)
    {
     if(WaitForSingleObject(p->hRead,0) == WAIT_OBJECT_0)
     {
      if(++p->errorCount>10)
      {
       message(F_DEBUG,"Module not actived very long times\n");
       p->errorCount = 0;
      }
      break;
     }
     p->errorCount = 0;
     if(p->entryspot->isHaveMessage())
     {
      p->entryspot->PutAsyncMessage();
      SetEvent(*p->hWrite);
     }
     p->entryspot->SetInputCmd(NULL);
     Execute(p);
     bStatus = TRUE;
    }
    p = p->next;
   }
  Unlock();
  return bStatus;
 }

 int CModuleBase::Execute(struct s_modEntry *pe)
 {
  SetEvent(pe->hRead);
  return 1;
 }

 unsigned _stdcall ThreadRoutine(void*context)
 {
  CModule *pmod;
  HANDLE  aEvent[3];
  struct  s_modEntry *smod = (struct s_modEntry*)context;

  if(!smod || !(pmod = smod->entryspot))
      return 0;
  
  WaitForSingleObject(smod->hStart,INFINITE);
  if(pmod->Open() < 0) 
   return 0;
  ResetEvent(smod->hStart);
  aEvent[0] = smod->hExit;
  aEvent[1] = smod->hRead;
  while(1)
  {
   DWORD bStatus = WaitForMultipleObjects(2,aEvent,FALSE,INFINITE);
   if(bStatus == (WAIT_OBJECT_0+0))
    break;
   pmod->Execute();
   ResetEvent(smod->hRead);
   SetEvent(*(smod->hWrite));
  }
  return pmod->Close();
 }

 void CModuleBase::ExecuteModule(void)
 {
  char buffer[255];
  if(Lock(FALSE)<0)
   return;
  struct  s_modEntry *smod = m_RootList.next;
  while(smod != &m_RootList)
  {
   if(smod->entryspot)
   {
    int count = 0;
    struct SCMD cmd;
    while(++count<=100)
    {
     cmd.data = NULL;
     if(smod->entryspot->ReadOutputCmd(&cmd)<0) break;
     switch(cmd.type)
     {
     case CMD_ENTER_MODULE:
      SendMessageToConsole(&cmd);
     break;
     case CMD_LEAVE_MODULE:
      SendMessageToConsole(&cmd);
     break;
     case CMD_DEBUG:
      message(F_DEBUG,"%s\n",(char*)cmd.data);
     break;
     case CMD_MSG:
      SendMessageToConsole(&cmd);
     break;
     case CMD_BIN:
      {
       unsigned char *start = cmd.data;
       unsigned char *ptrs  = cmd.data;
       unsigned char *ptre  = cmd.data+cmd.bin.size;
repeat_find:
       while ( ptrs < ptre && ptrs[0] != '\a' ) ptrs++;
       if ( ptrs != ptre )
        {
          unsigned long count = 0;
          if( ptrs-start )
           IODevice.Write(start,ptrs-start);

          while( ptrs < ptre && ptrs[0]== '\a' )
           {
            count++;
            ptrs++;
           }
          Sleep(250*count);
          if ( ptrs != ptre ) 
           {
            start = ptrs;
            goto repeat_find;
           }
        }else
          IODevice.Write(start,ptre-start);
      }
     break;
     }
     if(cmd.data) 
      free(cmd.data);
    }
    if(count >100 )
    {
     message(F_DEBUG,"Hmm... looped to CModuleBase::ExecuteModule\n");
     break;
    }
   }
   smod = smod->next;
  }
  Unlock();
 }

 void SendMessageToConsole(struct SCMD *pCmd)
 {
  MSGINFO im;
  DWORD FColor,BColor;

  ScreenDevice.CalcColor(pCmd->msg.attr,&FColor,&BColor);

  im.type                       = pCmd->type;
  im.ttl                        = pCmd->msg.ttl;
  im.hWnd                       = ScreenDevice.GetWindowHandle();
  im.BColor                     = BColor;
  im.FColor                     = FColor;
  im.Msg                        = (char*)pCmd->data;
  im.Font.lfont.lfWeight        = FW_NORMAL;
  im.Font.lfont.lfOrientation   = 0;
  im.Font.lfont.lfEscapement    = 0;
  im.Font.lfont.lfItalic        = FALSE;
  im.Font.lfont.lfUnderline     = FALSE;
  im.Font.lfont.lfStrikeOut     = FALSE;
  im.Font.lfont.lfCharSet       = DEFAULT_CHARSET;
  im.Font.lfont.lfOutPrecision  = OUT_DEFAULT_PRECIS;
  im.Font.lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  im.Font.lfont.lfQuality       = DEFAULT_QUALITY;
  if(ScreenDevice.FontName(NULL,0)<sizeof(im.Font.lfont.lfFaceName)-1)
    ScreenDevice.FontName(im.Font.lfont.lfFaceName,sizeof(im.Font.lfont.lfFaceName));
  ScreenDevice.FontSize((int*)&im.Font.lfont.lfWidth,(int*)&im.Font.lfont.lfHeight);
  ScreenDevice.CalcPos(pCmd->msg.p.x,pCmd->msg.p.y,(int*)&im.Pos.x,(int*)&im.Pos.y);
  pIOMessage->Insert(&im);
 }


 int MSGQUEUE(void)
 {
  MSG msg;
  while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
  { 
   if(msg.message == WM_QUIT)
   {
    break;
   }
   DispatchMessage(&msg); 
  }
  return (msg.message != WM_QUIT)?1:0;
 }


 int CModuleBase::Init(void)
 {
  int bCode = 1;
  DWORD bStatus,bStart = 0;
  int x,y;
  Lock();
  struct  s_modEntry *smod = m_RootList.next;
  while(smod != &m_RootList)
  {
   if(smod->entryspot && smod->entryspot->GetModuleName())
    message(F_DEBUG|F_OUTPUT,"Инициализация расширения: %s :",smod->entryspot->GetModuleName());
   ScreenDevice.GetCursorPos(&x,&y);
   ScreenDevice.CursorPos(1,x,y);
   MSGQUEUE();
   DWORD stime = GetTickCount();
   SetEvent(smod->hStart);
   while(GetTickCount()-stime<INIT_MODULE_TIMEOUT*1000)
   {
    if(!MSGQUEUE())
    {
     bCode = -1;
     break;
    }
    bStatus = WaitForSingleObject(smod->hStart,0);
    if(bStatus == WAIT_TIMEOUT)
     {
      bStart = 1;
      break;
     }
    Sleep(1000);
    ScreenDevice.GetCursorPos(&x,&y);
    if ( x >= SCRCOL )
     {
      message(F_OUTPUT,"\n");
     }
    message(F_OUTPUT,"*");
   }
   if(bCode == -1) break;
   if ( bStart )
    {
     message(F_DEBUG|F_OUTPUT,"%s\n","Ok");
    }else
     {
      message(F_DEBUG|F_OUTPUT,"\nИнициализация расширения: %s : ошибка(timeout)\n",smod->entryspot->GetModuleName());
     }
   MSGQUEUE();
   smod = smod->next;   
  }
  Unlock();
  return bCode;
 }
