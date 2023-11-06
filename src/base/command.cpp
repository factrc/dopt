
#include "dopt.h"

 CCommand::CCommand(int dwMaxCmd)
 {
  m_Root.cmdMaxCount = dwMaxCmd;
  m_Root.next        = &m_Root;
  m_Root.prev        = &m_Root;
  m_Root.cmdCount    = 0;
  m_Shared           = CreateEvent(NULL,FALSE,TRUE,NULL);
 }

 CCommand::~CCommand()
 {
  _ASSERT(m_Shared != NULL);
  Lock();
   struct ScmdEntry *p = m_Root.next;
   while(p!=&m_Root)
   {
    p = rm_entry(p);
   }
  Unlock();
  CloseHandle(m_Shared);
 }

 struct ScmdEntry *CCommand::rm_entry(struct ScmdEntry*p)
 {
  struct ScmdEntry *tmp = p->next;
  _ASSERT(p->cmd.data != NULL);
  free(p->cmd.data);
  p->prev->next = p->next;
  p->next->prev = p->prev;
  free(p);
  _ASSERT(m_Root.cmdCount>=0);
  m_Root.cmdCount-=1;
  return tmp;
 }

 int CCommand::Lock(int bWait)
 {
  DWORD w = (bWait == TRUE)?INFINITE:0;
  DWORD bStatus = WaitForSingleObject(m_Shared,w);
  if(bStatus != WAIT_OBJECT_0)
    return -1;
  return 1;
 }
 void CCommand::Unlock(void)
 {
  SetEvent(m_Shared);
 }

 int CCommand::Clear( void )
  {
   Lock();
   if(m_Root.next == &m_Root || m_Root.cmdCount == 0 ) // fast exit if command buffer empty
    {
     Unlock();
     return 1;
    }
   while( m_Root.next != &m_Root && m_Root.cmdCount != 0 )
    {
     struct ScmdEntry *p = m_Root.next;
     m_Root.next = rm_entry(p);
    }
   Unlock();
   return 1;
  }
 int CCommand::Push(struct SCMD *msg)
 {
   Lock();
   if(m_Root.cmdCount+1>m_Root.cmdMaxCount)
   {
    Unlock();
    return -1;
   }
   struct ScmdEntry *p = (struct ScmdEntry *)calloc(1,sizeof(ScmdEntry));
   if(!p)
   {
    Unlock();
    return -1;
   }
   memcpy(&p->cmd,msg,sizeof(SCMD));
   switch(msg->type)
   {
   case CMD_BIN:
     p->cmd.data = (unsigned char*)calloc(1,p->cmd.bin.size);
     memcpy(p->cmd.data,msg->data,msg->bin.size);
   break;    
   case CMD_MSG:
   case CMD_DEBUG:
     p->cmd.data = (unsigned char*)strdup((char*)msg->data);
   break;
   }
   p->prev           = m_Root.prev;
   p->next           = m_Root.prev->next;
   m_Root.prev->next = p;
   m_Root.prev       = p;
   m_Root.cmdCount  += 1;
  Unlock();
  return 1;
 }
 
 int CCommand::Pop(struct SCMD *msg)
 {
  if(Lock(FALSE)<0)
   return -1;
  if(m_Root.next == &m_Root || m_Root.cmdCount == 0) // fast exit if command buffer empty
  {
   Unlock();
   return -1;
  }
  struct ScmdEntry *p = m_Root.next;
  memcpy(msg,&p->cmd,sizeof(SCMD));
  switch(p->cmd.type)
  {
   case CMD_BIN:
    msg->data = (unsigned char*)calloc(1,p->cmd.bin.size);
    memcpy(msg->data,p->cmd.data,p->cmd.bin.size);
   break;    
   case CMD_MSG:
   case CMD_DEBUG:
    msg->data = (unsigned char*)strdup((char*)p->cmd.data);
   break;
   default:
    _ASSERT( p->cmd.data == NULL );
  }
  m_Root.next = rm_entry(p);
  Unlock();
  return 1;
 }
