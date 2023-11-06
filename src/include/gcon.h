
 #define CURSOR_NORMAL  0
 #define CURSOR_HALF    3
 #define CURSOR_BLOCK   6
 #define CURSOR_ENABLE  1
 #define CURSOR_DISABLE 0

 #define BGR_TO_RGB(clr)   clr_table[(clr>>4)&0x7] 
 #define FGR_TO_RGB(clr)   clr_table[clr&0x7] 

 struct GColor
 {
  int bgrn;
  int fgrn;
 };

 struct GFont
 {
  char *name;
  int   attr;
  int   quality;
  int   pitch;
  int   charset;
  int   xsize;
  int   ysize;
  HFONT font;
  char *FontName;
 };

 class GConsole
 {
 public:
  GConsole();
 ~GConsole();
  int Init(long window_mode,int col=SCRCOL,int row=SCRROW+1);
  int Close();
  int SelectFont(char *name,int q,int a);
  int SelectFont(HFONT newFont);
  int Print(char *frm,...);
  int Update(void);
  int Clear(void);
  int CursorSize(int,long);
  int CursorVisible(int,long);
  int CursorPos(int,int c,int r);
  int OutputChr(int,int co,int ro,int clr,char c);
  int OutputChr(int,int co,int ro,int size,int clr,char c);
  int OutputString(int,int c,int r,char attr,char *str);
  int OutputLine(int,int ro,char*str,char*dat,int len);
  int OutputString(int,int c, int r,char *attr,char *str,int count);
  int FontName(char*buf,int len);
  int FontSize(int *x,int *y);
  int CalcPos(int x,int y,int *vx,int *vy);
  int CalcColor(int attr,DWORD *fcolor,DWORD *bcolor);
  int BitBlt(HDC dest,RECT &rect);
  int BitBlt(HDC dest);
  HANDLE GetUpdateEvent(void);
  HWND  GetWindowHandle(void);
  void  Flush(void);
  int   GetCursorPos(int *c,int *r);
 private:
  int CreateTerminalWindow(long window_mode,int xs,int ys,int xe,int ye);
  int ChangeSize(void);
  int CreateBackBuffer(HWND hWnd);
  int CreateUpdateBuffer(int col,int row);
  int CreateFont(void);
  int DeleteFont(void);
  int DeleteUpdateBuffer(void);
  int DeleteBackBuffer(void);
  int Lock(void);
  int Unlock(void);
  friend static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
 private:
  long m_event_delay;
  int  m_bInit,m_Count;
  CRITICAL_SECTION acl;
  HWND      m_hWnd;
  HWND      m_OwnerhWnd;
  HDC       m_backHDC;
  HBITMAP   m_backBMP;
  BitArray *m_bArray;
  GColor    m_color;
  GFont     m_fnt;
  HANDLE    m_DirtyEvent;
  int m_window_mode;
  int m_Col;
  int m_Row;
  int m_dx_offset;
  int m_dy_offset;
  int m_max_XP;
  int m_max_YP;
  int m_dx;
  int m_dy;
  int m_mod_XP;
  int m_mod_YP;
  int m_EndBuffer;
  int m_Dirty;
  int m_CursorPosX;
  int m_CursorPosY;
  int m_CursorSize;
  int m_CursorVisible;
  int m_fCursor;
  char *m_ChrArray;
  char *m_AttrArray;
  char *m_OldChrArray;
  char *m_OldAttrArray;
 };
