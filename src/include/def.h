
 typedef struct
 {
  void (*Proc)(void);
  unsigned int Len;
  unsigned int State;
 }PDCPROC;

 typedef struct tag_Option
 {
  char name[40];
  union
  {
   char     *s;
   int       i;
   double    f;
  };
 }Option;

// #define BIND_DEFAULT_FILENAME   "./script/bind.cfg"

 #define MAX_READ_ONE_SHOT 1024
// 24
 #define CPP_WAITBUFFER    512

 #define COMM_DEFAULT_PORTNAME  "COM1"
 #define COMM_DEFAULT_BAUD      19200
 #define COMM_DEFAULT_STOP      ONESTOPBIT
 #define COMM_DEFAULT_PARITY    EVENPARITY
 #define COMM_DEFAULT_BITSDATA  8
 #define COMM_DEFAULT_XON       0x13
 #define COMM_DEFAULT_XOFF      0x14
 #define COMM_DEFAULT_RTSDELAY  500
 #define COMM_DEFAULT_RXSIZE    8192
 #define COMM_DEFAULT_TXSIZE    8192

 #define PRINT_BUSY1        0
 #define PRINT_READY        1
 #define PRINT_BUSY2        2


 #define KEYBOARD_TABLE_DEFAULT "keyb.tab"
 #define RUS_LAYOUT         0
 #define LAT_LAYOUT         1

// window defines
 #define WIN_POS_DEFAULT_X    0
 #define WIN_POS_DEFAULT_Y    0
 #define WIN_WIDTH_DEFAULT    648
 #define WIN_HEIGHT_DEFAULT   507
 #define DELAY_BETWEEN_EVENTS 150
/*
 console dopt defines (warning this parameter is critical changes )
*/
#define DOPTCOL     80
#define DOPTROW     75
#define SCRCOL      80
#define SCRROW      25


/*
 Config files from debug trace and another function 
*/
#define CONFIG_FILE          "dopt.cfg"

#define F_DEBUG         (unsigned int)0x80000000
#define F_OUTPUT        (unsigned int)0x08000000

#define TRACE_WRITE   0
#define TRACE_READ    1
/*
 BINDING FUNCTION
*/
//#define FM_SYNC	   0
//#define FM_ASYNC   1
//#define FM_READY   0
//#define FM_BUSY    1

/*
   PLUGIN  COMMAND
*/
//#define OM_MODE        0x100


#ifndef _DOPT_DEFINE_DATA
 extern  unsigned char *gwin_CHR_Mem;
 extern  unsigned char *gwin_PAM_Mem;
 extern  unsigned char *gwin_DAM_Mem;
 extern  unsigned char *gwin_TAB_Mem;
 extern  int gwin_Cur_Row;
 extern  int gwin_Cur_Col;
 extern  int gwin_Cur_Start;
 extern  int gwin_Cur_End;
 extern  int gwin_Cur_Control;
 extern  int gwin_Format_Mode;
 extern  int gwin_Protect_Mode;
 extern  int gwin_Test_Mode;
 extern  int gwin_Insert_Mode;
 extern  int gwin_Lowcase_Mode;
 extern  int gwin_Keylock_Mode;
 extern  int gwin_Shflock_Mode;
 extern  int gwin_Led_Stats[9];
 extern  int gwin_Led[9];
 extern  int gwin_MLU;
 extern  int gwin_MLL;
 extern  int gwin_SLU;
 extern  int gwin_SLL;
 extern  int gwin_SVW_Row;
 extern  int gwin_f_crsr_start;
 extern  int gwin_f_crsr_end;
 extern  int gwin_ESC_u_Flag;
 extern  int gwin_f_disp_lines;
#endif

