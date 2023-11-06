#include "dopt.h"


 CValue::CValue(const char*name):value(NULL),vname(NULL)
 {
  vname = (char*)realloc(vname,strlen(name)+1);
  value = (char*)realloc(value,strlen(name)+6);
  strcpy(vname,name);
  if(value&&vname)
   sprintf(value,"%s = \"\"",vname);
 }

 CValue::~CValue()
 {
   if(value)
   {
    free(value);
    value = NULL;
   }
   if(vname)
   {
    free(vname);
    vname = NULL;
   }
 }

 long CValue::size(void)
 {
  char *str = strchr(value,'=');
  return (str)?strlen(str):0;
 }

 long CValue::sizeA(void)
 {
  return (value)?strlen(value):0;
 }

 char* CValue::get_the_is_type(long type)
 {
  char *str,
       *ret_ptr = NULL;
  long szp;

  str = strchr(value,'=');

  if(str && ( ret_ptr = (char*)calloc(1,strlen(str))) )
  {
   ++str;
   for( int i=0;*str&&*str!='\n';i++){
    ret_ptr[i] = *str++;
   }
   trim(ret_ptr);
   szp = strlen(ret_ptr);
   if(szp>=2 && ret_ptr[0]=='"'){
    ret_ptr[szp-1] = 0;
    memmove(&ret_ptr[0],&ret_ptr[1],szp-1);
   }
  }
  if(ret_ptr)
  {
   switch(type)
   {
    case V_INT:
     str = ret_ptr;
     while(*str){
      if(isspace(*str)||isdigit(*str))
      {
       str++;
       continue;
      }
      free(ret_ptr);
      ret_ptr = NULL;
      break;
     }
    break;
    case V_PTR:
     str = ret_ptr;
     while(*str){
      if(isspace(*str)||isxdigit(*str))
      {
       str++;
       continue;
      }
      free(ret_ptr);
      ret_ptr = NULL;
      break;
     }
    break;
   }
  }
  return ret_ptr;
 }

 long CValue::get(int*is_int)
 {
  char *tmp_ptr = get_the_is_type(V_INT);
  if(tmp_ptr)
  {
    trim(tmp_ptr);
    *is_int = atoi(tmp_ptr);
    free(tmp_ptr);
  }
  return -1;
 }
 long CValue::get(void**is_ptr)
 {
  char *tmp_ptr = get_the_is_type(V_PTR);
  if(tmp_ptr)
  {
   trim(tmp_ptr);
   sscanf(tmp_ptr,"%X",(long*)is_ptr);
   free(tmp_ptr);
  }
  return -1;
 }
 long CValue::get(char*is_str)
 {
  char *tmp_ptr = get_the_is_type(V_STR);
  if(tmp_ptr)
  {
   strcpy(is_str,tmp_ptr);
   free(tmp_ptr);
  }
  return -1;
 }
 
 CValue& CValue::operator=(char*is_str)
 {
  if(value&&vname)
  {
   free(value);
   value = (char*)calloc(1,strlen(vname)+strlen(is_str)+7); //  = ""
   sprintf(value,"%s = \"%s\"\n",vname,is_str);
  }
  return *this;
 }
 CValue& CValue::operator=(void*is_ptr)
 {
  if(value&&vname)
  {
   free(value);                              // 00 00 00 00 = 8 + 6 = 14
   value = (char*)calloc(1,strlen(vname)+15);//  = ""
   sprintf(value,"%s = \"%x\"\n",vname,is_ptr);
  }
  return *this;
 }
 CValue& CValue::operator=(int is_int)
 {
  *this = (unsigned long) is_int;
  return *this;
 }
 CValue& CValue::operator=(unsigned int is_uint)
 {
  *this = (unsigned long) is_uint;
  return *this;
 }
 CValue& CValue::operator=(long is_long)
 {
  *this = (unsigned long) is_long;
  return *this;
 }
 CValue& CValue::operator=(unsigned long is_ulong)
 {
  if(value&&vname)
  {
   free(value);                              // 00 00 00 00 = 8 + 6 = 14
   value = (char*)calloc(1,strlen(vname)+25);//  = ""
   sprintf(value,"%s = \"%d\"\n",vname,is_ulong);
  }
  return *this;
 }

 long CValue::from(char*str)
 {
  long ret = -1;
  if(!str||!vname||!value) return ret;

  while(str){
   char *p1 = strchr(str,'='),*p2;
   if(p1){
    char *tmp = (char*)calloc(1,p1-str+1);
    long szp;
    if(!tmp) return ret;
    memcpy(tmp,str,p1-str);
    trim(tmp);
    if(!stricmp(tmp,vname)){
     p2 = strchr(++p1,'\n');
     if(!p2){
      p2 = p1+strlen(p1);
     }
     tmp = (char*)realloc(tmp,p2-p1+1);
     memcpy(tmp,p1,p2-p1);
     tmp[p2-p1]=0;
     trim(tmp);
     szp = strlen(tmp);
     if(szp>=2&&tmp[0]=='"'){
      tmp[szp-1] = 0;
      memmove(&tmp[0],&tmp[1],szp-1);
     }
     *this = (char*)tmp;
     ret = 1;
     free(tmp);
     break;
    }else
     free(tmp);
   }else break; // nothing name name = ["]value["]\n
   p1 = strchr(++str,'\n');
   if(p1)
    str = p1+1;
  }
  return ret;
 }


/*
  long ret = -1;
  char *p = str;
  char *nm,*vl;
  long len,len1;

  if(!str||!vname||!value)
   return ret;

  while(*str)
  {
   while(*str && isspace(*str)) str++;
   if(!*str)
    break;
   p = str;
   while(*str && !isspace(*str) && *str!='=') str++;
   if(!strnicmp(p,vname,str-p))
   {
    nm = p;
    len  = str-p;
   }
   if(*str!='=')
    while(*str && *str!='=') str++;
   if(!*str)
    break;
   str++;
   while(*str && isspace(*str)) str++;
   p = str;
   if(*str == '"')
   {
    str++;
    p = str;
    while(*str)
    {
     while(*str && *str!='"') str++;
     if(*str == '"' && str[-1] != '\\') break;
    }
   }
*/


/*

   while(*str && isspace(
   



   char *p1 = strchr(str,'='),*p2;
   if(p1)
   {
    char *tmp = (char*)calloc(1,p1-str+1);
    long szp;
    if(!tmp) return ret;
    memcpy(tmp,str,p1-str);
    trim(tmp);
    if(!stricmp(tmp,vname))
    {
     p2 = strchr(++p1,'\n');
     if(!p2)
     {
      p2 = p1+strlen(p1);
     }
     tmp = (char*)realloc(tmp,p2-p1+1);
     memcpy(tmp,p1,p2-p1);
     tmp[p2-p1]=0;
     trim(tmp);
     szp = strlen(tmp);
     if(szp>=2&&tmp[0]=='"')
     {
      tmp[szp-1] = 0;
      memmove(&tmp[0],&tmp[1],szp-1);
     }
     *this = (char*)tmp;
     ret = 1;
     free(tmp);
     break;
    }else
     free(tmp);
   }else break; // nothing name name = ["]value["]\n
   p1 = strchr(++str,'\n');
   if(p1)
    str = p1+1;
  }
  return ret;
 }
*/