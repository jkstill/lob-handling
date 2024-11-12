
/* --------------------------------------------------------------------------------------------- *
 * COMMON DEFINES
 * --------------------------------------------------------------------------------------------- */

#define ARG_DB     1
#define ARG_USER   2
#define ARG_PWD    3
#define ARG_HOME   4

#define ARG_COUNT  5

#define SIZE_STR   260
#define SIZE_BUF   2048
#define SIZE_TAB   5

#define SIZE_ARRAY 100
#define NB_LOAD    10
#define SIZE_COL1  20
#define SIZE_COL2  30
#define SIZE_COL3  8
#define NUM_COLS   3

/* --------------------------------------------------------------------------------------------- *
 * PROGRAM ARGUMENTS
 * --------------------------------------------------------------------------------------------- */

#if defined(OCI_CHARSET_WIDE)

  #if defined(_MSC_VER)

    #define omain           wmain
    #define oarg            otext
    #define GET_ARG(s, i)   wcsncat(s, argv[i], sizeof(s))

  #else

    #define omain           main
    #define oarg            char
    #define GET_ARG(s, i)   mbstowcs(s, argv[i], sizeof(s))

  #endif

    #define ostrlen         wcslen
    #define osprintf        swprintf

#else

  #define omain           main
  #define oarg            char
  #define print_args(x)   printf(x)
  #define GET_ARG(s, i)   strncat(s, argv[i], sizeof(s)-1)

  #define ostrlen         strlen
  #define osprintf        snprintf

#endif

