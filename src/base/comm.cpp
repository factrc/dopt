
 #include <process.h>
 #include "dopt.h"

 extern DWORD  __EventTime;
 extern long   __EventCode;


 Comm::Comm()
 {
  memset(&m_rOver,0,sizeof(m_rOver));
  memset(&m_wOver,0,sizeof(m_wOver));
  memset(&m_Event,0,sizeof(m_Event));
  m_rOver.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
  m_wOver.hEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
  m_Event.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
  m_hTimer = CreateEvent(NULL,TRUE,FALSE,NULL);
  m_hdev  = INVALID_HANDLE_VALUE;
  m_fFile = FALSE;
  m_fOpen = FALSE;
  m_WriteStatus = 0;
  m_WriteBytes  = 0;
  m_WriteLow    = 0;
  m_WriteHigh   = 0;
  m_WriteMax    = 10000;
  m_WriteBuffer = (unsigned char*)calloc(1,m_WriteMax+1);
  m_recvSize    = 0;
 }

 Comm::~Comm()
 {
  Close();
  if(m_rOver.hEvent)
   CloseHandle(m_rOver.hEvent);
  if(m_wOver.hEvent)
   CloseHandle(m_wOver.hEvent);
  if(m_Event.hEvent)
   CloseHandle(m_Event.hEvent);
  if(m_hTimer)
   CloseHandle(m_hTimer);
  if(m_WriteBuffer)
   free(m_WriteBuffer);
 }

 long Comm::Open(void)
 {
  char *n;
  DCB  dcb;
  if(m_fOpen == TRUE)
   return 1;
  m_fFile = get_option("COM_file")->i;
  n       = get_option("COM_port")->s;
  if(m_fFile == FALSE)
   m_hdev = CreateFile(n,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);
  else
   m_hdev = CreateFile(n,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if(m_hdev==INVALID_HANDLE_VALUE)
  {
   message(F_DEBUG,"CreateFile(\"%s\") error: %u\n",n,GetLastError());
   goto IO_failed;
  }
  if(m_fFile == FALSE)
  {
   memset(&dcb,0,sizeof(dcb));
   dcb.DCBlength   = sizeof(DCB);
   dcb.fBinary     = TRUE;
   dcb.BaudRate    = get_option("COM_baud")->i;
   dcb.ByteSize    = get_option("COM_data")->i;
   dcb.StopBits    = get_option("COM_stop")->i;
   dcb.fDtrControl = DTR_CONTROL_ENABLE;
   dcb.fRtsControl = RTS_CONTROL_ENABLE;
   if(get_option("COM_parity")->i != NOPARITY)
   {
    dcb.fParity = TRUE;
    dcb.Parity  = get_option("COM_parity")->i;
   }
   if(get_option("COM_xflow")->i == TRUE)
   {
    dcb.fInX     = 
    dcb.fOutX    = true;
    dcb.XoffChar = get_option("COM_xoff")->i;
    dcb.XonChar  = get_option("COM_xon")->i;
    dcb.XoffLim  = 0x1000; // CHANGES
    dcb.XonLim   = 0x100;
   }
   
   if(!SetCommState(m_hdev,&dcb))
   {
    message(F_DEBUG,"SetCommState error: %u\n",GetLastError());
    goto IO_failed;
   }
   SetupComm(m_hdev,get_option("COM_rxsize")->i,get_option("COM_txsize")->i);
   COMMTIMEOUTS tm;
   memset(&tm,0,sizeof(tm));
   GetCommTimeouts(m_hdev,&tm);
   tm.WriteTotalTimeoutConstant = 2000;
   SetCommTimeouts(m_hdev,&tm);
  }else
   ResetEvent(m_wOver.hEvent);
  SetCommMask(m_hdev,EV_RXCHAR|EV_ERR);
  m_fOpen = TRUE;
  Signal(IO_RTS,FALSE);
  return 1;
IO_failed:
  if(m_hdev != INVALID_HANDLE_VALUE)
  {
   CloseHandle(m_hdev);
   m_hdev = INVALID_HANDLE_VALUE;
  }
  return 0;
 }

 long Comm::Close(void)
 {
  if(m_fOpen == TRUE)
  {
   _ASSERT(m_hdev != INVALID_HANDLE_VALUE);
   Signal(IO_RTS,FALSE);
   CloseHandle(m_hdev);
   m_fOpen = FALSE;
  }
  return 1;
 }

 long Comm::Lock(void)
 {
  return 1;
 }

 long Comm::Unlock(void)
 {
  return 1;
 }

 HANDLE Comm::WEvent(void)
 {
  HANDLE htm;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return INVALID_HANDLE_VALUE;
  }
  htm = m_wOver.hEvent;
  Unlock();
  return htm;
 }

 HANDLE Comm::GetReadEvent(void)
 {
  HANDLE htm;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return INVALID_HANDLE_VALUE;
  }
  htm = m_hTimer;
  Unlock();
  return htm;
 }

 HANDLE Comm::REvent(void)
 {
  HANDLE htm;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return INVALID_HANDLE_VALUE;
  }
  htm = m_Event.hEvent;
  Unlock();
  return htm;
 }

 DWORD Comm::Select(void)
 {
  COMSTAT stats;
  DWORD error;
  DWORD status = IO_ERROR;
  long count;

  Lock();
  if(m_fOpen == FALSE)  goto select_exit;
  if(m_fFile)
  {
   status = IO_OK;
   SetEvent(m_Event.hEvent);
   goto select_exit;
  }
  for(count = 0;count<10;count++)
  {
   if(WaitCommEvent(m_hdev,&m_MaskEvent,&m_Event))
    break;
   if(GetLastError() == ERROR_IO_PENDING)
   {
    status = IO_OK;
    goto select_exit;
   }
  }
  if(count == 10)
  {
   if(GetLastError()!=ERROR_IO_PENDING)
    message(F_DEBUG,"Select() WaitCommError()==FALSE\n");
   goto select_exit;
  }
  if((m_MaskEvent & EV_ERR) == EV_ERR)// && error)
  {
   if(ClearCommError(m_hdev,&error,&stats))
    message(F_DEBUG,"Error = %s, InputQueue=%d, OutputQueue=%d\n",
            error2string(error),stats.cbInQue,stats.cbOutQue);
   goto select_exit;
  }
  status = IO_OK;
select_exit:
  Unlock();
  return status;
 }

 long Comm::send_bytes(void)
 {
  DWORD bStatus;
  long diff;
  m_wOver.Internal     =
  m_wOver.InternalHigh = 
  m_wOver.Offset       =
  m_wOver.OffsetHigh   =  0;
  diff = m_WriteHigh-m_WriteLow;
  bStatus = WriteFile(m_hdev,m_WriteBuffer+m_WriteLow,diff,&m_WriteBytes,&m_wOver);
  if(bStatus == FALSE && GetLastError() != ERROR_IO_PENDING)
  {
   DWORD error;
   COMSTAT stat;
   ClearCommError(m_hdev,&error,&stat);
   message(F_DEBUG,"error write % bytes : %d\n",diff,error);
   m_WriteBytes = 0;
  }
  m_WriteStatus = TRUE;
  return 1;
 }

 long Comm::WriteFlush(void)
 {
  Lock();
  if(m_fOpen == FALSE || m_fFile == TRUE)
  {
   Unlock();
   return -1;
  }
  if(GetOverlappedResult(m_hdev,&m_wOver,(DWORD*)&m_WriteBytes,TRUE) == FALSE)
  {
   if(GetLastError() != ERROR_IO_PENDING)
   {
    DWORD error;
    COMSTAT stat;
    ClearCommError(m_hdev,&error,&stat);
    message(F_DEBUG,"error write % bytes : %d\n",m_WriteBytes,error);
    m_WriteBytes = 0;
   }
  }
  if(m_WriteBytes)
  {
   message_trace(TRACE_WRITE,(unsigned char*)m_WriteBuffer+m_WriteLow,m_WriteBytes);
  }
  m_WriteStatus = FALSE;
  m_WriteLow+=m_WriteBytes;
  if(m_WriteHigh>m_WriteLow)
  {
   int diff = m_WriteHigh-m_WriteLow;
   memmove(m_WriteBuffer,m_WriteBuffer+m_WriteLow,diff);
   m_WriteLow  = 0;
   m_WriteHigh = diff;
   send_bytes();
  }else
  {
   m_WriteLow  = 
   m_WriteHigh = 0;
   ResetEvent(m_wOver.hEvent);
  }
  Unlock();
  return 1;
 }

 long Comm::Write(void*buffer,long len)
 {
  long r=len;
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return -1;
  }
  if(!len)
  {
   Unlock();
   return len;
  }
  if( m_fFile == TRUE)
  {
   message_trace(TRACE_WRITE,(unsigned char*)buffer,len);
   Unlock();
   return r;
  }
  while(len>0)
  {
   if(add_bytes(*((unsigned char*)buffer))<0)
   {
    break;                 // FIXED add flush if full
   }else
   {
    buffer = (unsigned char*)buffer + 1;
    len--;
   }
  }
  if(m_WriteStatus == FALSE) send_bytes();
  Unlock();
  return r;
 }

 long Comm::Read(void*buffer,long len)
 {
  long read,count;
  COMSTAT stats;
  DWORD error;

  Lock();

  if(m_fOpen == FALSE || !len)
  {
   read = (!len)?len:-1;
   goto exit_read;
  }
  if(m_fFile == TRUE)
  {
/*
    static DWORD ts = GetTickCount();

    DWORD te = GetTickCount();
    if(te-ts<200)
     {
      read = 0;
      ts = te;
      goto exit_ok_read;
     }
*/
    for(count = 0;count<10;count++)
    {
     if(ReadFile(m_hdev,buffer,len,(DWORD*)&read,NULL) && read)
      break;
#ifndef _DEBUG
     SetFilePointer(m_hdev,0,NULL,FILE_BEGIN);
#endif
    }
    if(count == 10)
    {
     read = -1;
     goto exit_read;
    }
   goto exit_ok_read;
  }
  read = -1;
  if(!ClearCommError(m_hdev,&error,&stats)) 
   goto exit_read;
  if(error||!stats.cbInQue)
  {
   if(error)
   {
error_message:
    message(F_DEBUG,"Error=%s, InQueue=%d, OutQueue=%d\n",
            error2string(error),stats.cbInQue,stats.cbOutQue);
   }
   goto exit_read;
  }
  m_rOver.Internal     =
  m_rOver.InternalHigh = 
  m_rOver.Offset       =
  m_rOver.OffsetHigh   = 0;

  len = min((DWORD)len,stats.cbInQue);

  if(ReadFile(m_hdev,buffer,len,(DWORD*)&read,&m_rOver) || (
     GetLastError() == ERROR_IO_PENDING &&
     GetOverlappedResult(m_hdev,&m_rOver,(DWORD*)&read,TRUE)
  )) goto exit_ok_read;
  read = -1;
  ClearCommError(m_hdev,&error,&stats);
  goto error_message;
exit_ok_read:
  m_recvSize+=read;
  message_trace(TRACE_READ,(unsigned char*)buffer,read);
exit_read:
  Unlock();
  return read;
 }

 long Comm::Signal(long type,long status)
 {
  Lock();
  if(m_fOpen == FALSE)
  {
   Unlock();
   return -1;
  }
  if(m_fFile == TRUE)
  {
   Unlock();
   return 1;
  }
  switch(type)
  {
   case IO_DTR:
    EscapeCommFunction(m_hdev,(status)?SETDTR:CLRDTR);
   break;
   case IO_RTS:
    EscapeCommFunction(m_hdev,(status)?SETRTS:CLRRTS);
   break;
  }
  Unlock();
  return 1;
 }
 char *Comm::error2string(DWORD error)
 {
  int i = 0;
  static char *lpstr[] = 
  {
   "No error",
   "Receive Queue overflow",
   "Receive Overrun Error",
   "Receive Parity Error",
   "Receive Framing error",
   "Break Detected",
   "TX Queue is full",
   "Requested mode unsupported"
  };
  switch(error)
  {
  case CE_RXOVER:   i = 1; break;
  case CE_OVERRUN:  i = 2; break;
  case CE_RXPARITY: i = 3; break;
  case CE_FRAME:    i = 4; break;
  case CE_BREAK:    i = 5; break;
  case CE_TXFULL:   i = 6; break;
  case CE_MODE:     i = 7; break;
  }
  return lpstr[i];
 }
