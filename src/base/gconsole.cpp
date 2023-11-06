//#define METHOD_NEW 1

#include <stdlib.h>
#include "dopt.h"
#include "resstr.hpp"



#define CUR_SZ(a) 2*((a)+1)
#define DELETE_OBJECT(a)         \
    if(a)                        \
    {                            \
     DeleteObject(a);            \
     a = NULL;                   \
    }
#define DELETE_DC(a)             \
    if(a)                        \
    {                            \
     DeleteDC(a);                \
     a = NULL;                   \
    }

 static int clr_table[8]=
 {
  0x000000,       // black    0
  0xFF0000,       //          1
  0x00FF00,       //          2  
  0xFFFF00,       //          3
  0x0000FF,       //          4
  0xFF00FF,       //          5
  0x00FFFF,       //          6
  0xFFFFFF        //          7
 };
 static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

// Lucida Console
 GConsole::GConsole()
 {
  m_Count        = 0;
  m_backHDC      = NULL;
  m_backBMP      = NULL;
  m_bArray       = NULL;
  m_ChrArray     = NULL;
  m_AttrArray    = NULL;
  m_OldChrArray  = NULL;
  m_OldAttrArray = NULL;
  m_color.bgrn   = BGR_TO_RGB(color_table[8]);
  m_color.fgrn   = FGR_TO_RGB(color_table[8]);
  m_fnt.font     = NULL;
  m_fnt.name     = strdup("Courier New");
  m_fnt.attr     = FW_MEDIUM;
  m_fnt.quality  = DRAFT_QUALITY;
  m_fnt.pitch    = FIXED_PITCH;
  m_fnt.charset  = OEM_CHARSET;
  m_CursorSize    = CUR_SZ(CURSOR_NORMAL);
  m_CursorVisible = CURSOR_ENABLE;
  m_CursorPosX    = 0;
  m_CursorPosY    = 0;
  m_Dirty         = 0;
  m_bInit         = FALSE;
  m_DirtyEvent    = CreateEvent(NULL,TRUE,0,NULL);
  m_window_mode   = 0;
  InitializeCriticalSection(&acl);
 }

 GConsole::~GConsole()
 {
  _ASSERT(m_backHDC == NULL);
  _ASSERT(m_backBMP == NULL);
  _ASSERT(m_fnt.font == NULL);
  CloseHandle(m_DirtyEvent);
  DeleteCriticalSection(&acl);
 }

 int GConsole::Close(void)
 {
  Lock();
  DeleteFont();
  DeleteBackBuffer();
  DeleteUpdateBuffer();


  m_bInit = FALSE;
  Unlock();
  return 0;
 }

 HANDLE GConsole::GetUpdateEvent(void)
 {
  return m_DirtyEvent;
 }
 int GConsole::Lock(void)
 {
#if 0
  EnterCriticalSection(&acl);
#endif
  return 0;
 }
 int GConsole::Unlock(void)
 {
#if 0
  LeaveCriticalSection(&acl);
#endif
  return 0;
 }

 int GConsole::CreateBackBuffer(HWND hWnd)
 {
   HDC     base_hdc = GetDC(hWnd);
   m_backHDC = CreateCompatibleDC(base_hdc);
   m_backBMP = CreateCompatibleBitmap(base_hdc,m_max_XP,m_max_YP);
   if(!m_backHDC||!m_backBMP)
   {
    DELETE_OBJECT(m_backBMP);
    DELETE_DC(m_backHDC);
    ReleaseDC(hWnd,base_hdc);
    return 0;
   }
   ReleaseDC(hWnd,base_hdc);
   SelectObject(m_backHDC,m_backBMP);
   SelectObject(m_backHDC,CreateSolidBrush(m_color.bgrn));
   SelectObject(m_backHDC,CreatePen(0,0,m_color.bgrn));
   Rectangle(m_backHDC,m_dx_offset,m_dy_offset,m_max_XP,m_max_YP);
   return 1;
 }

 int GConsole::CreateUpdateBuffer(int col,int row)
 {
  long size = col*row;
  int  sym  = (color_table[8]<<8)|' ';
  m_bArray      = new BitArray(size);
  m_ChrArray    = (char *)calloc(sizeof(char),size);
  m_AttrArray   = (char *)calloc(sizeof(char),size);
  m_OldChrArray  = (char *)calloc(sizeof(char),size);
  m_OldAttrArray = (char *)calloc(sizeof(char),size);
  if(!m_bArray || !m_ChrArray || !m_AttrArray)
  {
   if(m_bArray) delete m_bArray;
   if(m_ChrArray) free(m_ChrArray);
   if(m_AttrArray) free(m_AttrArray);
   if(m_OldChrArray) free(m_OldChrArray);
   if(m_OldAttrArray) free(m_OldAttrArray);
   m_ChrArray     = NULL;
   m_AttrArray    = NULL;
   m_OldChrArray  = NULL;
   m_OldAttrArray = NULL;
   return 0;
  }
  for(int i = 0;i<size;i++)
  {
   m_ChrArray[i]  = ' ';
   m_AttrArray[i] = ' ';
   m_OldChrArray[i]  = 0;
   m_OldAttrArray[i] = -1;
  }
  m_bArray->setting();
  return 1;
 }

 static LOGFONT lfnt;

 int GConsole::CreateFont(void)
 {
  SIZE l_sz;
  int j=0;
  int inc=1;
  char buffer[256];

  if(!m_fnt.font)
  {
   for(int i=0;i<100;i++)
   {

     memset(&lfnt,0,sizeof(lfnt));
     lfnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
     strcpy(lfnt.lfFaceName,m_fnt.name?m_fnt.name:"Courier New");
     lfnt.lfCharSet = m_fnt.charset;
     lfnt.lfEscapement = 0;
     lfnt.lfOrientation = 0;
     lfnt.lfQuality     = m_fnt.quality;
     lfnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
     lfnt.lfPitchAndFamily = m_fnt.pitch;
     lfnt.lfHeight = m_dy;
     lfnt.lfWidth  = m_dx+j;
     lfnt.lfWeight = m_fnt.attr;
     m_fnt.font = CreateFontIndirect(&lfnt);
     SelectObject(m_backHDC,m_fnt.font);
     GetTextExtentPoint32(m_backHDC,"X",1,&l_sz);
     if(l_sz.cx<m_dx||l_sz.cx>m_dx+4)
     {
      inc = (l_sz.cx<m_dx)?1:-1;
      DELETE_OBJECT(m_fnt.font);
     }
     j+=inc;
     if(m_fnt.font) break;
   }
   if(!m_fnt.font) return 0;
  }else
   SelectObject(m_backHDC,m_fnt.font);

  GetTextExtentPoint32(m_backHDC,"X",1,&l_sz);
  m_fnt.xsize    = l_sz.cx;
  m_fnt.ysize    = l_sz.cy;
  if(GetTextFace(m_backHDC,sizeof(buffer)-1,NULL)<sizeof(buffer))
  {
   GetTextFace(m_backHDC,sizeof(buffer)-1,buffer);
   message(F_DEBUG,"Selected font name: %s\n",buffer);
   if ( m_fnt.FontName )
    free( m_fnt.FontName );
   m_fnt.FontName = strdup(buffer);
  }
  SetTextAlign(m_backHDC,TA_NOUPDATECP);
  return 1;
 }

 int GConsole::DeleteFont(void)
 {
  DELETE_OBJECT(m_fnt.font);
  return 1;
 }


 int GConsole::DeleteUpdateBuffer(void)
 { 
   if(m_bArray) 
    delete m_bArray;
   if(m_ChrArray)
    free(m_ChrArray);
   if(m_AttrArray)
    free(m_AttrArray);
   if(m_OldChrArray)
    free(m_OldChrArray);
   if(m_OldAttrArray)
    free(m_OldAttrArray);
   m_bArray       = NULL;
   m_ChrArray     = NULL;
   m_AttrArray    = NULL;
   m_OldChrArray  = NULL;
   m_OldAttrArray = NULL;
   return 1;
 }

 int GConsole::DeleteBackBuffer(void)
 {
  DELETE_OBJECT(m_backBMP)
  DELETE_DC(m_backHDC);
  return 1;
 }

 int GConsole::Init(long window_mode,int col,int row)
 {
  RECT sz;
  RECT client_size;
  if(col <= 0)  col = SCRCOL;
  if(row <= 0)  row = SCRROW;
  Lock();
  m_event_delay = get_option("WINDOW_event_delay")->i;
  sz.left   = get_option("WINDOW_x")->i;
  sz.top    = get_option("WINDOW_y")->i;
  sz.right  = get_option("WINDOW_width")->i;
  sz.bottom = get_option("WINDOW_height")->i;
  if(!m_hWnd  && !CreateTerminalWindow(window_mode,sz.left,sz.top,sz.right,sz.bottom))
  {
   Unlock();
   return 0;
  }
  GetClientRect(m_OwnerhWnd,&client_size);
  m_Col       = col;
  m_Row       = row;
  m_dx_offset = 0;
  m_dy_offset = 0;
  m_max_XP    = client_size.right;
  m_max_YP    = client_size.bottom;
  m_dx        = m_max_XP/col;
  m_dy        = m_max_YP/row;
  m_mod_XP    = m_dx*col;
  m_mod_YP    = m_dy*row;

  if(m_mod_XP!=m_max_XP)
   m_dx_offset = (m_max_XP-m_mod_XP)/2;

  if(!CreateUpdateBuffer(col,row))
  {
   Unlock();
   return 0;
  }
  if(!CreateBackBuffer(m_hWnd))
  {
   DeleteUpdateBuffer();
   Unlock();
   return 0;
  }
  if(!CreateFont())
  {
   DeleteBackBuffer();
   DeleteUpdateBuffer();
   Unlock();
   return 0;
  }
  CreateCaret(m_hWnd,NULL,m_dx,m_CursorSize);
  if(m_CursorVisible == CURSOR_DISABLE)
      HideCaret(m_hWnd);
  else
      ShowCaret(m_hWnd);

  m_EndBuffer = m_Col*m_Row;
  m_bInit = TRUE;
  Unlock();
  Clear();
  return 1;
 }

 int GConsole::FontName(char *buf,int len)
 {
  Lock();
  if(!buf)
  {
   int len = (m_fnt.FontName)?strlen(m_fnt.FontName):0;
   Unlock();
   return len;
  }
  if(m_fnt.FontName)
  {
   strncpy(buf,m_fnt.FontName,len-1);
   buf[len-1] = 0;
  }
  Unlock();
  return len;
 }
 int GConsole::FontSize(int *x,int *y)
 {
  Lock();
  *x = m_fnt.xsize;
  *y = m_fnt.ysize;
  Unlock();
  return 0;
 }
 int GConsole::CursorSize(int update,long nSize)
 {
  Lock();
  if(m_bInit)
  {
   m_CursorSize = CUR_SZ(nSize);
   if(m_CursorSize>m_dy) m_CursorSize = m_dy;
   if( m_fCursor++ < 1 && update)
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 1;
 }
 int GConsole::CursorVisible(int update,long stat)
 {
  Lock();
  if( m_bInit)
   {
    m_CursorVisible = stat;
    if( m_fCursor++ < 1 && update) SetEvent(m_DirtyEvent);
   }
  Unlock();
  return 1;
 }
 int GConsole::GetCursorPos(int *c,int *r)
 {
  Lock();
  *c = m_CursorPosX;
  *r = m_CursorPosY;
  Unlock();
  return 0;
 }
 int GConsole::CursorPos(int update,int c,int r)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(c>=0);
   _ASSERT(r>=0);
   _ASSERT(c<m_Col);
   _ASSERT(r<m_Row);
   m_CursorPosX = c;
   m_CursorPosY = r;
   if( m_fCursor++ < 1 && update)  SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 1;
 }

 int GConsole::SelectFont(char *name,int q,int a)
 {
  Lock();
  if ( m_fnt.name )
   free(m_fnt.name);
  m_fnt.name    = strdup(name);
  m_fnt.charset = OEM_CHARSET;
  m_fnt.quality = q;
  m_fnt.pitch   = FIXED_PITCH;
  m_fnt.attr    = a;
  Unlock();
  return 1;
 }

 int GConsole::SelectFont(HFONT newFont)
 {
  if(newFont)
  {
   GFont backup;
   memcpy(&backup,&m_fnt,sizeof(m_fnt));
   Close();
   memcpy(&m_fnt,&backup,sizeof(backup));
   m_fnt.font = newFont;
   m_fnt.name = NULL;
   if(!Init(m_window_mode,m_Col,m_Row))
   {
    DELETE_OBJECT(m_fnt.font);
    m_fnt.font = NULL;
    Init(m_window_mode,m_Col,m_Row);
   }
  }
  return 0;
 }

 int GConsole::Print(char *frm,...)
 {
  va_list arg;
  char buffer[1024],*ptr = buffer;

  va_start(arg,frm);
  vsprintf(buffer,frm,arg);
  va_end(arg);
  Lock();
  if(m_bInit)
  {
   long offset = m_CursorPosX+m_CursorPosY*m_Col;
   CharToOem(buffer,buffer);
   for(;*ptr;ptr++)
   {
    if(*ptr == '\n')
    {
     m_CursorPosX = 0;
     m_CursorPosY++;
     offset = m_CursorPosY*m_Col;
     if(offset>=m_EndBuffer) offset = m_EndBuffer-m_Col;
     if(m_CursorPosY>m_Row) m_CursorPosY--;
    }else
    {
//TRY
     m_bArray->set(offset);
     m_ChrArray[offset]  = *ptr;
     m_AttrArray[offset] = color_table[8];
     if(offset>=m_EndBuffer) offset = m_EndBuffer-1;
     else
     {
      m_CursorPosX++;
      if(m_CursorPosX>m_Col)
      {
       m_CursorPosX = 0;
       m_CursorPosY++;
       if(m_CursorPosY>m_Row) m_CursorPosY--;
      }
      offset++;
     }
    }
   }
   if ( m_Dirty++ < 1  )
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 1;
 }

 int GConsole::BitBlt(HDC dest,RECT &rect)
 {
  int ret=0;
  Lock();
  if(m_bInit)
  {
   ret = (int) ::BitBlt(
                dest,
                rect.left,rect.top,
                rect.right-rect.left,
                rect.bottom-rect.top,
                m_backHDC,
                rect.left,rect.top,SRCCOPY
               );
  }
  Unlock();
  return ret;
 }
 int GConsole::BitBlt(HDC dest)
 {
  int ret;
  Lock();
  if(!m_bInit)
   ret = 0;
  else
   ret = ( int ) ::BitBlt(dest,0,0,m_max_XP,m_max_YP,m_backHDC,0,0,SRCCOPY);
  Unlock();
  return ret;
 }
 int GConsole::Clear(void)
 {
  Lock();
  if(m_bInit)
  {
   long size = m_Col*m_Row;
//TRY
   m_bArray->setting();
   for(int i = 0;i<size;i++)
   {
    m_ChrArray[i]  = ' ';
    m_AttrArray[i] = color_table[8];
   }
   if ( m_Dirty++ < 1)
    SetEvent(m_DirtyEvent);
  }
  m_CursorPosX = 0;
  m_CursorPosY = 0;
  Unlock();
  return 0;
 }
 int GConsole::OutputChr(int update,int co,int ro,int clr,char c)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(co>=0);
   _ASSERT(ro>=0);
   _ASSERT(co<m_Col);
   _ASSERT(ro<m_Row);
   long offset = co+m_Col*ro;
   m_ChrArray[offset]  = c;
   m_AttrArray[offset] = clr;
   m_bArray->set(offset);
   if( m_Dirty++ < 1 && update )
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 0;
 }
 int GConsole::CalcPos(int x,int y,int *vx,int *vy)
 {
  Lock();
  long yoffset = m_dy_offset+(y*m_dy);
  long xoffset = m_dx_offset+(x*m_dx);
  *vy = yoffset;
  *vx = xoffset;
  Unlock();
  return 0;
 }
 int GConsole::OutputChr(int update,int co,int ro,int size,int clr,char c)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(co>=0);
   _ASSERT(ro>=0);
   _ASSERT(co<m_Col);
   _ASSERT(ro<m_Row);
   long offset = co+m_Col*ro;
   char *pChr,*pAttr;
   pChr  = &m_ChrArray[offset];
   pAttr = &m_AttrArray[offset];
   _ASSERT(offset+size<=m_EndBuffer);
   for(int i = 0;i<size;i++)
   {
    *pChr++  = c;
    *pAttr++ = clr;
   }
   m_bArray->set(offset,size);

   if( m_Dirty++ < 1 && update)
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 0;
 }

 int GConsole::OutputString(int update,int c,int r,char attr,char *str)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(c>=0);
   _ASSERT(r>=0);
   _ASSERT(c<m_Col);
   _ASSERT(r<m_Row);
   long len = strlen(str);
   long offset = c+m_Col*r;
   char *pChr,*pAttr;
   _ASSERT((int)(offset+len)<=(int)m_EndBuffer);

   pChr  = &m_ChrArray[offset];
   pAttr = &m_AttrArray[offset];

   for( int i = 0; i<len ; i++)
   {
    *pChr++  = *str++;
    *pAttr++ = attr;
   }
   m_bArray->set(offset,len);

   if( m_Dirty++ < 1 && update )
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 0;
 }
 int GConsole::OutputString(int update,int c, int r,char *attr,char *str,int count)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(c>=0);
   _ASSERT(r>=0);
   _ASSERT(c<m_Col);
   _ASSERT(r<m_Row);
   long offset = c+m_Col*r;
   char *pChr,*pAttr;
   pChr  = &m_ChrArray[offset];
   pAttr = &m_AttrArray[offset];
   _ASSERT(offset+count<=m_EndBuffer);
   for(int i = 0;i<count;i++)
   {
    *pChr++  = *str++;
    *pAttr++ = *attr++;
   }
   m_bArray->set(offset,count);

   if( m_Dirty++ < 1 && update)
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 0;
 }
 int GConsole::OutputLine(int update,int ro,char*str,char*dat,int len)
 {
  Lock();
  if(m_bInit)
  {
   _ASSERT(ro>=0);
   _ASSERT(ro<m_Row);
   long offset = m_Col*ro,psize;
   char *pChr,*pAttr;
   pChr  = &m_ChrArray[offset];
   pAttr = &m_AttrArray[offset];
   _ASSERT(offset+len*m_Col<=m_EndBuffer);
   psize = offset;
   for(int j = 0;j<len;j++)
   {
    for(int i = 0;i<m_Col;i++)
    {
     *pChr++  = *str++;
     *pAttr++ = *dat++;
     psize++;
    }
   }
   m_bArray->set(offset,psize-offset);
   if( m_Dirty++ < 1 && update)
    SetEvent(m_DirtyEvent);
  }
  Unlock();
  return 0;
 }
 int GConsole::ChangeSize(void)
 {
  Close();
  return Init(m_window_mode,m_Col,m_Row);
 }

 void GConsole::Flush(void)
 {
  Lock();
  if( m_Dirty++ < 1 )
   SetEvent(m_DirtyEvent);
  Unlock();
 }
 int GConsole::CalcColor(int attr,DWORD *fcolor,DWORD *bcolor)
 {
  Lock();
   *fcolor = FGR_TO_RGB(attr);
   *bcolor = BGR_TO_RGB(attr);
  Unlock();
  return 1;
 }

 HWND GConsole::GetWindowHandle(void)
 {
  Lock();
  if(m_bInit)
  {
   Unlock();
   return m_hWnd;
  }
  Unlock();
  return NULL;
 }

 int GConsole::CreateTerminalWindow(long window_mode,int xs,int ys,int xe,int ye)
 {
  WNDCLASSEX wcex;
  char window_class[255];
  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
  
  sprintf(window_class,"Dopt Terminal(Graphics)");//,(void*)this);

  wcex.cbSize          = sizeof(WNDCLASSEX);
  wcex.style           = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc     = (WNDPROC)WndProc;
  wcex.cbClsExtra      = 0;
  wcex.cbWndExtra      = sizeof(this);
  wcex.hInstance       = hInstance;
  wcex.hIcon           = NULL;
  wcex.hCursor         = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground   = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wcex.lpszMenuName    = NULL;
  wcex.lpszClassName   = window_class;
  wcex.hIconSm         = NULL;

  if(RegisterClassEx(&wcex)!=NULL)
  {
   cResStr termName(IDS_DOPTWINDOW);
   if(!window_mode)
   {
    RECT rect;
    GetWindowRect((m_OwnerhWnd = GetDesktopWindow()),&rect);
    m_hWnd = CreateWindow(window_class,termName.GetString(),WS_POPUP,0, 0,rect.right,rect.bottom,NULL,NULL,hInstance,NULL);
   }else{
    m_hWnd = CreateWindow(window_class,termName.GetString(),WS_POPUP|WS_THICKFRAME|WS_CAPTION,
                         xs,ys,xe,ye,NULL,NULL,hInstance,NULL);
    m_OwnerhWnd = m_hWnd;
   }
   if(m_hWnd)
   {
    SetWindowLong(m_hWnd,sizeof(this)-4,(LONG)this);
    ShowWindow(m_hWnd,SW_SHOW);
    UpdateWindow(m_hWnd);
   }
   m_window_mode = window_mode;
  }
  return (long)m_hWnd;
 }

 LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
 {
  RECT rect;
  GConsole *p = (GConsole *)GetWindowLong(hWnd,sizeof(GConsole*)-4);
  if(p)
  {
   switch (message)
   {
     case WM_CREATE:
     break;
     case WM_EXITSIZEMOVE:
      if(p->m_window_mode)
      {
       p->ChangeSize();
       WN_Redraw_All();
      }
     break;
     case WM_KEYUP:
     case WM_SYSKEYUP:
      keyboard_input((lParam>>16)&0xFF,(lParam>>24)&1,lParam&0xFFFF,0);
     break;
     case WM_SETFOCUS:  
      keyboard_clear_flags();
     break;
     case WM_ERASEBKGND:
     break;
     case WM_KEYDOWN:
     case WM_SYSKEYDOWN:
      {
       DWORD res = keyboard_input((lParam>>16)&0xFF,(lParam>>24)&1,lParam&0xFFFF,1);
       if ( res == 0x1 )
        {
         cResStr termExit(IDS_DOPTEXIT);
         DWORD code = MessageBox(hWnd,termExit.GetString(),"Сообщение",MB_YESNO|MB_SYSTEMMODAL|MB_ICONQUESTION);
         if( code == IDYES)
          PostQuitMessage(1);
        }
      }
     break;
     case WM_PAINT:
      if(GetUpdateRect(hWnd,&rect,FALSE))
      {
       PAINTSTRUCT ps;
       HDC hdc = BeginPaint(hWnd, &ps);
       p->BitBlt(hdc,rect);
       EndPaint(hWnd, &ps);
      }
     break;
     case WM_CLOSE:
      p->Clear();
      PostQuitMessage(0);
     break;
     default:
     return DefWindowProc(hWnd, message, wParam, lParam);
   }
  }else
   return DefWindowProc(hWnd, message, wParam, lParam);
  return 0;
 }


  int GConsole::Update(void)
  {
   RECT rect;
   int linestart,lineend;

   if ( !m_bInit )
    return 0;

   Lock();

   if( m_fCursor )
    {
     static int old_cursor = -1;
     if(m_CursorVisible)
      {
       static long oldsz = -1;
       if(oldsz != m_CursorSize)
       {
        CreateCaret(m_hWnd,NULL,m_dx,m_CursorSize);
        oldsz = m_CursorSize;
       }
      SetCaretPos(m_dx_offset+m_CursorPosX*m_dx,m_dy_offset+m_CursorPosY*m_dy-m_CursorSize+m_dy);
      if ( old_cursor != m_CursorVisible )
       {
        ShowCaret(m_hWnd);
        old_cursor = m_CursorVisible;
       }
     }else
       {
        if ( old_cursor != m_CursorVisible )
         {
          HideCaret(m_hWnd);
          old_cursor = m_CursorVisible;
         }
       }
     m_fCursor = 0;
    }

   if( !m_Dirty )
    {
      ResetEvent(m_DirtyEvent);
      Unlock();
     return 1;
    }

   switch(m_Count)
    {
#ifndef METHOD_NEW
    case 0:
     linestart   = 0;
     lineend     = SCRROW+1;
     rect.left   = 0;
     rect.right  = m_max_XP;
     rect.top    = m_dy_offset;//+linestart*m_dy;
     rect.bottom = m_max_YP;
     m_Count     = 0;
    break;
#else
    case 0:
     linestart   = 0;
     lineend     = 4;
     rect.left   = 0;
     rect.right  = m_max_XP;
     rect.top    = 0;
     rect.bottom = m_dy_offset+lineend*m_dy;
     m_Count     = 1;
    break;
    case 1:
     linestart   = 4;
     lineend     = 13;
     rect.left   = 0;
     rect.right  = m_max_XP;
     rect.top    = m_dy_offset+linestart*m_dy;
     rect.bottom = m_dy_offset+(lineend+1)*m_dy;
     m_Count     = 2;
    break;
    case 2:
     linestart   = 13;
     lineend     = (SCRROW-4);
     rect.left   = 0;
     rect.right  = m_max_XP;
     rect.top    = m_dy_offset+linestart*m_dy;
     rect.bottom = m_dy_offset+(lineend+1)*m_dy;
     m_Count     = 3;
    break;
    case 3:
     linestart   = (SCRROW-4);
     lineend     = SCRROW+1;
     rect.left   = 0;
     rect.right  = m_max_XP;
     rect.top    = m_dy_offset+linestart*m_dy;
     rect.bottom = m_max_YP;
     m_Count     = 0;
    break;
 #endif
   }

  long xoffset,
       bClear  = 0,
       olattr  = -1,
       yoffset = m_dy_offset+linestart*m_dy,
       offset  = linestart*m_Col;
        
  char *pArrChr   = &m_ChrArray[offset];
  char *pArrAttr  = &m_AttrArray[offset];
  char *pArrOChr  = &m_OldChrArray[offset];
  char *pArrOAttr = &m_OldAttrArray[offset];

  for ( ; linestart < lineend ; linestart++ )
    {
     xoffset = m_dx_offset;
     for ( int i = m_Col ; --i >= 0 ; )
      {
       char chr,attr,ochr,oattr;
       if ( !m_bArray->get(offset) )
        goto next_symbol_check;
       chr   = *pArrChr;
       attr  = *pArrAttr;
       ochr  = *pArrOChr;
       oattr = *pArrOAttr;
       if ( chr != ochr || attr != oattr )
        {
          *pArrOChr  = chr;
          *pArrOAttr = attr;
          if ( olattr != attr )
           {
            olattr = attr;
            SetBkColor(m_backHDC,BGR_TO_RGB(attr));
            SetTextColor(m_backHDC,FGR_TO_RGB(attr));
           }
         bClear = TextOut(m_backHDC,xoffset,yoffset,(char*)&chr,1);
        }
       m_bArray->unset(offset);

next_symbol_check:

       offset++;
       xoffset+=m_dx;
       pArrChr++;
       pArrAttr++;
       pArrOChr++;
       pArrOAttr++;
      } 
     yoffset+=m_dy;
  }
   if ( bClear )
    InvalidateRect(m_hWnd,&rect,FALSE);

   if (m_Count)
    {
     Unlock();
     return 0;
    }
   m_Dirty = 0;
   ResetEvent(m_DirtyEvent);
   Unlock();
  return 1;
 }






/*
 int GConsole::Update(void)
 {
  DWORD bClear = FALSE;
  RECT rect;
  int linestart,lineend;
  Lock();
  if(!m_bInit)
  {
   Unlock();
   return 0;
  }
  if(m_fCursor)
  {
   static int old_cursor = -1;
   if(m_CursorVisible)
   {
    static long oldsz = -1;
    if(oldsz!=m_CursorSize)
    {
     CreateCaret(m_hWnd,NULL,m_dx,m_CursorSize);
     oldsz = m_CursorSize;
    }
    SetCaretPos(m_dx_offset+m_CursorPosX*m_dx,m_dy_offset+m_CursorPosY*m_dy-m_CursorSize+m_dy);
   }
   if(old_cursor!=m_CursorVisible)
   {
    if(m_CursorVisible)
     ShowCaret(m_hWnd);
    else
     HideCaret(m_hWnd);
    old_cursor = m_CursorVisible;
   }
   m_fCursor = FALSE;
  }
  if(m_Dirty)
  {
   switch(m_Count)
   {
#ifdef METHOD_NEW
   case 0:
    linestart   = 0;
    lineend     = 4;
    rect.left   = 0;
    rect.right  = m_max_XP;
    rect.top    = 0;
    rect.bottom = m_dy_offset+lineend*m_dy;
    m_Count     = 1;
   break;
#else
   case 0:
    linestart   = 0;
    lineend     = SCRROW+1;
    rect.left   = 0;
    rect.right  = m_max_XP;
    rect.top    = m_dy_offset+linestart*m_dy;
    rect.bottom = m_max_YP;
    m_Count     = 0;
   break;
#endif
   case 1:
    linestart   = 4;
    lineend     = 13;
    rect.left   = 0;
    rect.right  = m_max_XP;
    rect.top    = m_dy_offset+linestart*m_dy;
    rect.bottom = m_dy_offset+(lineend+1)*m_dy;
    m_Count     = 2;
   break;
   case 2:
    linestart   = 13;
    lineend     = (SCRROW-4);
    rect.left   = 0;
    rect.right  = m_max_XP;
    rect.top    = m_dy_offset+linestart*m_dy;
    rect.bottom = m_dy_offset+(lineend+1)*m_dy;
    m_Count     = 3;
   break;
   case 3:
    linestart   = (SCRROW-4);
    lineend     = SCRROW+1;
    rect.left   = 0;
    rect.right  = m_max_XP;
    rect.top    = m_dy_offset+linestart*m_dy;
    rect.bottom = m_max_YP;
    m_Count     = 0;
   break;
   }
   long olattr  = -1;
   long offset  = linestart*m_Col;
   long yoffset = m_dy_offset+linestart*m_dy;
   long xoffset;
   for(int j = linestart;j<lineend;j++)
   {
    xoffset = m_dx_offset;
    for(int i = 0;i<m_Col;i++)
    {
     if(m_bArray->get(offset))
     {
      long chr,attr,ochr,oattr;
      chr   = (long)m_ChrArray[offset];
      attr  = (long)m_AttrArray[offset];
      ochr  = m_OldChrArray[offset];
      oattr = m_OldAttrArray[offset];
      if(ochr!=chr || oattr!=attr)
      {
       m_OldChrArray[offset]  = (char)chr;
       m_OldAttrArray[offset] = (char)attr;
       if(olattr != attr)
       {
        olattr = attr;
        SetBkColor(m_backHDC,BGR_TO_RGB(attr));
        SetTextColor(m_backHDC,FGR_TO_RGB(attr));
       }
       TextOut(m_backHDC,xoffset,yoffset,(char*)&chr,1);
       bClear = TRUE;
      }
      m_bArray->unset(offset);
     }
     offset++;
     xoffset+=m_dx;
    }
    yoffset+=m_dy;
   }
   if(bClear == TRUE)
   {
    InvalidateRect(m_hWnd,&rect,FALSE);
   }
   if(m_Count)
   {
    Unlock();
    return 0;
   }
   m_Dirty = 0;
  }
  ResetEvent(m_DirtyEvent);
  Unlock();
  return 1;
 }
*/

