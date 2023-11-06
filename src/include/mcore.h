
// define in sec
#define INIT_MODULE_TIMEOUT  25

#define INC_MODULE_COUNT()     m_RootList.currentModuleCount++
#define DEC_MODULE_COUNT()     m_RootList.currentModuleCount--

 class CModuleBase;

 struct s_modEntry
 {
  struct s_modEntry *prev;
  struct s_modEntry *next;
  union
  {
   struct
   {
    HANDLE       hStart;
    HANDLE       hExit;
    HANDLE       hThread;
    HANDLE       hRead;
    HANDLE      *hWrite;
    DWORD        idThread;
    CModule     *entryspot;
    CModuleBase *owner;
    DWORD        errorCount;
   };
////////////////////////////////////////
   struct
   {
    long    maxModuleCount;
    long    currentModuleCount;
   };
  };
 };

 class CModuleBase
 {
  DWORD    m_fOpen;
///////////////////////////////
  HANDLE   m_ModuleShared;
  HANDLE   m_hWriteEvent;
  struct s_modEntry m_RootList;
///////////////////////////////
  int                Lock(int bWait = TRUE);
  void               Unlock(void);
  struct s_modEntry *alloc_item(CModule*);
  void               free_item(struct s_modEntry*);
  int                Execute(struct s_modEntry*);
  friend unsigned _stdcall ThreadRoutine(void*);
 public:
  CModuleBase(DWORD Count = 0x100);
 ~CModuleBase();
  int    Init();
  int    Clear();
  HANDLE GetEvent(void);
  int    Insert(CModule *mod);
  int    Remove(CModule *mod);
  void   ExecuteModule(void);
  int    ExecuteCommandIfFound(unsigned long key);
  int    ExecuteCommandIfFound(void);
 };

 inline HANDLE CModuleBase::GetEvent(void)
 {
  HANDLE h;
  Lock();
  h = m_hWriteEvent;
  Unlock();
  return h;
 }

