

 class CValue
 {
 public:
  CValue(const char*name);
 ~CValue();
  CValue& operator=(char*);             // for string
  CValue& operator=(void*);             // for hex
  CValue& operator=(int);               // for int
  CValue& operator=(unsigned int);      // for int 
  CValue& operator=(long);              // for int 
  CValue& operator=(unsigned long);     // for int 
  long from(char*str);
  long get(int*);
  long get(void**);
  long get(char*);
  long size(void);                      // return length
  long sizeA(void);                     // return length
  operator char*() const{return value;}
 private:
  enum{V_INT = 0x10,V_PTR = 0x11,V_STR = 0x12};
  char *get_the_is_type(long type); 
  char *value;
  char *vname;
 };
