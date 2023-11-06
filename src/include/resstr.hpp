

 #include <windows.h>
 #include "resource.h"


 #define RES_STRING_SIZE 256

 #define SAFE_DEL_ARRAY( a )  \
     if ((a)) { \
      delete [] (a);  \
      a = NULL;       \
     }


 class cResStr
  {
    char *m_Str;
  public:
    cResStr ( );
    cResStr ( DWORD idRes );
   ~cResStr ( );
    int String( DWORD idRes );
    char *GetString ( void );
  };


  inline cResStr::cResStr( ) : m_Str(NULL)
   {
   }
  inline cResStr::~cResStr( )
   {
    delete [] m_Str;
   }

  inline cResStr::cResStr( DWORD idRes ) : m_Str(NULL)
   {
    String( idRes );
   }
  inline int cResStr::String( DWORD idRes )
   {
    delete [] m_Str;
    m_Str = new char [ RES_STRING_SIZE ];
    if ( m_Str )
     {
      return (int)::LoadString(GetModuleHandle(NULL),idRes,m_Str,RES_STRING_SIZE-1 );
     }
    return 0;
   }
  inline char * cResStr::GetString ( void )
   {
    return m_Str;
   }



