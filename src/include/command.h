#define CMD_BIN          0x1
#define CMD_MSG          0x2
#define CMD_DEBUG        0x3
#define CMD_ENTER_MODULE 0x4
#define CMD_LEAVE_MODULE 0x5

 struct SCMD
 {
  int type;
  struct
  {
   unsigned char *data;
   union
   {
    struct
    {
     POINT p;
     DWORD attr;
     DWORD ttl;
    }msg;
    struct
    {
     DWORD          size;
    }bin;
   };
  };
 };
 
 struct ScmdEntry
 {
  struct ScmdEntry *next;
  struct ScmdEntry *prev;
  union
  {
   struct SCMD  cmd;
   struct
   {
    long cmdCount;
    long cmdMaxCount;
   };
  };
 };

 class CCommand
 {
  HANDLE m_Shared;
  struct ScmdEntry m_Root;
  int  Lock(int bWait = TRUE);
  void Unlock();
  struct ScmdEntry *rm_entry(struct ScmdEntry*p);
 public:
  CCommand(int MaxCmd = 0x100);
 ~CCommand();
  int Push(struct SCMD *msg);
  int Pop(struct SCMD *msg);
  int Clear( void );
 };
