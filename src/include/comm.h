 

 #define IO_ERRORBASE       0x00800000
 #define IO_OK		          0x00000000
 #define IO_ERROR	          (IO_ERRORBASE+1)
 #define IO_UNPLEMENTED	    (IO_ERRORBASE+2)
 #define IO_BUSY	          (IO_ERRORBASE+3)
 #define IO_INVALIDARG	    (IO_ERRORBASE+4)
 #define IO_PENDING	        (IO_ERRORBASE+5)
 #define IO_EXIT	          (IO_ERRORBASE+6)
 #define IO_UPDATE	        (IO_ERRORBASE+7)

 #define IO_RTS             0x1
 #define IO_DTR             0x2

 class Comm
 {
  long   m_fFile,m_fOpen;
  HANDLE m_hdev;
  HANDLE m_hTimer;
  OVERLAPPED m_rOver;
  OVERLAPPED m_wOver;
  OVERLAPPED m_Event;
  unsigned char*m_WriteBuffer;
  DWORD      m_MaskEvent;
  DWORD      m_WriteBytes;
  DWORD      m_WriteLow;
  DWORD      m_WriteHigh;
  DWORD      m_WriteMax;
  DWORD      m_WriteStatus;
  DWORD      m_recvSize;
  long Lock(void);
  long Unlock(void);
  long add_bytes(unsigned char);
  long send_bytes(void);
  char *error2string(DWORD error);
 public:
  Comm();
 ~Comm();
  long   Open(void);
  long   Close(void);
  long   Write(void*buf,long len);
  long   Read(void *buf,long len);
  long   Signal(long,long);
  HANDLE WEvent(void);
  HANDLE GetReadEvent(void);
  HANDLE REvent(void);
  DWORD  Select(void);
  long   WriteFlush(void);
 };

 inline long Comm::add_bytes(unsigned char bb)
 {
  if(m_WriteHigh<m_WriteMax)
  {
   m_WriteBuffer[m_WriteHigh++] = bb;
   return 1;
  }
  return -1;
 }
