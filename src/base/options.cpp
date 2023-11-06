 

 #include "dopt.h"

 #define OPTSTR      1
 #define OPTINT      2
 #define OPTFLOAT    3
 #define OPTFLAG     4
 #define REGISTER(a,b,c) RegisterParam((a),(b),(void*)(c))

 struct __int_arg
 {
  struct __int_arg *next;
  char *name;
  char *value;
 };

 struct __soption
 {
   struct __soption *next;
   char*name;
   int (*fn)(char *str);
 };
 struct __sparam
 {
  char *name;
  struct __soption *opt;
 };

 
 static unsigned long asz = 0;
 static Option *aopt,VOID_OPTION;
 static struct __sparam paramlist[] = 
 {
   {"COM",NULL},
   {"DOPT",NULL},
   {"MODULE",NULL},
   {"CONSOLE",NULL}
 };
 struct __int_arg *intlist;


 static int init_internal_arg(void);
 static int parse_internal_arg(char *s,char **pdest);

 static int read_config_file(char*);
 static int parse_opt(int index,char *str,int len);
 static int read_param(char **str,char **opt,char **value);


 static int com_define(char*str);
 static int baud_define(char*str);
 static int parity_define(char*str);
 static int stop_define(char*str);
 static int xflow_define(char*str);
 static int xon_define(char*str);
 static int xoff_define(char*str);
 static int tx_define(char*str);
 static int rx_define(char*str);
 static int trace_define(char*str);
 static int rtsdelay_define(char*str);

 static int dopt_key_define(char*str);
 static int dopt_print_define(char*str);
 static int dopt_color_define(char*str);
 static int dopt_resetk_define(char*str);
 static int dopt_layoutk_define(char*str);

 static int console_row_define(char*str);
 static int console_col_define(char*str);
 static int console_rect_define(char*str);
 static int console_mode_define(char*str);
 static int console_delay_define(char*str);
 static int console_debug_define(char*str);
 static int console_font_define(char*str);

 static int module_name_define(char*str);

 static int RegisterParam(char *section,char *name,void *fun)
 {
   for(int i = 0;i<sizeof(paramlist)/sizeof(paramlist[0]);i++)
   {
    if(!strcmp(paramlist[i].name,section))
    {
     struct __soption *p = (struct __soption *)calloc(1,sizeof(struct __soption));
     p->next = paramlist[i].opt;
     p->name = strdup(name);
     p->fn   = (int (*)(char*))fun;
     paramlist[i].opt = p;
     return 1;
    }
   }
  return 0;
 }

 Option* get_option(char*name)
 {
  if(aopt)
  {
    for(unsigned long i=0;i<asz;i++)
    {
     if(((unsigned long*)aopt[i].name)[0]!=((unsigned long*)name)[0])
      continue;
     if(!strcmp(aopt[i].name,name))
      return &aopt[i];
    }
  }
  return &VOID_OPTION;
 }

 long set_option(char*name,unsigned long value,int type)
 {
  Option *op = get_option(name);
  if(op==&VOID_OPTION)
  { 
    if(!aopt) aopt = (Option*) calloc(1,sizeof(Option));
    else      aopt = (Option*)realloc(aopt,sizeof(Option)*(asz+1));
    if(!aopt) return 0;
    op = &aopt[asz++];
    memset(op,0,sizeof(Option));
    strcpy(op->name,name);
  }
  switch(type)
  {
    case OPTSTR:
     if(op->s) free(op->s);
     op->s = strdup((char*)value);
    break;
    case OPTINT:  
     op->i = (int)value;
    break;
    default:
    return 0;
  }
  return 1;
 }


 int parse_argument(char*argv)
 {
   init_internal_arg();
   REGISTER("COM","Port",com_define);
   REGISTER("COM","Baud",baud_define);
   REGISTER("COM","Parity",parity_define);
   REGISTER("COM","Stop",stop_define);
   REGISTER("COM","Xflow",xflow_define);
   REGISTER("COM","Xon",xon_define);
   REGISTER("COM","Xoff",xoff_define);
   REGISTER("COM","Txsize",tx_define);
   REGISTER("COM","Rxsize",rx_define);
   REGISTER("COM","Trace",trace_define);
   REGISTER("COM","Rtsdelay",rtsdelay_define);

   REGISTER("DOPT","Keyboard",dopt_key_define);
   REGISTER("DOPT","Print_status",dopt_print_define);
   REGISTER("DOPT","Color",dopt_color_define);
   REGISTER("DOPT","Reset_key",dopt_resetk_define);
   REGISTER("DOPT","Layout_key",dopt_layoutk_define);


   REGISTER("CONSOLE","Row",console_row_define);
   REGISTER("CONSOLE","Col",console_col_define);
   REGISTER("CONSOLE","Rect",console_rect_define);
   REGISTER("CONSOLE","Mode",console_mode_define);
   REGISTER("CONSOLE","Delay",console_delay_define);
   REGISTER("CONSOLE","Debug",console_debug_define);
   REGISTER("CONSOLE","Font",console_font_define);


   REGISTER("MODULE","Name",module_name_define);



// --------------------------------------------
   set_option("FLAG_debug",0,OPTINT);
// --------------------------------------------
   set_option("WINDOW_event_delay",DELAY_BETWEEN_EVENTS,OPTINT);   
   set_option("WINDOW_x",WIN_POS_DEFAULT_X,OPTINT);
   set_option("WINDOW_y",WIN_POS_DEFAULT_Y,OPTINT);
   set_option("WINDOW_width",WIN_WIDTH_DEFAULT,OPTINT);
   set_option("WINDOW_height",WIN_HEIGHT_DEFAULT,OPTINT);
   set_option("WINDOW_mode",0,OPTINT);
// "WINDOW_font"
// --------------------------------------------
   set_option("PRINT_status",PRINT_BUSY1,OPTINT);
// --------------------------------------------
   set_option("COM_file",0,OPTINT);
   set_option("COM_port",(unsigned long)COMM_DEFAULT_PORTNAME,OPTSTR);
   set_option("COM_baud",COMM_DEFAULT_BAUD,OPTINT);
   set_option("COM_data",COMM_DEFAULT_BITSDATA,OPTINT);
   set_option("COM_stop",COMM_DEFAULT_STOP,OPTINT);
   set_option("COM_parity",COMM_DEFAULT_PARITY,OPTINT);
   set_option("COM_xon",COMM_DEFAULT_XON,OPTINT);
   set_option("COM_xoff",COMM_DEFAULT_XOFF,OPTINT);
   set_option("COM_rts_delay",COMM_DEFAULT_RTSDELAY,OPTINT);
   set_option("COM_rxsize",COMM_DEFAULT_RXSIZE,OPTINT);
   set_option("COM_txsize",COMM_DEFAULT_TXSIZE,OPTINT);
   set_option("COM_xflow",0,OPTINT);
// --------------------------------------------
   set_option("CONSOLE_col",DOPTCOL,OPTINT);
   set_option("CONSOLE_row",SCRROW+1,OPTINT);
// CONSOLE_color
// --------------------------------------------
   set_option("KEYBOARD_table",(unsigned long)KEYBOARD_TABLE_DEFAULT,OPTSTR);
// --------------------------------------------
   if(!read_config_file(argv))
   {
    message(F_DEBUG|F_OUTPUT,"Ошибка при разборе файла конфигурации\n");
    return -1;
   }
   return 1;
 } 

 static int read_param(char **str,char **opt,char **value)
 {
  long sz1 = -1,sz2 = -1;
  char *s1,*s2,*ptr = *str;
 
  while(*ptr && isspace(*ptr)) ptr++;
  if(!*ptr)return -1;
  s1 = ptr;
  while(*ptr && *ptr!='=' && !isspace(*ptr)) ptr++;
  sz1 = ptr-s1;
  while(*ptr && *ptr!='=') ptr++;
  if(!*ptr++) return 0;
  while(*ptr && isspace(*ptr)) ptr++;
  if(*ptr == '[')
  {
   s2 = ++ptr;
   while(*ptr && *ptr!=']')
   {
    if(*ptr++ == '"')
    {
     while(*ptr && *ptr!='"') ptr++;
     if(*ptr == '"') ptr++;
    }
   }
   if(*ptr!=']')
   {
    message(F_DEBUG,"expected ']'\n");
    return 0;
   }
   sz2 =  ptr-s2;
   ptr++;
  }else
   if(*ptr == '"')
   {
    s2 = ++ptr;
    while(*ptr && *ptr!='"') ptr++;
    if(*ptr!='"')
    {
     message(F_DEBUG,"expected '\"'\n");
     return 0;
    }
    sz2 = ptr-s2;
    ptr++;
   }else
   {
    s2 = ptr;
    while(*ptr && !isspace(*ptr)) ptr++;
    sz2 = ptr-s2;
   }
  if(sz1!=-1 && sz2!=-1)
  {
   *opt   = (char*)calloc(1,sz1+1);
   *value = (char*)calloc(1,sz2+1);
   strncpy(*opt,s1,sz1);
   (*opt)[sz1]   = 0;
   strncpy(*value,s2,sz2);
   (*value)[sz2] = 0;
   *str = ptr;
   return 1;
  }
  return 0;
 }

 static int read_config_file(char*argv)
 {
  FILE *fdin;
  char *buffer,*end,*ptr,*s,*e;
  int filesize;

  if((fdin = fopen(CONFIG_FILE,"rt"))==NULL) return 1;
  
  filesize = filelength(fileno(fdin));
  buffer   = (char*)calloc(1,filesize);
  if(!buffer)
  {
   fclose(fdin);
   return 0;
  }
  fread(buffer,1,filesize,fdin);
  fclose(fdin);

  ptr = buffer;
  end = buffer+filesize;
  while(ptr<end)
  {
   while(*ptr && isspace(*ptr)) ptr++;
   if(!*ptr) break;
   if(*ptr == '#')
   {
    while(*ptr && *ptr!='\n') ptr++;
    continue;
   }
   while(*ptr && isspace(*ptr)) ptr++;
   s = ptr;
   while(*ptr && *ptr != '=' &&!isspace(*ptr)) ptr++;
   e = ptr;
   while(*ptr && *ptr != '=') ptr++;
   if(!*ptr) continue;
   *e = 0;
   long check = sizeof(paramlist)/sizeof(paramlist[0]);
   for(int i = 0;i<sizeof(paramlist)/sizeof(paramlist[0]);i++)
   {
    if(paramlist[i].name && !strncmp(paramlist[i].name,s,e-s)) break;
   }
   if( i != sizeof(paramlist)/sizeof(paramlist[0]))
   {
    while(*ptr && *ptr!='{') ptr++;
    if(!*ptr++)
    {
     message(F_DEBUG,"expected '{'\n");
     free(buffer);
     return 0;
    }
    s=ptr;
    while(*ptr && *ptr!='}')
    {
     if(*ptr == '"') 
     {
      while(*ptr && *ptr != '"') ptr++;
      if(!*ptr++)
      {
       message(F_DEBUG,"expected '\"'\n");
       free(buffer);
       return 0;
      }
     }
     if(*ptr == '#' )
     {
      while(*ptr && *ptr != '\n') ptr++;
     }
     if(*ptr) ptr++;
    }
    e = ptr;
    if(!*ptr++)
    {
     message(F_DEBUG,"expected '}'\n");
     free(buffer);
     return 0;
    }
    char *ppp = (char*)calloc(1,e-s+1);
    memcpy(ppp,s,e-s);
    s = ppp;
    while(*s)
    {
     if(*s == '#')
     {
      e = s;
      while(*e && *e!='\n') e++;
      memmove(s,e,strlen(e)+1);
     }else
      if(*s == '"')
      {
       while(*s && *s != '"') s++;
       if(*s == '"') s++;
      }else s++;
    }
    if(!parse_opt(i,ppp,strlen(ppp)))
    {
     free(ppp);
     free(buffer);
     return 0;
    }
    free(ppp);
   }else
   {
    message(F_DEBUG,"unknown %s\n",s);
    free(buffer);
    return 0;
   }
  }
  free(buffer);
  return 1;
 }

 static int parse_opt(int index,char *str,int len)
 {
  int code;
  char *end = str+len,*m1,*m2;
  while(str<end)
  {
   if(!(code = read_param(&str,&m1,&m2))) 
   {
    message(F_DEBUG,"param not define\n");
    return 0;
   }
   if(code == -1) break;
   struct __soption *ptr = paramlist[index].opt;
   while(ptr)
   {
    if(!strcmp(ptr->name,m1))
    {
     if(!ptr->fn(m2))
     {
      message(F_DEBUG,"bad value %s = %s in section %s\n",m1,m2,paramlist[index].name);
      free(m1);
      free(m2);
      return 0;
     }
     break;
    }
    ptr = ptr->next;
   }
   free(m1);
   free(m2);
  }
  return 1;
 }

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
 static int init_internal_arg(void)
 {
  char root[MAX_PATH];
  if(!GetCurrentDirectory(sizeof(root),root)) return 0;
  struct __int_arg  *ptr = (struct __int_arg  *)calloc(1,sizeof(struct __int_arg));
  ptr->name  = "$ROOT$";
  ptr->value = strdup(root);
  ptr->next  = intlist;
  intlist    = ptr;
  return 1;
 }
 static int parse_internal_arg(char *s,char **pdest)
 {
  struct __int_arg  *ptr = intlist;
  *pdest = NULL;
  while(ptr)
  {
   char *psrc;
   if((psrc = strstr(s,ptr->name)))
   {
    int size = strlen(s)-strlen(ptr->name)+strlen(ptr->value)+1;
    *pdest = (char*)calloc(1,size);
    strncpy(*pdest,s,psrc-s);
    strcat(*pdest,ptr->value);
    strcat(*pdest,psrc+strlen(ptr->name));
   }
   ptr = ptr->next;
  }
  if(!*pdest)
  {
   *pdest = strdup(s);
  }
  return 1;
 }

 static int ivaluetype(char*str,char*frm)
 {
  if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
      strcpy(frm,"0x%X");
  else
      strcpy(frm,"%d");
  return 0;
 }

 static int com_define(char*str)
 {
  char *buf;
  int ffile = 0;
  parse_internal_arg(str,&buf);
  if(strchr(buf,'/')||strchr(buf,'\\'))
    ffile = 1;
  set_option("COM_port",(unsigned long)buf,OPTSTR);
  if(ffile)
   set_option("COM_file",ffile,OPTINT);
  free(buf);
  return 1;
 }
 static int baud_define(char*str)
 {
  long n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("COM_baud",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int parity_define(char*str)
 {
  long parity;
  if(!strncmp(str,"EVEN",4))
  {
   parity = EVENPARITY;
  }else
   if(!strncmp(str,"ODD",3))
   {
    parity = ODDPARITY;
   }else
    if(!strncmp(str,"NONE",4))
    {
     parity = NOPARITY;
    }else
     return 0;
  set_option("COM_parity",parity,OPTINT);
  return 1;
 }
 static int stop_define(char*str)
 {
  long n;
  if(!strncmp(str,"ONE",3))
  {
   n = ONESTOPBIT;
  }else
   if(!strncmp(str,"ONE5",4))
   {
    n = ONE5STOPBITS;
   }else
     if(!strncmp(str,"TWO",3))
     {
      n = TWOSTOPBITS;
     }else
     {
      message(F_DEBUG,"stopbits unknown value: %s\n",str);
      return 0;
     }
  set_option("COM_stop",n,OPTINT);
  return 1;
 }
 static int xflow_define(char*str)
 {
  int status;
  if(!strncmp(str,"FALSE",5))
  {
   status = 0;
  }else
   if(!strncmp(str,"TRUE",4))
   {
    status = 1;
   }else
     return 0;
  set_option("COM_xflow",status,OPTINT);
  return 1;
 }
 static int xon_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("COM_xon",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int xoff_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("COM_xoff",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int tx_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("COM_txsize",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int rx_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("COM_rxsize",n,OPTINT);
  }else return 0;
  return 1;
 }

 static int rtsdelay_define(char*str)
 {
  int dim,n;
  char *ptr = str,frm[20];
  if(*ptr == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr+=2;
  while(*ptr&&isdigit(*ptr)) ptr++;
  if(!*ptr) dim = 1;
  else 
   if(!strncmp(ptr,"ms",2))   dim = 1;
   else
    if(!strncmp(ptr,"sec",3)) dim = 1000;
  *ptr = 0;
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)!=1) return 0;
  set_option("COM_rts_delay",n*dim,OPTINT);
  return 1;
 }

 static int trace_define(char*str)
 {
  char *s;
  int type;
  int offset;
  while(*str)
  {
   while(*str && isspace(*str)) str++;
   if(!*str) break;
   if(!strncmp(str,"input",5))
   {
    offset = 5;
    type   = 1;
   }else
    if(!strncmp(str,"output",6))
    {
     offset = 6;
     type   = 2;
    }else
     return 0;
   str = str+offset;
   while(*str && isspace(*str)) str++;
   if(!*str++)
   {
    message(F_DEBUG,"trace: not found separator\n");
    return 0;
   }
   while(*str && isspace(*str)) str++;
   if(*str == '"')
   {
    s = ++str; 
    while(*str && *str!='"') str++;
    if(!*str) return 0;
   }else
   {
    s = str;
    while(*str && !isspace(*str)) str++;
   }
   if(strncmp(s,"NONE",4))
   {
    char *buf,*b1;
    set_option("COM_trace",1,OPTINT);
    b1 = (char*)calloc(1,str-s+1);
    strncpy(b1,s,str-s);
    parse_internal_arg(b1,&buf);
    switch(type)
    {
    case 1:
     set_option("COM_trace_input",(unsigned long)buf,OPTSTR);
    break;
    case 2:
     set_option("COM_trace_output",(unsigned long)buf,OPTSTR);
    break;
    }
    free(buf);
    free(b1);
   }
  }
  return 1;
 }

 static int dopt_key_define(char*str)
 {
  char *buf;
  parse_internal_arg(str,&buf);
  set_option("KEYBOARD_table",(unsigned long)buf,OPTSTR);
  free(buf);
  return 1;
 }
 static int dopt_print_define(char*str)
 {
  if(!strncmp(str,"READY",5))
  {
   set_option("PRINT_status",PRINT_READY,OPTINT);
  }else
    if(!strncmp(str,"BUSY",4))
    {
     set_option("PRINT_status",PRINT_BUSY2,OPTINT);
    }else
     if(!strncmp(str,"NONE",4))
     {
      set_option("PRINT_status",PRINT_BUSY1,OPTINT);
     }else
       return 0;
  return 1;
 }

 static int dopt_color_define(char*str)
 {
  unsigned char icolor[32];
  int iindex = 0,size,hex;
  char sbuf[20];

  size = PDC_Get_Color_Table(NULL,0);
  _ASSERT(size == sizeof(icolor)/sizeof(icolor[0]));
  PDC_Get_Color_Table(icolor,size);
  hex = icolor[iindex];
  while(*str)
  {
   while( *str && isspace(*str) ) str++; // space
   char *s = str;
   while( *str && !isspace(*str) && *str!=',') str++;
   size = min(str-s,sizeof(sbuf)-1);
   memcpy(sbuf,s,size);
   sbuf[size] = 0;
   if(*str == ',') str++;
   if(sscanf(sbuf,"%X",&hex)!=1)
   {
    iindex++;
    continue;
   }
   if(iindex<sizeof(icolor)/sizeof(icolor[0]))
   {
    icolor[iindex++] = hex&0xFF;
   }
  }
  size = PDC_Get_Color_Table(NULL,0);
  PDC_Set_Color_Table(icolor,size);
  return 1;
 }

 static int console_row_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("CONSOLE_row",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int console_col_define(char*str)
 {
  int n;
  char frm[20];
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)==1)
  {
   set_option("CONSOLE_col",n,OPTINT);
  }else return 0;
  return 1;
 }
 static int console_rect_define(char*str)
 {
  int x,y,w,h;
  if(sscanf(str,"%d,%d,%d,%d",&x,&y,&w,&h)!=4)
   if(sscanf(str,"%X,%X,%X,%X",&x,&y,&w,&h)!=4)
   {
    return 0;
   }
  set_option("WINDOW_x",x,OPTINT);
  set_option("WINDOW_y",y,OPTINT);
  set_option("WINDOW_width",w+8,OPTINT);
  set_option("WINDOW_height",h+27,OPTINT);
  return 1;
 }
 static int console_mode_define(char*str)
 {
  if(!strncmp(str,"WINDOW",6))
  {
   set_option("WINDOW_mode",1,OPTINT);
  }else
   if(!strncmp(str,"FULLSCREEN",10))
   {
    set_option("WINDOW_mode",0,OPTINT);
   }
  return 1;
 }
 static int console_delay_define(char*str)
 {
  int dim,n;
  char *ptr = str,frm[20];
  if(*ptr == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr+=2;
  while(*ptr&&isdigit(*ptr)) ptr++;
  if(!*ptr) dim = 1;
  else 
   if(!strncmp(ptr,"ms",2))   dim = 1;
   else
    if(!strncmp(ptr,"sec",3)) dim = 1000;
  *ptr = 0;
  ivaluetype(str,frm);
  if(sscanf(str,frm,&n)!=1) return 0;
  set_option("WINDOW_event_delay",n*dim,OPTINT);
  return 1;
 }
 static int console_debug_define(char*str)
 {
  if(!strncmp(str,"NONE",4))
  {
   set_option("FLAG_debug",0,OPTINT);
  }else
  {
   set_option("FLAG_debug",1,OPTINT);
   if(!strncmp(str,"WINDOW",6))
     set_option("DEBUG_options",(unsigned long)"WINDOW",OPTSTR);
   else
   {
     while(*str && isspace(*str))str++;
     char *s = str;
     if(*str == '"')  while(*str && *str!='"') str++;
     else             while(*str && !isspace(*str)) str++;
     if(*s == '"' && !*str) return 0;
     *str = 0;
     set_option("FLAG_debug",1,OPTINT);
     char *buf;
     parse_internal_arg(s,&buf);
     set_option("DEBUG_options",(unsigned long)buf,OPTSTR);
     free(buf);
   }
  }
  return 1;
 }
 static int console_font_define(char*str)
 {
  char *name = NULL;
  int quality = -1;
  int attr    = -1;
  while(*str)
  {
   while(*str && isspace(*str)) str++;
   if(!*str) break;
   char *s = str;
   while(*str && !isspace(*str) && *str!='=')str++;
   if(!strncmp(s,"quality",str-s))
   {
    while(*str && isspace(*str)) str++;
    if(*str == '=')
    {
     str++;
     while(*str && isspace(*str)) str++;
     char *s = str;
     if(*str == '"')
     {
      s++;
      str++;
      while(*str && *str!='"') str++;
      if(*str != '"') break;
      str++;
     }else
      while(*str && !isspace(*str)) str++;
     if(!strncmp(s,"PROOF",str-s))
     {
      quality = PROOF_QUALITY;
     }else
      if(!strncmp(s,"DRAFT",str-s))
      {
       quality = DRAFT_QUALITY;
      }else
       if(!strncmp(s,"DEFAULT",str-s))
       {
        quality = DEFAULT_QUALITY;
       }else
        break;
    }else break;
   }else
    if(!strncmp(s,"name",str-s))
    {
     while(*str && isspace(*str)) str++;
     if(*str == '=')
     {
      str++;
      while(*str && isspace(*str)) str++;
      char *s = str;
      if(*str == '"')
      {
       s++;
       str++;
       while(*str && *str!='"') str++;
       if(*str != '"') break;
      }else
       while(*str && !isspace(*str)) str++;
      if(name) free(name);
      name = (char*)calloc(1,str-s+1);
      strncpy(name,s,str-s);
      if(*str == '"') str++;
     }else break;
    }else
     if(!strncmp(s,"attr",str-s))
     {
      while(*str && isspace(*str)) str++;
      if(*str == '=')
      {
       str++;
       while(*str && isspace(*str)) str++;
       char *s = str;
       if(*str == '"')
       {
        s++;
        str++;
        while(*str && *str!='"') str++;
        if(*str != '"') break;
        str++;
       }else
        while(*str && !isspace(*str)) str++;
       if(!strncmp(s,"THIN",str-s))
       {
        attr = FW_THIN;
       }else
        if(!strncmp(s,"LIGHT",str-s))
        {
         attr = FW_LIGHT;
        }else
         if(!strncmp(s,"NORMAL",str-s))
         {
          attr = FW_NORMAL;
         }else
          if(!strncmp(s,"MEDIUM",str-s))
          {
           attr = FW_MEDIUM;
          }else
           if(!strncmp(s,"BOLD",str-s))
           {
            attr = FW_BOLD;
           }else
            if(!strncmp(s,"EXTRABOLD",str-s))
            {
             attr = FW_EXTRABOLD;
            }else
              break;
      }else break;
     }
  }
  if(!name || quality == -1 || attr == -1)
  {
   if(name) free(name);
   return 0;
  }
  ScreenDevice.SelectFont(name,quality,attr);
  return 1;
 }

 static int dopt_resetk_define(char*str)
 {
  set_option("DOPT_reset_key",(unsigned long)str,OPTSTR);
  return 1;
 }
 static int dopt_layoutk_define(char*str)
 {
  set_option("DOPT_layout_key",(unsigned long)str,OPTSTR);
  return 1;
 }

 static int read_old_module_config(char *fname);

 static int module_name_define(char*str)
 {
  char *buf;
  int ffile = 0;
  parse_internal_arg(str,&buf);
  read_old_module_config(buf);
  free(buf);
  return 1;
 }

 int ins_module(char*name,char*mname,char*Key,char*asyncmsg,char*phonea,char*phoneb,int swap,int onlynum)
 {
  CModule *pmod;
  pmod = new CModule(name,mname);
  pmod->SetKey(Key);
  if(asyncmsg[0])
   pmod->SetMessage(asyncmsg);
  if(phonea[0])
   pmod->SetPhoneA(phonea);
  if(phoneb[0])
   pmod->SetPhoneB(phoneb);
  pmod->SetSwap(swap);
  pmod->SetNumber(onlynum);
  pModuleDevice->Insert(pmod);
  return 1;
 }

 static int read_old_module_config(char *fname)
 {
  FILE *fd;
  int exit = 0;
  char buf[4096],*pstr;
  int onlynum,swap;
  char *s,*v,
        name[1024],
        mname[1024],
        Key[255],
        phonea[255],
        phoneb[255],
        asyncmsg[255];

  fd = fopen(fname,"rt");
  if(!fd) return -1;
  while(fgets(buf,sizeof(buf)-1,fd))
  {
   if((pstr = strchr(buf,'\n'))) *pstr = 0;
   if(!*buf || *buf == '#') continue;
   fname[0]    = 
   name[0]     =
   Key[0]      = 
   asyncmsg[0] = 
   phonea[0]   = 
   phoneb[0]   = 0;
   swap        = 0;
   onlynum     = 0;
   exit        = 0;
   pstr = buf;
   while(*pstr && isspace(*pstr)) pstr++;
   if(strncmp(pstr,"bind",4)) continue;
   pstr+=4;
   for(int i = 0; i<3;i++)
   {
    while(*pstr && isspace(*pstr)) pstr++;
    if(*pstr != '"') goto thenext;
    s = ++pstr;
    while(*pstr && *pstr!='"') pstr++;
    if(!*pstr) goto thenext;
    *pstr++ = 0;
    switch(i)
    {
     case 0:
      strcpy(Key,s);
     break;
     case 1:
      strcpy(name,s);
     break;
     case 2:
      strcpy(mname,s);
     break;
    }
   }
   while(*pstr && isspace(*pstr)) pstr++;
   if(*pstr == ',') pstr++;
   while(*pstr)
   {
    while(*pstr && isspace(*pstr)) pstr++;
    s  = pstr;
    while(*pstr && *pstr!='=' && *pstr!=',') pstr++;
    if(*pstr == '=')
    {
     *pstr++ = 0;
     while(*pstr && isspace(*pstr)) pstr++;
     if(*pstr == '"')
     {
      v  = ++pstr;
      while(*pstr && *pstr!='"') pstr++;
     }else
     {
      v  = pstr;
      while(*pstr && *pstr != ',') pstr++;
     }
     if(!*pstr) exit  = 1;
     *pstr = 0;
    }else 
    {
     if(!*pstr) exit  = 1;
     v  = pstr;
     *v = 0;
    }
   if(!strncmp(s,"only_number",10))
   {
    onlynum = 1;
   }else
    if(!strncmp(s,"swap_phone",10))
    {
     swap = 1;
    }else
     if(!strncmp(s,"async_msg",9))
     {
      strcpy(asyncmsg,v);
     }else
       if(!strncmp(s,"phonea",6))
       {
         strcpy(phonea,v);
       }else
        if(!strncmp(s,"phoneb",6))
        {
         strcpy(phoneb,v);
        }
    if(exit)
     break;
    pstr++;
   }
// insert module
   if(!mname[0] || !name[0]) continue;
   ins_module(name,mname,Key,asyncmsg,phonea,phoneb,swap,onlynum);
thenext:
   ;
  }
  fclose(fd);
  return 1;
 }


