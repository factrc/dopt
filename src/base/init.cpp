

 #include "dopt.h"

 static char _id[] = "(C) CyRat,2000";

 int init_dopt_emulation(HWND wnd)
 {
  int status;
  CPP_Init();
  WN_Init();
  if(!keyboard_init())
  {
   MessageBox(wnd,keyboard_error(),"ќшибка",MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
   return 0;
  }
  message(F_DEBUG,"Keyboard Driver 1.0 Ok\n");

  status = IODevice.Open();
  if(status == 1)
  {
   message(F_DEBUG,"IO Driver 1.2 Ok\n");
  }
  if(pModuleDevice->Init()<0)
  {
   return 0;
  }
  IODevice.Signal(IO_RTS,TRUE);
  message(F_OUTPUT,"%-10s %-15s\n","Program:","gdopt.exe");
  message(F_OUTPUT,"%-10s %s, %s, %s \n","Compiled:",__DATE__,__TIME__,_id);
  message(F_OUTPUT,"ќжидание ответа от станции...");
  return status;
 }
