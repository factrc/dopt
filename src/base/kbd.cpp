
#include "dopt.h"

#define CTRL(a)   ((a) == 0x1d || (a) == 0x38 )
#define SHIFT(a)  ((a) == 0x2A||(a) == 0x36)

 int    keyboard_input(int scan,int ench,int rp,int stat);
 int    keyboard_init(void);
 int    keyboard_lang_set(int);    
 int    keyboard_lang_switch(void);
 char*  keyboard_error(void);
 static int keyboard_LoadTable(void);
 unsigned long str_key_to_scan(const char*);

 static unsigned short    _table_keys[2][2][4][256];
 static int               _g_Lang,key_error;
 static CScanName         _key_name_define;
 static char *            _error_keyboard[] = 
 {
  "Раскладка клавиатуры неопределена",
  "Не найдена раскладка клавиатуры",
  "Файл раскладки клавиатуры поврежден",
  "При запуске скрипта произошла ошибка"
 };

 static long SetRus(unsigned long code);
 static long Reset(unsigned long code);
 unsigned long key_dopt_reset;
 unsigned long key_dopt_rus;

 int keyboard_init(void)
 {
  memset((void*)_table_keys,0,sizeof(_table_keys));
  keyboard_lang_set(RUS_LAYOUT);
  DWORD bStatus = keyboard_LoadTable();
  key_dopt_reset = str_key_to_scan(get_option("DOPT_reset_key")->s);
  key_dopt_rus   = str_key_to_scan(get_option("DOPT_layout_key")->s);
  return bStatus;
 }

 int keyboard_close(void)
 {
  return 0;
 }

 char* keyboard_error(void)
 {
  if(key_error<0)
  {
   int num = ((-key_error)-1)%(sizeof(_error_keyboard)/sizeof(char*));
   return _error_keyboard[num];
  }
  return "Клавиатура Ok";
 } 

 int keyboard_lang_switch(void)
 {
  message(F_DEBUG,"Language switch to %d\n",_g_Lang^1);
  return (_g_Lang ^= 1);
 }

 int  keyboard_lang_set(int l)
 {
  return _g_Lang = (l!=0)?1:0;
 }

 int  keyboard_clear_flags(void)
 {
  keyboard_input(0x1d,0,1,0); // clear CTRL
  keyboard_input(0x2a,0,1,0); // clear SHIFT
  return 1;
 }

/*
 unsigned long get_key_from_scan( unsigned long scan )
  {
    int shift = (scan>>16)&0xFF;
    int ctrl  = (scan>>24)&0xFF;
    int ench  = (scan>>8)&0x1;
    int pos,rus;
//    t_scan = scan|((ench<<8)&0x100)|(ctrl<<24)|(shift<<16);
    pos = ctrl<<1;
    rus = _g_Lang^ctrl;
    pos+=shift;
    return _table_keys[ench][rus][pos][scan];
  }
*/

 int keyboard_input(int scan,int ench,int rp,int stat)
 {
   static int  ctrl  = 0,
               shift = 0;
   int rus           = 0;
   int ret           = 0;
   int press_special = 0;
   int pos;
   unsigned char buf[256];
   unsigned long code;
   unsigned long t_scan;
   static   int  count_pressKey = 0;

   stat&=1;

   if(CTRL(scan))
    ctrl = stat;
   else
   if(SHIFT(scan)&&ench == 0)
    shift = stat;
   else
   {
    if(stat)
    { 
      if ( scan == 0x3e && ench == 0 && ctrl == 1 && shift == 1 )// ctrl+shift+F4  Hehe :)
        return 1;

      t_scan = scan|((ench<<8)&0x100)|(ctrl<<24)|(shift<<16);
      pos = ctrl<<1;
      rus = _g_Lang^ctrl;
      pos+=shift;
      code = _table_keys[ench][rus][pos][scan];

      count_pressKey++;

      if(t_scan == key_dopt_reset)
      {
       Reset(t_scan);
      }else
       if(t_scan == key_dopt_rus)
       {
        SetRus(t_scan);
       }else
        if( t_scan == 0x10177 || count_pressKey > 3 ) // SHIFT + KEY_CT
        {
         pIOMessage->Clear();
         count_pressKey = 0;
        }
      rp = rp%2;
      pModuleDevice->ExecuteCommandIfFound(t_scan);
      while(rp-->0)
      {
       if(code&0xFF00)
        buf[ret++] = (unsigned char)(code>>8);
       buf[ret++] =  (unsigned char)(code&0xFF); 
      }
      IODevice.Write(buf,ret);
    }
   }
  return 0;
 }

 static long SetRus(unsigned long code)
 {
  int stat = keyboard_lang_switch()&1;
  struct
  {
   unsigned char*m;
   char attr;
  }msg[] = 
  {
   {(unsigned char*)"         ",(char)color_table[8]},
   {(unsigned char*)"CYRILLIC ",(char)color_table[3]}
  };
  WN_Puts_Abs_Fix(SCRROW,SCRCOL-9,msg[stat].m,msg[stat].attr);
  ScreenDevice.Flush();
//  while(ScreenDevice.Update()!=1);
  return 0;
 }

 static long Reset(unsigned long code)
 {
  ScreenDevice.Clear();
  message(F_OUTPUT,"Перезагрузка терминала... Ждите!");
  ScreenDevice.CursorSize(TRUE,CURSOR_BLOCK);
  ScreenDevice.CursorVisible(TRUE,CURSOR_ENABLE);
//  ScreenDevice.Update();
  IODevice.Signal(IO_RTS,FALSE);
  Sleep(get_option("COM_rts_delay")->i);
  IODevice.Signal(IO_RTS,TRUE);
  return 0;
 }


 static int keyboard_LoadTable(void)
 {
  FILE *infd;
  char str[1024],keys_name[255],*table_name;

  if(!(table_name = get_option("KEYBOARD_table")->s))
  {
   key_error = -1; 
   return 0;
  }
  if(!(infd = fopen(table_name,"rt")))
  {
   key_error = -2;
   return 0;
  }
  while(fgets(str,sizeof(str),infd))
  {
   if(*str=='\n'||*str==';') continue;
   int ech,scan,raw,shift,ctrl,mix,raw1,shift1,ctrl1,mix1;

   if(sscanf(str,"%x%x%x%x%x%x%x%x%x%x%[^\n]",&ech,&scan,&raw,&shift,&ctrl,&mix,
      &raw1,&shift1,&ctrl1,&mix1,keys_name)!=11)
   {
    fclose(infd);
    key_error = -3;
    return 0;
   }
   if(_key_name_define.Find(keys_name))
   {
    message(F_DEBUG,"Warning: key name dublicate allow only first key\n");
   }
//                   eng layout
   _table_keys[ech&1][ 0 ][0][scan&0xFF] = raw;
   _table_keys[ech&1][ 0 ][1][scan&0xFF] = shift;
   _table_keys[ech&1][ 0 ][2][scan&0xFF] = ctrl;
   _table_keys[ech&1][ 0 ][3][scan&0xFF] = mix;
//                  another layout
   _table_keys[ech&1][ 1 ][0][scan&0xFF] = raw1;
   _table_keys[ech&1][ 1 ][1][scan&0xFF] = shift1;
   _table_keys[ech&1][ 1 ][2][scan&0xFF] = ctrl1;
   _table_keys[ech&1][ 1 ][3][scan&0xFF] = mix1;
// insert key-scan and binding to name
   _key_name_define.Insert(keys_name,(ech)?(scan|0x100):scan);
  }
  fclose(infd);
  return 1;
 }

 unsigned long str_key_to_scan(const char*keyname)
 {
// 4 byte
// 76543210 76543210 76543210 76543210
// 00000000 00000000 00000000 00000000
//   ctrl     shift   extend  scancode
  unsigned long scan=0;
  const char *start,*end;
  start = keyname;
  if(!keyname) return 0;
  end = strchr(start,'+');
  while(end)
  {
   if(!strnicmp(keyname,"ctrl",4)||!strnicmp(keyname,"alt",3))
   {
    scan |= 0x1000000;
   }else
    if(!strnicmp(keyname,"shift",5))
    {
     scan |=0x10000;
    }else
    {
     message(F_DEBUG,"invalid keyname: %s\n",keyname);
     return 0;
    }
   start = end+1;
   end = strchr(start,'+');
  }
  if(!_key_name_define.Find(start))
  {
   message(F_DEBUG,"invalid keyname: %s\n",keyname);
   return 0;
  }
  return scan|=_key_name_define.Find(start);
 }

 unsigned long CScanName::Find(const char*name)
 {
  struct _key_scan_name *ptr = list;
  while(ptr)
  {
   if(!stricmp(name,ptr->name)) return ptr->scan;
   ptr = ptr->next;
  }
  return 0;
 }

 unsigned long CScanName::Insert(const char*name,unsigned long scan)
 {
  struct _key_scan_name *ptr = (struct _key_scan_name*)
                               calloc(1,sizeof(struct _key_scan_name));
  if(ptr)
  {
   ptr->next = list;
   ptr->name = trim(strdup(name));
   ptr->scan = scan;
   list = ptr;
  }
  return ptr!=NULL;
 }
 
 void CScanName::removeall(void)
 {
  struct _key_scan_name *ptr = list,*tmp;
  while(ptr)
  {
   tmp = ptr->next;
   free(ptr->name);
   free(ptr);
   ptr = tmp;
  }
  list = NULL;
 }

#undef CTRL
#undef SHIFT
