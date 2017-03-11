#ifndef _PBXBASE_H_
#define _PBXBASE_H_

#include <stdio.h>

struct pbxbase_flags {
	unsigned int flags;
};

extern unsigned int __unsigned_int_flags_dummy;
#define pbxbase_test_flag(p,flag) 		({ \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags & (flag)); \
					})

#define pbxbase_set_flag(p,flag) 		do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags |= (flag)); \
					} while(0)

#define pbxbase_clear_flag(p,flag) 		do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags &= ~(flag)); \
					} while(0)


    #define NONE          "\033[m"   
    #define RED           "\033[0;32;31m"   
    #define LIGHT_RED     "\033[1;31m"   
    #define GREEN         "\033[0;32;32m"   
    #define LIGHT_GREEN   "\033[1;32m"   
    #define BLUE          "\033[0;32;34m"   
    #define LIGHT_BLUE    "\033[1;34m"   
    #define DARY_GRAY     "\033[1;30m"   
    #define CYAN          "\033[0;36m"   
    #define LIGHT_CYAN    "\033[1;36m"   
    #define PURPLE        "\033[0;35m"   
    #define LIGHT_PURPLE "\033[1;35m"   
    #define BROWN         "\033[0;33m"   
    #define YELLOW        "\033[1;33m"   
    #define LIGHT_GRAY    "\033[0;37m"   
    #define WHITE         "\033[1;37m"   

			
#endif
