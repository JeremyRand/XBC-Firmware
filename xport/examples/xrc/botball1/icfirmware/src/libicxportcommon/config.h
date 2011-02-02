#ifndef IC_CONFIG_H
#define IC_CONFIG_H
// config.h

#define PRE_PROCESS 1

//#define THROW PrintToBuffer("Throwing from " __FILE__ ":%d\n", __LINE__), throw

// For cexp.c:
#define CHAR_TYPE_SIZE 8
#define BITS_PER_UNIT 8
#define INT_TYPE_SIZE 32
#define LONG_TYPE_SIZE 32
#define HOST_BITS_PER_INT 32
#define HOST_BITS_PER_LONG 32
#define TARGET_BELL 7
#define TARGET_BS 8
#define TARGET_FF 12
#define TARGET_NEWLINE 10
#define TARGET_CR 13
#define TARGET_TAB 9
#define TARGET_VT 12

//typedef int Int;

#define INLINE inline

#define DEFAULT_SERIAL_PORT "com1"
#define DEFAULT_EDITOR "emacs"
#define LIB_DIRECTORY "c:\\ic\\libs"
#define IC_LIB_DIRECTORY "c:\\ic\\libs"
#define IC_LIB_FILE "c:\\ic\\libs\\lib_hb.lis"

#define MAX_ALLOCATED_DATA_SIZE 4000000

#define FILE_DIRECTORY_DELIMITERS "\\/"

#define FILE_DIRECTORY_DELIMITER '\\'
#define READ_BINARY "rb"
#define WRITE_BINARY "wb"
#endif
