
#include "dopt.h"
#include <time.h>

 static void debug(unsigned char*buf,int len);
 static void trace(int flag,unsigned char*buf,int len);

 static DWORD bDebug,bTrace;

 void InitDebugSection(void)
 {
  bDebug = get_option("FLAG_debug")->i;
  bTrace = get_option("COM_trace")->i;
 }

 char *trim(char*str)
 {
  char *start = str;
  int size = strlen(str);
  while(*str&&*str==' ') str++;
  memmove(start,str,size+1);
  str =(char*)((long)start+(long)start+(long)size-(long)str);
  while(str>=start&&(*--str==' '))*str=0;
  return start;
 }

 int message(int flag,char*frm,...)
 {
  char buffer[1024];
  if((flag&F_OUTPUT))
  {
   va_list arg;
   va_start(arg,frm);
   vsprintf(buffer,frm,arg);
   va_end(arg);
   ScreenDevice.Print(buffer);
   while(ScreenDevice.Update()!=1);
  }
  if((flag&F_DEBUG)&& bDebug == TRUE)
  {
   va_list arg;
   va_start(arg,frm);
   vsprintf(buffer,frm,arg);
   va_end(arg);
   debug((unsigned char*)buffer,strlen(buffer));
  } 
  return 1;
 }
 int message_trace(int flag,unsigned char*buf,int len)
 {
  if(bTrace == TRUE)
  {
   trace(flag,(unsigned char*)buf,len);
  }
  return 0;
 }
 static void debug(unsigned char*buf,int len)
 {
  char *fname = get_option("DEBUG_options")->s;
  char *times;
  int i;
  if(!fname)
  {
   return;
  }
  if(!strcmp(fname,"WINDOW"))
  {
   ;
  }else
  {
   FILE *fin = fopen(fname,"at");
   if(!fin)
   {
    return;
   }
   time_t tms;
   time(&tms);
   times = ctime(&tms);
   for(i=0;times[i] && times[i]!='\n';i++) i++;
   if(times[i])times[i]=' ';
   fwrite(times,1,strlen(times),fin);
   fwrite(buf,1,len,fin);
   fclose(fin);
  }
 }
 static void trace(int flag,unsigned char*buf,int len)
 {
  char *fname;
  FILE *fd;
  switch(flag)
  {
   case TRACE_WRITE:
    fname = get_option("COM_trace_output")->s;
   break;
   case TRACE_READ:
    fname = get_option("COM_trace_input")->s;
   break;
   default:
   return;
  }
  if(!fname || !(fd = fopen(fname,"ab"))) return;
  fwrite(buf,1,len,fd);
  fclose(fd);
 }


/*
   char *opt = get_option("DEBUG_option")->s;
   if(opt)
   {
    trim(opt);
    if(!strncmp(opt,"file",4))
    {
     debug(opt+4,);
    }
   }

   char *opt = get_option("TRACE_option")->s;
   c_trace.send(FNAME_TRACE,);
*/
/*
-----------------------------------------------------
Debug Function
-----------------------------------------------------
*/
/*
static HWND debugWnd;
static WNDPROC def_debugProc;

#pragma warning (disable: 4065)
static long CALLBACK DebugChildProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
 switch(message)
 {
  default:
   return CallWindowProc((WNDPROC)def_debugProc,hWnd,message,wParam,lParam);
 }
 return 0;
}
#pragma warning (default: 4065)

 static long CALLBACK DebugParentWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
 {
  switch(message)
  {
   case WM_SIZE:
   MoveWindow(debugWnd,0,0,LOWORD(lParam),HIWORD(lParam),TRUE);
   break;
   case WM_CLOSE:
   case WM_DESTROY:
   break;
   default:
   return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
 }
#include <windowsx.h>
 static int  message_DebugWindow(unsigned char*str,int size_str)
 {
  if(debugWnd)
  {
   if(size_str>10*KB) size_str = 10*KB;
   int size = Edit_GetTextLength(debugWnd);
   if(size+size_str>10*KB)
   {
    Edit_SetSel(debugWnd,0,size_str);
    Edit_ReplaceSel(debugWnd,"");
   }
   str[size_str]=0;
   Edit_SetSel(debugWnd,(WPARAM)-1,(LPARAM)-1);
   Edit_ReplaceSel(debugWnd,str);
  }
  return debugWnd!=NULL;
 }

 int CreateDebugWnd(HWND hWnd,HINSTANCE g_hInstance)
 {
  HWND wnd;
  WNDCLASSEX wcex;
  RECT rect;
  char *class_str_debug = "DEBUGDOPT";
 
  wcex.cbSize            = sizeof(WNDCLASSEX);
  wcex.style             = CS_HREDRAW | CS_VREDRAW|CS_OWNDC;
  wcex.lpfnWndProc       = (WNDPROC)DebugParentWndProc;
  wcex.cbClsExtra        = 
  wcex.cbWndExtra        = 0;
  wcex.hInstance         = g_hInstance;
  wcex.hIcon             = NULL;
  wcex.hCursor           = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground     = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName      = NULL;
  wcex.lpszClassName     = class_str_debug;
  wcex.hIconSm           = NULL;

  debugWnd=NULL;

  if(!RegisterClassEx(&wcex)) return 0;
  HWND primary = GetDesktopWindow();
  RECT deb_size;
  GetClientRect(primary,&deb_size);
  wnd =  CreateWindow(class_str_debug,"Debug Window",
                      WS_VISIBLE|WS_POPUP|WS_THICKFRAME|WS_CAPTION,
                      0,deb_size.bottom/2,deb_size.right,deb_size.bottom/2,
                      NULL, NULL,g_hInstance,NULL);
  if(!wnd){
   UnregisterClass(class_str_debug,g_hInstance);
   return 0;
  }

  debugWnd = CreateWindow("RICHEDIT","",
                          WS_VISIBLE|WS_CHILD|ES_LEFT|ES_MULTILINE|
                          ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL,
                          0,0,0,0,
                          wnd, NULL,g_hInstance,NULL);
  if(!debugWnd){
   UnregisterClass(class_str_debug,g_hInstance);
   return 0;
  }
  def_debugProc = (WNDPROC)::GetWindowLong(debugWnd,GWL_WNDPROC);
  ::SetWindowLong(debugWnd,GWL_WNDPROC,(long)DebugChildProc);
  ::GetClientRect(wnd,&rect);
  ::MoveWindow(debugWnd,0,0,rect.right,rect.bottom,TRUE);
  return (debugWnd)?1:0;
 }
#undef KB
*/
/*
 long sys_debug(int level,char *frm,...)
 {
  char buffer[4096];
  va_list arg;
  va_start(arg,frm);
  vsprintf(buffer,frm,arg);
  va_end(arg);
  debug((unsigned char*)buffer,strlen(buffer));
  return 0;
 }
*/
//#undef O_STR
//#undef O_INT
//#undef O_REAL
//#undef O_FLAG
