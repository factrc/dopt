

 struct _key_scan_name
 {
  struct _key_scan_name *next;
  char *name;
  unsigned long scan;
 };
 class CScanName
 {
  struct _key_scan_name *list;
 public:
   CScanName():list(NULL){}
  ~CScanName(){if(list) removeall();}
   unsigned long Find(const char*name);
   unsigned long Insert(const char*name,unsigned long scan);
   void removeall(void);
 };
