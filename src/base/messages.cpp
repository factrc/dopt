  #include "dopt.h"



  static LRESULT CALLBACK MyFunc (HWND, UINT, WPARAM, LPARAM);
  static unsigned int __stdcall ThreadProc(void*);

  CMessages::CMessages(int MaxMessages)
  {
   unsigned id;
   memset(&m_MessageList,0,sizeof(m_MessageList));
   m_BannerShared = CreateEvent(NULL,FALSE,TRUE,NULL);
   m_MessageList.next = &m_MessageList;
   m_MessageList.prev = &m_MessageList;
   m_MessageList.body.root.mmsg = MaxMessages;
   m_hThread = (HANDLE)_beginthreadex(NULL,0,ThreadProc,this,0,&id);
   m_hExit   = CreateEvent(NULL,TRUE,FALSE,NULL);
  }


  CMessages::~CMessages()
  {
   if(m_hExit)
    SetEvent(m_hExit);
   if(m_hThread != (HANDLE)-1)
    WaitForSingleObject(m_hThread,INFINITE);
   if(m_hThread != (HANDLE)-1)
    CloseHandle(m_hThread);
   if(m_hExit)
    CloseHandle(m_hExit);
   Clear();
   CloseHandle(m_BannerShared);
  }

  void CMessages::clear_item(struct ts_msg *p)
  {
   if(p)
   {
    if(p->body.leaf.hWnd)
    {
     SendMessage(p->body.leaf.hWnd,WM_CLOSE,0,0);
     DestroyWindow(p->body.leaf.hWnd);
    }
    memset(p,0,sizeof(*p));
   }
  }

  int CMessages::Lock(DWORD bWait)
  {
   DWORD w = bWait?INFINITE:0,bStatus;
   bStatus = WaitForSingleObject(m_BannerShared,w);
   if(bStatus != WAIT_OBJECT_0) 
    return -1;
   return 1;
  }
  void CMessages::Unlock(void)
  {
   SetEvent(m_BannerShared);
  }

  int CMessages::ins_item(struct ts_msg*ptr)
  {
   ptr->next                = m_MessageList.prev->next;
   ptr->prev                = m_MessageList.prev;
   m_MessageList.prev->next = ptr;
   m_MessageList.prev       = ptr;
   m_MessageList.body.root.cmsg++;
   return 1;
  }
  struct ts_msg* CMessages::rm_item(struct ts_msg*ptr)
  {
   struct ts_msg* next = ptr->next;

   ptr->next->prev     = ptr->prev;
   ptr->prev->next     = ptr->next;

   if(m_MessageList.body.root.cmsg--<0)
    m_MessageList.body.root.cmsg = 0;
   return next;
  }

  static unsigned int __stdcall ThreadProc(void*lParam)
  {
   CMessages *p = (CMessages *)lParam;
   if(p)
   {
    while(1)
    {
     if(WaitForSingleObject(p->m_hExit,1000) == WAIT_OBJECT_0)
      break;
     p->Poll();
    }
   }
   return 0;
  }

  int CMessages::Clear(void)
  {
   if(Lock(0)<0) return -1;
   struct ts_msg *ptr = m_MessageList.next;
   while(ptr != &m_MessageList)
   {
    struct ts_msg *tmp = ptr;

    ptr = rm_item(ptr);
    clear_item(tmp);
    free(tmp);
   }
   Unlock();
   return 1;
  }

  int CMessages::RemoveOverlapped(struct scookie *pcookie)
  {
   struct ts_msg *ptr = m_MessageList.next;
   while(ptr != &m_MessageList)
   {
    RECT rt,rz;
    struct ts_msg *tmp = ptr;
    rt.left   = ptr->body.leaf.rect.left;
    rt.right  = ptr->body.leaf.rect.right;
    rt.top    = ptr->body.leaf.rect.top;
    rt.bottom = ptr->body.leaf.rect.bottom;
    rz.left   = pcookie->p.x;
    rz.top    = pcookie->p.y;
    rz.right  = pcookie->p.x+pcookie->sz.cx;
    rz.bottom = pcookie->p.y+pcookie->sz.cy;
    if( rz.top >= rt.top && rz.top <=rt.bottom &&
       ((rz.left>=rt.left&&rz.left<=rt.right)||(rz.right>=rt.left&&rz.right<=rt.right))
      )
    {
     
     ptr = rm_item(ptr);
     clear_item(tmp);
     free(tmp);
    }else
     ptr = ptr->next;
   }
   return 1;
  }

  int CMessages::Insert(struct MSGINFO *imsg)
  {
   char *ppp,*ptmp,*pss;
   struct ts_msg *n;
   if(!imsg) return -1;

   if(Lock(0)<0) 
   {
     message(F_DEBUG,"InsertMessage is lost\n");
     return -1;
   }
   
   if(m_MessageList.body.root.mmsg!=-1 && m_MessageList.body.root.cmsg+1>m_MessageList.body.root.mmsg)
   {
    if(m_MessageList.next != &m_MessageList)
    {
     struct ts_msg *tmp = m_MessageList.next;
     rm_item(tmp);
     clear_item(tmp);
     free(tmp);
    }
    else 
    {
     Unlock();
     return -1;
    }
   }

   if ( imsg->type != CMD_ENTER_MODULE && imsg->type != CMD_LEAVE_MODULE )
    {
     pss  = ptmp = imsg->Msg;
      do
      {
       imsg->Msg = pss;
       n = (struct ts_msg*)calloc(1,sizeof(struct ts_msg));
       if ( ppp = strstr(pss,"\\n") )
        {
         *ppp = 0;
         ppp+=2;
        }
       if(n)
       {
         n->body.leaf.born     = GetTickCount();
         n->body.leaf.ttl      = imsg->ttl;
         n->body.leaf.hWnd     = CreateMessage(imsg,n);
         ins_item(n);
       }
       imsg->Pos.y = imsg->Pos.y + imsg->Font.lfont.lfHeight;
       pss = ppp;
      }while(pss && *pss);
      imsg->Msg = ptmp;
    }else
      {
       // here code
      }
   Unlock();
   return 0;
  }

  int CMessages::Poll(void)
  {
   if(Lock(FALSE)<0) return 0;
   DWORD tick;
   struct ts_msg *ptr = m_MessageList.next,*tmp;
   while(ptr!=&m_MessageList)
   {
    tick = GetTickCount();
    long timediff = tick-ptr->body.leaf.born;
    if((timediff<0 || (DWORD)timediff>ptr->body.leaf.ttl))
    {
     tmp = rm_item(ptr);
     clear_item(ptr);
     free(ptr);
     ptr = tmp;
     continue;
    }
    ptr = ptr->next;
   }
   Unlock();
   return 0;
  }

  HWND CMessages::CreateMessage(struct MSGINFO *imsg,struct ts_msg*nn)
  {
   SIZE sz,sz1;
   HDC hDC;
   LOGFONT font;
   struct scookie *pcookie;
   DWORD style = WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE | WS_DISABLED;

   pcookie = (struct scookie*)calloc(1,sizeof(struct scookie));

   pcookie->msgln  = strlen(imsg->Msg);
   pcookie->msg    = strdup(imsg->Msg);
   pcookie->BColor = imsg->BColor;
   pcookie->FColor = imsg->FColor;
   nn->body.leaf.rect.left = pcookie->p.x    = imsg->Pos.x;
   nn->body.leaf.rect.top  = pcookie->p.y    = imsg->Pos.y;

   memcpy(&font,&imsg->Font.lfont,sizeof(font));
   font.lfPitchAndFamily = FIXED_PITCH;

   pcookie->font = CreateFontIndirect(&font);

   HWND hWnd = CreateWindowEx(WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,"STATIC","",style,
                              pcookie->p.x,pcookie->p.y,1,1,
                              imsg->hWnd,NULL,GetModuleHandle(NULL),NULL);
  if(hWnd)
  {
   hDC = GetDC(hWnd);  
   HFONT oldFont = (HFONT)SelectObject(hDC,pcookie->font);
   GetTextExtentPoint32(hDC,pcookie->msg,pcookie->msgln,&sz);
   GetTextExtentPoint32(hDC,"X",1,&sz1);
   SelectObject(hDC,oldFont);
   ReleaseDC(hWnd,hDC);
   pcookie->sz.cx = sz.cx;
   pcookie->sz.cy = sz.cy;
   nn->body.leaf.rect.right   = pcookie->p.x+pcookie->sz.cx;
   nn->body.leaf.rect.bottom  = pcookie->p.y+pcookie->sz.cy;
   RemoveOverlapped(pcookie);
   SetWindowLong(hWnd,GWL_USERDATA,(long)pcookie);
   pcookie->func = (WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC);
   SetWindowLong(hWnd,GWL_WNDPROC,(long)MyFunc);
   MoveWindow(hWnd,pcookie->p.x,pcookie->p.y,sz.cx,sz.cy,FALSE);
   ShowWindow(hWnd,SW_SHOW);
  }
  else
  {
   if(pcookie->font)
    DeleteObject(pcookie->font);
   if(pcookie->msg)
    free(pcookie->msg);
   free(pcookie);
  }
  return hWnd;
 }

 static LRESULT CALLBACK MyFunc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
 {
  PAINTSTRUCT ps;
  struct scookie *pcookie = (struct scookie*)GetWindowLong(hWnd,GWL_USERDATA);
  if(pcookie)
  {
   if(message == WM_CLOSE)
   {
    WNDPROC func = pcookie->func;
    if(pcookie->font)   DeleteObject(pcookie->font); 
    if(pcookie->msg)    free(pcookie->msg);
    free(pcookie);
    SetWindowLong(hWnd,GWL_USERDATA,(long)NULL);
    return CallWindowProc(func,hWnd,message,wParam,lParam);
   }
   if(message == WM_KILLFOCUS)
    return 0;
   if ( message == WM_ERASEBKGND )
    return 0;
   if( message == WM_PAINT )
   {
    HDC hdc = BeginPaint(hWnd, &ps);
      SetTextColor(hdc,pcookie->FColor);
      SetBkColor(hdc,pcookie->BColor);
      if(pcookie->font)
       SelectObject(hdc,pcookie->font);
      TextOut(hdc,0,0,pcookie->msg,pcookie->msgln);
	  EndPaint(hWnd, &ps);
   }else 
     return CallWindowProc(pcookie->func,hWnd,message,wParam,lParam);
  }
  return 0;
 }

