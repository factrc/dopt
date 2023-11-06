

 class BitArray
 {
  private:
   unsigned long  *m_array;
   long m_size;
   void fill(unsigned long value);
   void copy2(unsigned long *ptr1);
   void copyfrom(unsigned long *ptr1,long size);
  public:
    BitArray(long size);    
   ~BitArray();
    unsigned long align(long i);
    int  get(long i);      
    void set(long i);       
    void unset(long i);     
    void toggle(long i);    
    void clear(void);       
    void setting(void);     
    long change(long size); 
    void set( long i, int size );
 };

 inline BitArray::~BitArray()
 {
  if(m_array)
  {
   delete [] m_array;
   m_array = 0;
  }
  m_size = 0;
 }

 inline BitArray::BitArray(long size):m_array(NULL),m_size(0)
 {
  m_size    = size/(sizeof(unsigned long)*8)+1;
  m_array  = new unsigned long[m_size];
  clear();
 }

 inline void BitArray::fill(unsigned long value)
 {
   for(int i=0;i<m_size;i++)
    m_array[i] = value;
 }
 inline void BitArray::clear(void)
 {
  fill(0L);
 }
 inline void BitArray::setting(void)
 {
  fill(~0L);
 }

 inline long BitArray::change(long size)
 {
  long new_size = size/(sizeof(unsigned long)+1);
  unsigned long *new_ptr = m_array;
  if(new_size>m_size){
    new_ptr = new unsigned long[new_size];
    if(new_ptr){
      memset((void*)new_ptr,0,size*sizeof(unsigned long));
      if(m_array){
       copy2(new_ptr);
       delete m_array;
      }
      m_array = new_ptr;
      m_size   = new_size;
    }
  }
  return new_ptr!=0;
 }
 inline void BitArray::copy2(unsigned long*ptr1)
 {
  memcpy(ptr1,m_array,m_size<<2);
 }
 inline void BitArray::copyfrom(unsigned long*ptr1,long size)
 {
  memcpy(m_array,ptr1,size>>2);
 }

 inline void BitArray::set(long i)
 {
   m_array[i>>5] |=  1<<(i&(sizeof(unsigned long)*8-1));
 }

 inline void BitArray::set( long i, int size )
 {
///*
  for ( ;size>=0;size-- )
   {
    m_array[i>>5] |=  1<<(i&(sizeof(unsigned long)*8-1));
    i++;
   }
//*/
 /*
  int modi = i & (sizeof(unsigned long)*8-1);
  int mod  = size & (sizeof(unsigned long)*8-1);
  i>>=5;
  size >>= 5;
  if ( modi )
   {
    int s = sizeof(unsigned long)*8 - modi;
    m_array[i] = ((~0) >> s);
    i++;
   }
  for ( int j = 0; j < size ; j++ )
   {
    m_array[i++] = ~0;
   }
  if ( mod )
   {
    size = sizeof(unsigned long)*8 - mod;
    m_array[i] = ((~0) >> size) << size;
   }
*/
 }

 inline void BitArray::unset(long i)
 {
   m_array[i>>5]  = ~(1<<(i&(sizeof(unsigned long)*8-1))); 
 }

 inline void BitArray::toggle(long i)
 {
   m_array[i>>5] ^=  1<<(i&(sizeof(unsigned long)*8-1));
 }
 inline int BitArray::get(long i)
 {
   return m_array[i>>5] & ( 1 << ( i & (sizeof(unsigned long)*8-1)) );

 }
 inline unsigned long BitArray::align(long i)
 {
   return m_array[i>>5];
 }
