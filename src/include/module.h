#define MOD_LOAD       0x01
#define MOD_OPEN       0x02
#define MOD_BUSY       0x04
#define MOD_CLOSE      0x08


 class CModule
 {
   struct __point
   {
    DWORD x;
    DWORD y;
    DWORD sz;
   };
   struct
   {
    char *Msg;
    char *Key;
    DWORD swap,num;
    __point pha;
    __point phb;
   }opt;
   HMODULE  m_hModule;
   DWORD    m_fOpen;
   CCommand m_InCmd;
   CCommand m_OutCmd;
   DWORD    m_Key;
   char    *FileName;
   char    *ModuleName;
/////////////////////////////////////////////////
   long (*p_init)(const char*);
   long (*p_done)(void);
   long (*p_process)(void**res,const char*param);
   long (*p_getstate)(long);
   long (*p_setstate)(long,void*,long);
   long   parse_return_value(char*rstr);
 public:
   CModule(char *name,char*fname);
  ~CModule();
   long      Open();
   long      Close();
   long      Execute(void);
   long      d_init(const char*);
   long      d_done(void);
   long      d_process(void**res,const char*param);
   long      d_getstate(long);
   long      d_setstate(long,void*,long);
   long      GetKey(void);
   long      GetKeyName ( char *buf, int size );
   char*     GetModuleName(void);
   long      ReadOutputCmd(SCMD *p);
   long      SetInputCmd(SCMD *p);
   long      isHaveMessage(void);
   long      PutAsyncMessage(void);
////////////////////////////////////////////////
   int       SetKey(char*Key);
   int       SetMessage(char *msg);
   int       SetPhoneA(char *);
   int       SetPhoneB(char *);
   int       SetSwap(int swap);
   int       SetNumber(int num);
 };

 inline long CModule::ReadOutputCmd(SCMD *p)
 {
  return m_OutCmd.Pop(p);
 }

 inline long CModule::GetKey(void)
 {
  return m_Key;
 }
 inline char* CModule::GetModuleName(void)
 {
  return (ModuleName)?ModuleName:"";
 }
 inline long CModule::isHaveMessage(void)
 {
  return opt.Msg?1:0;
 }
 inline int  CModule::SetNumber(int num)
 {
  opt.num = num;
  return 1;
 }
