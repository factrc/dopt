

  struct scookie
  {
   char   *msg;
   DWORD   BColor,FColor;
   DWORD   msgln;
   POINT   p;
   HFONT   font;
   WNDPROC func;
   SIZE    sz;
  };

  struct ts_msg
  {
   struct ts_msg *prev;
   struct ts_msg *next;
   union
   {
    struct
    {
     DWORD born;
     DWORD ttl;
     HWND  hWnd;
     RECT  rect;
    }leaf;
    struct
    {
     long mmsg;
     long cmsg;
    }root;
   }body;
  };

  struct MSGINFO
  {
   HWND   hWnd;
   POINT  Pos;
   DWORD  BColor,FColor;
   DWORD  ttl;
   DWORD  type;
   struct
   {
    LOGFONT lfont;
   }Font;
   char  *Msg;
  };


  class CMessages
  {
   HANDLE           m_hExit;
   HANDLE           m_hThread;
   struct ts_msg    m_MessageList;
   HANDLE           m_BannerShared;
   int  Poll(void); //+
   int  Lock(DWORD bWait=TRUE); //+
   void Unlock(void);//+
   void           clear_item(struct ts_msg*);//+
   int            ins_item(struct ts_msg*);//+
   struct ts_msg* rm_item(struct ts_msg*ptr);//+
   HWND           CreateMessage(struct MSGINFO *,struct ts_msg*); //+
   int            RemoveOverlapped(struct scookie *pcookie);
   friend static unsigned int __stdcall ThreadProc(void*);//+
  public:
   CMessages(int MaxMessages = 0x100);   //+
  ~CMessages();                        //+
   int Insert(struct MSGINFO *);
   int Clear(void);
  };

